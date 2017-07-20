# Script to bridge results from ompp DB to Modgen output DB
# The script takes three arguments:
# (1) a example Modgen output Access MDB for the model, 
#     with the correct number of sub-samples, and containing parameters as well as tables.
# (2) an ompp DB, containing the scenario and run to be converted to modgen mdb format
# (3) the name of the target mdb which will be produced by this script,
#     which will have a structure like (1) but content from (2).

use strict;
use warnings;

my $script_name = "result_bridge";
my $script_version = '1.1';

use Getopt::Long::Descriptive;

my ($opt, $usage) = describe_options(
	$script_name.' %o',
	[ 'help|h'     => 'print usage message and exit' ],
	[ 'version|v'  => 'print version and exit' ],
	[ 'model|m=s'  => 'model name' ],
	[ 'ref_db|r=s' => 'reference database (Modgen mdb)' ],
	[ 'in_db|i=s'  => 'input database (ompp sqlite)' ],
	[ 'out_db|o=s' => 'output database (Modgen mdb)' ],
	[ 'keep_tmp|k' => 'keep temporary files in ./tmp_result_bridge' ],
	[ 'verbose'    => 'verbose log output' ],
);

if ($opt->version) {
	print $script_name.' version '.$script_version."\n";
	exit 0;
}

if ($opt->help) {
	print $usage->text;
	exit 0;
}

# Add the directory of this script (ompp/Perl) to the search path 
use FindBin; # locate this script
use lib "$FindBin::Bin/."; # use the directory of this script for resolution of 'use' statements

# reference Modgen DB
$opt->ref_db or die "ref_db is required";
my $ref_db = $opt->ref_db;
-f $ref_db or die "invalid ref_db";

# input ompp DB
$opt->in_db or die "in_db is required";
my $in_db = $opt->in_db;
-f $in_db or die "invalid in_db";

# output Modgen DB
$opt->out_db or die "out_db is required";
my $out_db = $opt->out_db;

# model name
$opt->model or die "model name is required";
my $model = $opt->model;


use Cwd qw(getcwd);

# OM_ROOT is needed to locate the dbcopy.exe utility
my $om_root = $ENV{'OM_ROOT'};
$om_root ne '' or die 'environment variable OM_ROOT not defined, stopped';

use File::Copy;
use File::Path qw(make_path remove_tree);
use File::Temp qw(tempdir);
use Capture::Tiny qw/capture tee capture_merged tee_merged/;

# use the common.pm module of shared functions
use common qw(
				warning change error info
				sqlite3_exe
				logmsg
				logerrors
				run_sqlite_script
				run_sqlite_statement
				ompp_tables_to_csv
				run_jet_statement
				modgen_tables_to_csv
		    );

my $retval; # return value from program
my $merged; # output from program
my $sql; # a SQL statement
			
# Following section adapted from common.pm (ompp_tables_to_csv)

#
# Get output table metadata (name, rank, hid, expr_dim_pos)
#

print "Processing metadata...\n";

# table_name is used as the key to tables in this script
# This assumes that the DB contains only one model.

my %table_rank;
my %table_hid;
my %table_expr_dim_pos;
my @tables;
{
	# Join table_dic with model_table_dic to get expr_dim_pos
	$sql =
	"Select table_dic.table_name, table_dic.table_rank, table_dic.table_hid, model_table_dic.expr_dim_pos
		From table_dic
		Inner Join model_table_dic
		On table_dic.table_hid = model_table_dic.table_hid
		Order By table_name;";
	my $tables_info = run_sqlite_statement($in_db, $sql, $retval);
	($retval == 0) or die;
	# Create array of table names and hash table of rank for each table.
	for my $record (split(/\n/, $tables_info)) {
		(my $col1, my $col2, my $col3, my $col4) = split(/\|/, $record);
		push @tables, $col1;
		$table_rank{$col1} = $col2;
		$table_hid{$col1} = $col3;
		$table_expr_dim_pos{$col1} = $col4;
	}
}

# Number of accumulators underlying measures
my %table_acc_count;
{
	# Get number of 'true' (non-derived) accumulators for each table
	$sql =
"SELECT table_dic.table_name, Count(table_acc.is_derived) AS acc_count
FROM table_acc INNER JOIN table_dic ON table_acc.table_hid = table_dic.table_hid
WHERE (((table_acc.is_derived)=0))
GROUP BY table_dic.table_name;";
	my $lines = run_sqlite_statement($in_db, $sql, $retval);
	($retval == 0) or die;
	for my $record (split(/\n/, $lines)) {
		(my $col1, my $col2) = split(/\|/, $record);
		$table_acc_count{$col1} = $col2;
	}
}

