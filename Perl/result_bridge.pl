# Script to bridge results from ompp DB to Modgen output DB
# The script takes three arguments:
# (1) a example Modgen output Access MDB for the model, 
#     with the correct number of sub-samples, and containing parameters as well as tables.
# (2) an ompp DB, containing the scenario and run to be converted to modgen mdb format
# (3) the name of the target mdb which will be produced by this script,
#     which will have a structure like (1) but content from (2).

use strict;
use warnings;

# Add the directory of this script (ompp/Perl) to the search path 
use FindBin; # locate this script
use lib "$FindBin::Bin/."; # use the directory of this script for resolution of 'use' statements


if ($#ARGV != 2) {
	# must have exactly three arguments
	print "usage: result_bridge example.mdb model.sqlite output.mdb\n";
	exit -1;
}

# example Modgen DB
my $example_mdb = shift @ARGV;
-f $example_mdb or die;

# input ompp DB
my $input_sqlite = shift @ARGV;
-f $input_sqlite or die;

# output Modgen DB
my $output_mdb = shift @ARGV;

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
			
# copy example MDB to output MDB
copy $example_mdb, $output_mdb;

# Following section adapted from common.pm (ompp_tables_to_csv)

#
# Get output table metadata (name, rank, hid, expr_dim_pos)
#

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
	my $tables_info = run_sqlite_statement($input_sqlite, $sql, $retval);
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
	my $lines = run_sqlite_statement($input_sqlite, $sql, $retval);
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
	my $lines = run_sqlite_statement($input_sqlite, $sql, $retval);
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
my $parameters_info = run_sqlite_statement($input_sqlite, $sql, $retval);
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

my $dbcopy_exe = "${om_root}/bin/dbcopy.exe";
-f $dbcopy_exe or die;

my $temp_dir;
my $keep_temp = 1;
if ($keep_temp == 0) {
	# For production, place dbcopy results in temporary directory, which is deleted at script end.
	my $temp_dir = tempdir( CLEANUP => 1 );
}
else {
	# For debugging, place dbcopy results in known fixed location.
	$temp_dir = "C:/Development/X/models/GMM/bridge_test/tmp";
	# Remove any pre-existing contents or else anomalies can result
	remove_tree $temp_dir || die;
	make_path $temp_dir || die;
}
print "temp_dir=$temp_dir\n";

# Assume model name is base name of sqlite file
my $model = $input_sqlite;
$model =~ s/[.]sqlite$//;
$model =~ s/^.*[\/]//;

# connection string to ompp DB
my $conn = "\"Database=${input_sqlite}; Timeout=86400; OpenMode=ReadWrite;\"";

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

# Open output Access DB
use Win32::OLE;
use Win32::OLE::Const 'Microsoft ActiveX Data Objects';

my $sConnect = "Provider = Microsoft.ACE.OLEDB.12.0; Data source = ${output_mdb}";
my $ADO_Conn = Win32::OLE->new('ADODB.Connection');	# creates a connection object
my $ADO_RS = Win32::OLE->new('ADODB.Recordset');		# creates a recordset object
$ADO_Conn->Open($sConnect);
if (Win32::OLE->LastError()) {
	print "Fatal Error: ", Win32::OLE->LastError(), "\n";
	exit -1;
}

my @fields; # working variable

#
# Get the scenario name, date-time
#

my $scenario_name;
my $scenario_date_time;
{
	my $workset_lst = "${temp_dir}/${model}/workset_lst.csv";
	-e $workset_lst or die;
	open INPUT, '<'.$workset_lst or die;
	# Skip header record
	my $header = <INPUT>;
	# Get single record for workset
	my $record = <INPUT>;
	chomp $record;
	@fields = split(/,/, $record);
	# Fields are set_id,base_run_id,model_id,set_name,is_readonly,update_dt
	$scenario_name = $fields[3];
	print "scenario_name=${scenario_name}\n";
	$scenario_date_time = $fields[5];
	print "scenario_date_time=${scenario_date_time}\n";
	close INPUT;
}

#
# Get the scenario description, note
#

my $scenario_descr;
my $scenario_note;
{
	my $workset_txt = "${temp_dir}/${model}/workset_txt.csv";
	-e $workset_txt or die;
	open INPUT, '<'.$workset_txt or die;
	# Skip header record
	my $header = <INPUT>;
	# Get first record.  Assume is for default language
	my $record = <INPUT>;
	chomp $record;
	@fields = split(/,/, $record);
	# Fields are set_id,lang_code,descr,note
	$scenario_descr = $fields[2];
	print "scenario_descr=${scenario_descr}\n";
	$scenario_note = $fields[3];
	print "scenario_note=${scenario_note}\n";
	close INPUT;
}

#
# Set the scenario information in the output DB
#

{
	# The following sql updates the columns for all languages.
	# using the default language in the ompp sqlite.
	$sql = "Update ScenarioDic
			Set [Name]='${scenario_name}',
			    [Description]='${scenario_descr} (${scenario_date_time})',
				[Note]='${scenario_note}'
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

for my $parameter (@parameters) {
	print "processing parameter ${parameter}\n";
	# Skip known ompp framework parameters with no modgen equivalent
	next if $parameter eq 'SimulationSeed';
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

# TODO hard-coded - obtain actual number of simulation members
my $members = 16;

#
# replace table values
#

# Single run assumed
my $table_dir = glob("${temp_dir}/${model}/run.*/");

for my $table (@tables) {
	print "processing table ${table}\n";
	
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

		# TODO compute CV based on @member_values
		my $cv = 0.0;
		push @out_fields, $cv;
		
		# TODO compute SE based on @member_values
		my $se = 0.0;
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
}