# Number of measures
my %table_expr_count;
{
	# Get number of expressions (aka derived accumulators) for each table
	$sql =
"SELECT table_dic.table_name, Count(table_acc.is_derived) AS acc_count
FROM table_acc INNER JOIN table_dic ON table_acc.table_hid = table_dic.table_hid
WHERE (((table_acc.is_derived)=1))
GROUP BY table_dic.table_name;";
	my $lines = run_sqlite_statement($in_db, $sql, $retval);
	($retval == 0) or die;
	for my $record (split(/\n/, $lines)) {
		(my $col1, my $col2) = split(/\|/, $record);
		$table_expr_count{$col1} = $col2;
	}
}


#
# Get parameter metadata (names, ranks)
#

$sql = "Select parameter_name, parameter_rank, type_hid
		From parameter_dic
		Order By parameter_name;";
my $parameters_info = run_sqlite_statement($in_db, $sql, $retval);
($retval == 0) or die;
# Create array of parameter names and hash table of rank for each parameter.
my %parameter_rank;
my %parameter_type;
my @parameters;
for my $record (split(/\n/, $parameters_info)) {
	(my $col1, my $col2, my $col3) = split(/\|/, $record);
	push @parameters, $col1;
	$parameter_rank{$col1} = $col2;
	$parameter_type{$col1} = $col3;
}

# Use dbcopy to dump the ompp scenario to a temporary folder

print "Extracting data from input db...\n";

my $dbcopy_exe = "${om_root}/bin/dbcopy.exe";
-f $dbcopy_exe or die;

my $temp_dir;
if ($opt->keep_tmp) {
	# For debugging, place dbcopy results in known fixed location.
	$temp_dir = "./tmp_result_bridge";
	# Remove any pre-existing contents or else anomalies can result
	remove_tree $temp_dir || die;
	make_path $temp_dir || die;
}
else {
	# For production, place dbcopy results in temporary directory, which is deleted at script end.
	$temp_dir = tempdir( CLEANUP => 1 );
}
print "temp_dir=$temp_dir\n" if $opt->verbose;

# connection string to ompp DB
my $in_db_msdos = $in_db;
$in_db_msdos =~ s@/@\\@g;
my $conn = "\"Database=${in_db_msdos}; Timeout=86400; OpenMode=ReadWrite;\"";

# Dump ompp DB to csv
($merged, $retval) = capture_merged {
	my @args = (
		"${dbcopy_exe}",
		"-m" , $model,
		"-dbcopy.To", "csv",
		"-dbcopy.IdCsv",
		"-dbcopy.DatabaseDriver", "SQLite",
		"-dbcopy.Database", $conn,
		"-dbcopy.OutputDir", $temp_dir,
		);
	system(@args);
};

if ($retval) {
	print $merged;
	die;
}

my @fields; # working variable

#
# Get the run name, date-time
#

print "Processing run information...\n";

my $run_id = -1;
my $run_name = 'untitled';
my $run_date_time = '';
my $run_sub_count = 1;
my $run_sub_completed = 0;
{
	my $run_lst = "${temp_dir}/${model}/run_lst.csv";
	-e $run_lst or die;
	open INPUT, '<'.$run_lst or die;
	# Skip header record
	my $header = <INPUT>;
	# Get single record for run
	my $record = <INPUT>;
	if (defined($record)) {
		chomp $record;
		@fields = split(/,/, $record);
		# Fields are run_id,model_id,run_name,sub_count,sub_started,sub_completed,create_dt,status,update_dt,run_digest
		$run_id = $fields[0];
		print "run_id=${run_id}\n" if $opt->verbose;
		$run_name = $fields[2];
		print "run_name=${run_name}\n" if $opt->verbose;
		$run_sub_count = $fields[3];
		print "run_sub_count=${run_sub_count}\n" if $opt->verbose;
		$run_sub_completed = $fields[5];
		print "run_sub_completed=${run_sub_completed}\n" if $opt->verbose;
		$run_date_time = $fields[6];
		print "run_date_time=${run_date_time}\n" if $opt->verbose;
	}
	else {
		close INPUT;
		die 'missing run record in run_lst';
	}
	close INPUT;
}

die "run is incomplete - stopped." if $run_sub_count != $run_sub_completed;

#
# Get the run description, note
#

my $run_descr = $run_name;
my $run_note = '';
{
	my $run_txt = "${temp_dir}/${model}/run_txt.csv";
	-e $run_txt or die;
	open INPUT, '<'.$run_txt or die;
	# Skip header record
	my $header = <INPUT>;
	# Get first record (if present).  Assume is for default language
	my $record = <INPUT>;
	if (defined($record)) {
		chomp $record;
		@fields = split(/,/, $record);
		# Fields are run_id,lang_code,descr,note
		$run_descr = $fields[2];
		print "run_descr=${run_descr}\n" if $opt->verbose;
		$run_note = $fields[3];
		print "run_note=${run_note}\n" if $opt->verbose;
	}
	close INPUT;
}

# copy example MDB to output MDB
copy $ref_db, $out_db;

# Open output Access DB
use Win32::OLE;
use Win32::OLE::Const 'Microsoft ActiveX Data Objects';

my $sConnect = "Provider = Microsoft.ACE.OLEDB.12.0; Data source = ${out_db}";
my $ADO_Conn = Win32::OLE->new('ADODB.Connection');	# creates a connection object
my $ADO_RS = Win32::OLE->new('ADODB.Recordset');		# creates a recordset object
$ADO_Conn->Open($sConnect);
if (Win32::OLE->LastError()) {
	print "Fatal Error: ", Win32::OLE->LastError(), "\n";
	exit -1;
}

#
# Set the scenario information in the output DB
#

{
	# The following sql updates the columns for all languages.
	# using the default language in the ompp sqlite.
	# Description is set to run_name, because it is used by Modgen web to name the run.
	$sql = "Update ScenarioDic
			Set [Name]='${run_name}',
			    [Description]='${run_descr}',
				[Note]='${run_date_time} ${run_note}'
			;";
	$ADO_RS = $ADO_Conn->Execute($sql);
	if (Win32::OLE->LastError()) {
		print "Fatal Error: ", Win32::OLE->LastError(), "\n";
		print "sql=${sql}\n";
		exit -1;
	}
}

#
# replace parameter values
#

# Single set assumed
my $parameter_dir = glob("${temp_dir}/${model}/set.*/");

print "Processing ".(1+$#parameters)." parameters...\n";
for my $parameter (@parameters) {
	print "processing parameter ${parameter}\n" if $opt->verbose;
	# Skip known ompp framework parameters with no modgen equivalent
	next if $parameter eq 'SimulationSeed';
	next if $parameter eq 'Member';
	next if $parameter eq 'MemberCount';
	
	# Remove all existing rows from parameter
	$sql = "Delete * From ${parameter};";
	$ADO_RS = $ADO_Conn->Execute($sql);
	if (Win32::OLE->LastError()) {
		print "Fatal Error: ", Win32::OLE->LastError(), "\n";
		print "sql=${sql}\n";
		exit -1;
	}
	
	# Insert all data from the csv file for the parameter
	my $parameter_csv = "${parameter_dir}/${parameter}.csv";
	-e $parameter_csv or die;
	open INPUT, '<'.$parameter_csv or die;
	# Skip header record
	<INPUT>;
	# Insert all other records
	while (<INPUT>) {
		chomp;
		my $values = $_;
		# Remove leading sub_id field from csv
		$values =~ s/^[^,]+,//;
		# TODO hard-coded type for file parameters
		if ($parameter_type{$parameter} == 21) {
			# type is file (string)
			# quote it
			$values = "'".$values."'";
		}
		$sql = "Insert Into ${parameter} Values (${values});";
		$ADO_RS = $ADO_Conn->Execute($sql);
		if (Win32::OLE->LastError()) {
			print "Fatal Error: ", Win32::OLE->LastError(), "\n";
			print "sql=${sql}\n";
			exit -1;
		}
	}
	close INPUT;
}

# number of simulation members
my $members = $run_sub_count;

#
# replace table values
#

# Single run assumed
my $table_dir = glob("${temp_dir}/${model}/run.*/");

print "Processing ".(1+$#tables)." tables...\n";
for my $table (@tables) {
	print "processing table ${table}\n" if $opt->verbose;
	
	# Get metadata info for table into working variables
	my $rank = $table_rank{$table};
	my $expr_dim_pos = $table_expr_dim_pos{$table};
	my $acc_count;
	my $expr_count;
	$acc_count = $table_acc_count{$table};
	$expr_count = $table_expr_count{$table};
	#print "rank=$rank expr_dim_pos=$expr_dim_pos is_user=$is_user acc_count=$acc_count expr_count=$expr_count\n";
	
	# Get member information into associative array
	my %table_all;
	%table_all = ();  # Empty it
	{
		my $table_all_csv = "${table_dir}/${table}.acc-all.csv";
		-e $table_all_csv or die;
		open INPUT, '<'.$table_all_csv or die;
		# Skip header record
		<INPUT>;
		# Insert all records
		while (<INPUT>) {
			chomp;
			my $line = $_;

			# in_fields example for rank 2 table:
			#  sub_id,dim0,dim1,acc0,acc1,... (both 'accumulators' and 'derived accumulators')
			my @fields = split(/,/, $line);
			for (my $expr_id = 0; $expr_id < $expr_count; $expr_id++) {
				my $key;
				my $value;
				
				my $sub_id = $fields[0];
				
				# build key
				# key looks like
				# sub_id,dim0,...dimN,expr_id
				$key = join(',',@fields[0..$rank]);
				$key .= ','.$expr_id;

				# get corresponding value
				$value = $fields[1 + $rank + $acc_count + $expr_id];
				
				# insert it into the hash
				$table_all{$key} = $value;
			}
		}
	}

	# Remove all existing rows from the table in the output Modgen Access DB
	$sql = "Delete * From ${table};";
	$ADO_RS = $ADO_Conn->Execute($sql);
	if (Win32::OLE->LastError()) {
		print "Fatal Error: ", Win32::OLE->LastError(), "\n";
		print "sql=${sql}\n";
		exit -1;
	}

	# Insert all data from the csv files for the table
	
	# Get the aggregate value from the csv file for the table
	my $table_csv = "${table_dir}/${table}.csv";
	-e $table_csv or die;
	open INPUT, '<'.$table_csv or die;
	# Skip header record
	<INPUT>;
	
	# Insert all records
	while (<INPUT>) {
		chomp;
		my $in_values = $_;
		my $out_values;

		# in_fields example for rank 2 table:
		#  expr_id,dim0,dim1,expr_value
		my @in_fields = split(/,/, $in_values);
		my $expr_id = $in_fields[0];
		my $expr_value = $in_fields[$rank + 1];

		# out_fields example for rank 2 table:
		#  Dim0, Dim1, Dim2, Value, CV, SE, Value0, Value1, ..., Value15
		# Note that one of Dim0,Dim1,Dim2 is the measure dimension
		my @out_fields;
		@out_fields = ();
		
		# Build the indices, inserting the measure index into the position required by modgen
		for (my $j = 0; $j < $rank; $j++) {
			if ($expr_dim_pos == $j - 1) {
				# place measure dimension into position before this dimension
				push @out_fields, $expr_id;
			}
			push @out_fields, $in_fields[$j + 1];
		}
		if ($expr_dim_pos == $rank - 1) {
			# measure dimension is last dimension
			push @out_fields, $expr_id;
		}

		push @out_fields, $expr_value;
		
		# Get values for each simulation member
		my @member_values;
		@member_values = (); # empty it
		for (my $sub_id = 0; $sub_id < $members; $sub_id++) {
			# construct key for sub value
			# key looks like
			# sub_id,dim,...dimN,expr_id
			my $key = $sub_id;
			if ($rank > 0) {
				$key .= ','.join(',',@in_fields[1..$rank]);
			}
			$key .= ','.$expr_id;
			# retrieve value
			my $expr_value_sub = $table_all{$key};
			#print "key=$key value=$expr_value_sub\n";
			push @member_values, $expr_value_sub;
		}
		#print "member_values=".join(',',@member_values)."\n";

		# compute summary statistics using @member_values
		my $se = 0.0;
		my $cv = 0.0;
		{
			my $N = 0;
			my $sum1 = 0;
			my $mu = 0;
			for my $X (@member_values) {
				if ($X ne 'null') {
					$N++;
					$sum1 += $X;
				}
			}
			
			if ($N > 1) {
				$mu = $sum1 / $N;
				my $sum2 = 0;
				for my $X (@member_values) {
					if ($X ne 'null') {
						$sum2 += ($X - $mu)*($X - $mu);
					}
				}
				my $var = $sum2 / ($N - 1);
				my $sd = sqrt($var);
				
				$se = $sd / sqrt($N);
				$cv = ($mu == 0) ? 0 : ($sd / $mu);
			}
		}

		push @out_fields, $cv;
		push @out_fields, $se;
		
		push @out_fields, @member_values;
		
		$out_values = join(',', @out_fields);
	
		$sql = "Insert Into ${table} Values (${out_values});";
		$ADO_RS = $ADO_Conn->Execute($sql);
		if (Win32::OLE->LastError()) {
			print "Fatal Error: ", Win32::OLE->LastError(), "\n";
			print "sql=${sql}\n";
			exit -1;
		}
	}
	close INPUT;
}

print "Finished processing.\n";
