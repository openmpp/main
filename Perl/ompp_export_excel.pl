# Copyright (c) 2013-2016 OpenM++ contributors
# This code is licensed under the MIT license (see LICENSE.txt for details)

# Script to export the output tables of a SQLite scenario to Excel 
# Workbook structure is identical to Modgen export to Excel "pivot table format"

use strict;
use warnings;

my $script_name = "ompp_export_excel";
my $script_version = '1.0';

use Getopt::Long::Descriptive;

my ($opt, $usage) = describe_options(
	$script_name.' %o <source-sqlite> <destination-file>',
	[ 'help|h'    => "print usage message and exit" ],
	[ 'version|v' => "print version and exit" ],
	[ 'lang|l=s'  => "language of labels (a language declared in the model)",
		{ default => '' } ],
	[ 'verbosity=i' => 'verbosity 0(default), 1, 2, 3',
		{ default => 0 } ],
);

if ($opt->version) {
	print $script_name.' version '.$script_version."\n";
	exit 0;
}

if ($opt->help) {
	print $usage->text;
	exit 0;
}

# Control verbosity of log output (0, 1, 2, 3)
my $verbosity = $opt->verbosity;

if ($#ARGV != 1) {
	# must have exactly two arguments
	print $usage->text;
	exit -1;
}

my $db = shift @ARGV;
my $workbook_xlsx = shift @ARGV;


my $lang_code_requested = $opt->lang;

use Excel::Writer::XLSX;
use File::Spec;
use File::Temp qw(tempdir);
use Capture::Tiny qw/capture tee capture_merged tee_merged/;

my $success;
my $failure;
my $retcode;
my $sql;
my $result;

use common qw(
				warning change error info
				sqlite3_exe
				logmsg
				logerrors
				run_sqlite_script
				run_sqlite_statement
				ompp_tables_to_csv
		    );
# Check for compile errors, and bail if any.
if ($@) {
	print $@;
	exit 1;
}

if (! -s $db) {
	logmsg error, $script_name, "SQLite database ${db} not found\n";
	exit 1;
}

# Remove existing output xlsx if present
if (-s $workbook_xlsx) {
	$failure = unlink $workbook_xlsx;
	if ($failure) {
		logmsg error, "unable to remove existing ${workbook_xlsx}";
		exit 1;
	}
}

# Extract csv versions of tables to a temporary folder
my $tmpdir = tempdir(CLEANUP => 1);

logmsg info, "excel_export", "extracting tables to ${tmpdir}" if $verbosity >= 1;
$failure = ompp_tables_to_csv($db, $tmpdir);
if ($failure) {
	logmsg error, $script_name, "unable to extract tables";
	exit 1;
};


my $Book = Excel::Writer::XLSX->new($workbook_xlsx);

###############
# SQLite Read #
###############


# Get model_id and model_name from model_dic
logmsg info, $script_name, "Get model_id and model_name from model_dic" if $verbosity >= 1;
$sql = "
  Select model_id, model_name
  From model_dic
  Where model_id = (Select Min(FM.model_id) From model_dic FM);
  ;
";
logmsg info, $script_name, "sql", $sql if $verbosity >= 2;
$result = run_sqlite_statement $db, $sql, $failure;
exit 1 if $failure;
chomp $result;
(my $model_id, my $model_name) = split(/[|]/, $result);
logmsg info, $script_name, "model_id=${model_id} model_name=${model_name}" if $verbosity >= 1;

# Set lang_id
my $lang_id;
if ($lang_code_requested ne "") {
	logmsg info, $script_name, "Get lang_id from lang_lst, for lang_code=${lang_code_requested}" if $verbosity >= 1;
	$sql = "
	  Select lang_id
	  From lang_lst
	  Where ( lang_code = '${lang_code_requested}' );
	  ;
	";
	logmsg info, $script_name, "sql", $sql if $verbosity >= 2;
	$result = run_sqlite_statement $db, $sql, $failure;
	exit 1 if $failure;
	logmsg info, $script_name, "result", $result if $verbosity >= 2;
	chomp $result;
	if ($result eq "") {
		logmsg error, $script_name, "Requested language code not found, using first language";
		# Continue anyway, using lang_id 0
		$lang_id = 0;
	}
	else {
		($lang_id) = split(/[|]/, $result);
	}
}
else {
	$lang_id = 0;  # default value is first language (lexicographical by language code).
}
logmsg info, $script_name, "lang_id=${lang_id}" if $verbosity >= 1;

# Get lang_code from lang_lst
logmsg info, $script_name, "Get lang_code from lang_lst" if $verbosity >= 1;
$sql = "
  Select lang_code
  From lang_lst
  Where ( lang_id = ${lang_id} );
  ;
";
logmsg info, $script_name, "sql", $sql if $verbosity >= 2;
$result = run_sqlite_statement $db, $sql, $failure;
exit 1 if $failure;
chomp $result;
(my $lang_code) = split(/[|]/, $result);
logmsg info, $script_name, "lang_code=${lang_code}" if $verbosity >= 1;

# Get table_ids, table_names, table_ranks from table_dic
logmsg info, $script_name, "Get table_ids, table_names, table_ranks from table_dic" if $verbosity >= 1;
$sql = "
  Select MT.model_table_id, TD.table_name, TD.table_rank
  From table_dic TD
  Inner Join model_table_dic MT ON (MT.table_hid = TD.table_hid)
  Where ( MT.model_id = ${model_id} )
  ;
";

logmsg info, $script_name, "sql", $sql if $verbosity >= 2;
$result = run_sqlite_statement $db, $sql, $failure;
exit 1 if $failure;
my @table_ids;
my @table_names;
my @table_ranks;
for my $record (split(/\n/, $result)) {
	(my $table_id, my $table_name, my $table_rank) = split(/[|]/, $record);
	push @table_ids, $table_id;
	$table_names[$table_id] = $table_name;
	$table_ranks[$table_id] = $table_rank;
}

# Get table_labels from table_dic_txt
logmsg info, $script_name, "Get table_labels from table_dic_txt" if $verbosity >= 1;
$sql = "
  Select MT.model_table_id, TXT.descr As table_description
  From model_table_dic MT
  Inner Join table_dic_txt TXT ON (TXT.table_hid = MT.table_hid)
  Where ( MT.model_id = ${model_id} And TXT.lang_id = ${lang_id} )
  ;
";

logmsg info, $script_name, "sql", $sql if $verbosity >= 2;
$result = run_sqlite_statement $db, $sql, $failure;
exit 1 if $failure;
my @table_labels;
for my $record (split(/\n/, $result)) {
	(my $table_id, my $table_description) = split(/[|]/, $record);
	$table_labels[$table_id] = $table_description;
}

# Get table_expr_sizes from table_expr
logmsg info, $script_name, "Get table_expr_sizes from table_expr" if $verbosity >= 1;
$sql = "
  Select MT.model_table_id, count(EX.expr_id) As table_expr_size
  From model_table_dic MT
  Inner Join table_expr EX ON (EX.table_hid = MT.table_hid)
  Where ( MT.model_id = ${model_id} )
  Group By MT.model_table_id";
logmsg info, $script_name, "sql", $sql if $verbosity >= 2;
$result = run_sqlite_statement $db, $sql, $failure;
exit 1 if $failure;
my @table_expr_sizes;
for my $record (split(/\n/, $result)) {
	(my $table_id, my $table_expr_size) = split(/[|]/, $record);
	$table_expr_sizes[$table_id] = $table_expr_size;
}

# Get table_expr_labels from table_expr_txt
logmsg info, $script_name, "Get table_expr_descriptions from table_expr_txt" if $verbosity >= 1;
$sql = "
  Select MT.model_table_id, TXT.expr_id AS expr_id, TXT.descr As table_expr_label
  From model_table_dic MT
  Inner Join table_expr_txt TXT ON (TXT.table_hid = MT.table_hid)
  Where (MT.model_id = ${model_id} And TXT.lang_id = ${lang_id} )
  ;
";
logmsg info, $script_name, "sql", $sql if $verbosity >= 2;
$result = run_sqlite_statement $db, $sql, $failure;
exit 1 if $failure;
my %table_expr_labels;
for my $record (split(/\n/, $result)) {
	(my $table_id, my $expr_id, my $table_expr_label) = split(/[|]/, $record);
	$table_expr_labels{$table_id, $expr_id} = $table_expr_label;
}

# Get table_dim_types and table_dim_margin from table_dims
logmsg info, $script_name, "Get table_dim_types and table_dim_margin from table_dims" if $verbosity >= 1;
$sql = "
  Select MT.model_table_id, D.dim_id, T.model_type_id As type_id, D.is_total As table_dim_margin
  From model_table_dic MT
  Inner Join table_dims D ON (D.table_hid = MT.table_hid)
  Inner Join model_type_dic T ON (T.model_id = MT.model_id AND T.type_hid = D.type_hid)
  Where ( MT.model_id = ${model_id} )
  ;
";
logmsg info, $script_name, "sql", $sql if $verbosity >= 2;
$result = run_sqlite_statement $db, $sql, $failure;
exit 1 if $failure;
my %table_dim_types;
my %table_dim_margin;
for my $record (split(/\n/, $result)) {
	(my $table_id, my $dim_id, my $type_id, my $table_dim_margin) = split(/[|]/, $record);
	$table_dim_types{$table_id, $dim_id} = $type_id;
	$table_dim_margin{$table_id, $dim_id} = $table_dim_margin;
}

# Get table_dim_labels from table_dims_txt
logmsg info, $script_name, "Get table_dim_labels from table_dims_txt" if $verbosity >= 1;
$sql = "
  Select MT.model_table_id, TXT.dim_id, TXT.descr As table_dim_label
  From model_table_dic MT
  Inner Join table_dims_txt TXT ON (TXT.table_hid = MT.table_hid)
  Where ( MT.model_id = ${model_id} And TXT.lang_id = ${lang_id} )
  ;
";
logmsg info, $script_name, "sql", $sql if $verbosity >= 2;
$result = run_sqlite_statement $db, $sql, $failure;
exit 1 if $failure;
logmsg info, $script_name, "result", $result if $verbosity >= 2;
my %table_dim_labels;
for my $record (split(/\n/, $result)) {
	(my $table_id, my $dim_id, my $table_dim_label) = split(/[|]/, $record);
	$table_dim_labels{$table_id, $dim_id} = $table_dim_label;
}

# Get enum_labels from type_enum_txt
logmsg info, $script_name, "Get enum_labels from type_enum_txt" if $verbosity >= 1;
$sql = "
  Select T.model_type_id As type_id, TXT.enum_id, TXT.descr As enum_label
  From model_type_dic T
  Inner Join type_enum_txt TXT ON (TXT.type_hid = T.type_hid)
  Where ( T.model_id = ${model_id} And TXT.lang_id = ${lang_id} )
  ;
";
logmsg info, $script_name, "sql", $sql if $verbosity >= 2;
$result = run_sqlite_statement $db, $sql, $failure;
exit 1 if $failure;
my %enum_labels;
for my $record (split(/\n/, $result)) {
	(my $type_id, my $enum_id, my $enum_label) = split(/[|]/, $record);
	$enum_labels{$type_id, $enum_id} = $enum_label;
}

# Create table_worksheet names by mangling table_names
my @table_worksheet_names;
my $worksheet_name_mangle_counter = 0;
for my $table_id (@table_ids) {
	my $table_worksheet_name = $table_names[$table_id];
	my $max_sheet_name_length = 31;
	if (length($table_worksheet_name) > $max_sheet_name_length) {
		# Truncate and mangle name to 31 characters (maximum Excel sheet name length).
		# Mangled name will look something like 'MyVeryLongNameWasBadlyTrunca#42'.
		# The '#' character cannot be part of a table name, so this approach is general,
		# works for any number of long names, and guarantees no clash of mangled names,
		# because the mangle counter is unique over all long table names.
		$worksheet_name_mangle_counter++;
		my $mangle_suffix = '#'.$worksheet_name_mangle_counter;
		$table_worksheet_name = substr($table_worksheet_name, 0, $max_sheet_name_length - length($mangle_suffix)).$mangle_suffix;
	}
	$table_worksheet_names[$table_id] = $table_worksheet_name; 
}

# Get parameter_ids, parameter_names from parameter_dic
logmsg info, $script_name, "Get parameter_ids, parameter_names from parameter_dic" if $verbosity >= 1;
$sql = "
  Select MP.model_parameter_id, P.parameter_name
  From parameter_dic P
  Inner Join model_parameter_dic MP ON (MP.parameter_hid = P.parameter_hid)
  Where ( MP.model_id = ${model_id} )
  ;
";
logmsg info, $script_name, "sql", $sql if $verbosity >= 2;
$result = run_sqlite_statement $db, $sql, $failure;
exit 1 if $failure;
my @parameter_ids;
my @parameter_names;
for my $record (split(/\n/, $result)) {
	(my $parameter_id, my $parameter_name) = split(/[|]/, $record);
	push @parameter_ids, $parameter_id;
	$parameter_names[$parameter_id] = $parameter_name;
}

# Get run_id, run_date_time, sub_count from run_lst
logmsg info, $script_name, "Get run_id, run_date_time, pieces from run_lst" if $verbosity >= 1;
$sql = "
  Select run_id, create_dt As run_date_time, sub_count As pieces
  From run_lst
  Where model_id = ${model_id} And run_id = (Select Min(RL.run_id) From run_lst RL);
  ;
";
logmsg info, $script_name, "sql", $sql if $verbosity >= 2;
$result = run_sqlite_statement $db, $sql, $failure;
exit 1 if $failure;
chomp $result;
logmsg info, $script_name, "result=${result}" if $verbosity >= 2;
if ($result eq '') {
	logmsg error, $script_name, "no runs in database";
	exit 1;
}
(my $run_id, my $run_date_time, my $pieces) = split(/[|]/, $result);
logmsg info, $script_name, "run_id=${run_id} run_date_time=${run_date_time} pieces=${pieces}" if $verbosity >= 1;

# Get set_id from run_option
logmsg info, $script_name, "Get set_id" if $verbosity >= 1;
$sql = "
  Select option_value From run_option Where run_id = ${run_id} And option_key = 'OpenM.SetId';
";
logmsg info, $script_name, "sql", $sql if $verbosity >= 2;
$result = run_sqlite_statement $db, $sql, $failure;
exit 1 if $failure;
chomp $result;
(my $set_id) = split(/[|]/, $result);
logmsg info, $script_name, "set_id=${set_id}" if $verbosity >= 1;

# Get set_name from run_option
logmsg info, $script_name, "Get set_id and set_name from run_option" if $verbosity >= 1;
$sql = "
  Select option_value From run_option Where run_id = ${run_id} And option_key = 'OpenM.SetName';
";
logmsg info, $script_name, "sql", $sql if $verbosity >= 2;
$result = run_sqlite_statement $db, $sql, $failure;
exit 1 if $failure;
chomp $result;
(my $set_name) = split(/[|]/, $result);
logmsg info, $script_name, "set_name=${set_name}" if $verbosity >= 1;

# Determine time-based or case-based using presence of specific ompp foundational parameters
my $time_based = 0;
my $case_based = 0;
for my $parameter_id (@parameter_ids) {
	my $parameter_name = $parameter_names[$parameter_id];
	$case_based = 1 if $parameter_name eq 'SimulationCases';
	$time_based = 1 if $parameter_name eq 'SimulationEnd';
}
if (!$case_based && !$time_based) {
	logmsg error, $script_name, "model is neither case-based nor time-based - unsupported";
	exit 1;
}

my $simulation_cases;
if ($case_based) {
	# Get simulation_cases from SimulationCases compatibility view
	logmsg info, $script_name, "Get simulation_cases from SimulationCases compatibility view" if $verbosity >= 1;
	$sql = "
	  Select Value As simulation_cases
	  From SimulationCases
	  ;
	";
	logmsg info, $script_name, "sql", $sql if $verbosity >= 2;
	$result = run_sqlite_statement $db, $sql, $failure;
	exit 1 if $failure;
	chomp $result;
	($simulation_cases) = split(/[|]/, $result);
	logmsg info, $script_name, "simulation_cases=${simulation_cases}" if $verbosity >= 1;
}

my $simulation_end;
if ($time_based) {
	# Get simulation_end from SimulationEnd compatibility view
	logmsg info, $script_name, "Get simulation_end from SimulationEnd compatibility view" if $verbosity >= 1;
	$sql = "
	  Select Value As simulation_end
	  From SimulationEnd
	  ;
	";
	logmsg info, $script_name, "sql", $sql if $verbosity >= 2;
	$result = run_sqlite_statement $db, $sql, $failure;
	exit 1 if $failure;
	chomp $result;
	(my $simulation_end) = split(/[|]/, $result);
	logmsg info, $script_name, "simulation_end=${simulation_end}" if $verbosity >= 1;
}

###############
# Excel write #
###############

my $PreviousSheet;
my $ContentsSheet;
{
	# Create the Contents sheet
	# Actually, just use the default 'Sheet1' which is already there, and rename it.
	my $Sheet = $Book->add_worksheet('Contents');
	$PreviousSheet = $Sheet;
	$ContentsSheet = $Sheet;
	$Sheet->write(0, 0, 'Table Name');
	$Sheet->write(0, 1, 'Table Description');
	$Sheet->write(0, 2, 'Worksheet names');
	my $row = 1;
	for my $table_id (@table_ids) {
		my $table_name = $table_names[$table_id];
		my $table_description = $table_labels[$table_id];
		my $table_worksheet_name = $table_worksheet_names[$table_id];
		$Sheet->write($row, 0, $table_name);
		$Sheet->write($row, 1, $table_description);
		$Sheet->write($row, 2, $table_worksheet_name);
		++$row;
	}
}

{
	# Create the Scenario Information sheet
	my $Sheet = $Book->add_worksheet('Scenario Information');
	$Sheet->write(0, 0, 'Directory');
	$Sheet->write(0, 1, '.');
	$Sheet->write(1, 0, 'Command Line');
	$Sheet->write(1, 1, "${model_name}.exe -sc ${set_name}.scex -s");
	$Sheet->write(2, 0, 'Full Report');
	$Sheet->write(2, 1, 'TRUE');
	$Sheet->write(3, 0, $case_based ? 'Cases' : 'Replicates');
	$Sheet->write(3, 1, $case_based ? $simulation_cases : $pieces);
	$Sheet->write(4, 0, $case_based ? 'Cases Requested' : 'Replicates Requested');
	$Sheet->write(4, 1, $case_based ? $simulation_cases : $pieces);
	$Sheet->write(5, 0, 'Language');
	$Sheet->write(5, 1, $lang_code);
	$Sheet->write(6, 0, 'coefficient of variation values (%)');
	$Sheet->write(6, 1, 'FALSE');
	$Sheet->write(7, 0, 'standard error values');
	$Sheet->write(7, 1, 'FALSE');
	$Sheet->write(8, 0, 'Simulation date and time');
	my $format = $Book->add_format();
	$format->set_num_format('yyyy/mm/dd h:mm:ss');
	$Sheet->write(8, 1, $run_date_time, $format);
	$Sheet->write(9, 0, $case_based ? 'Subsamples:' : 'Replicates:');
	#$Sheet->write(9, 1, ''); # Deliberately empty
}

# Create the table sheets
for my $table_id (@table_ids) {

	my $table_name = $table_names[$table_id];
	my $table_worksheet_name = $table_worksheet_names[$table_id];
	my $table_rank = $table_ranks[$table_id];
	my $table_expr_size = $table_expr_sizes[$table_id];
	logmsg info, $script_name, "table_name=${table_name} table_rank=${table_rank} table_expr_size=${table_expr_size}" if $verbosity >= 1;
	
	# Create a worksheet for the output table
	my $Sheet = $Book->add_worksheet($table_worksheet_name);
	$PreviousSheet = $Sheet;
	
	# Column headers
	# Classification dimensions
	if ($table_rank > 0) {
		for my $dim_id (0..$table_rank - 1) {
			my $col = $dim_id;
			my $table_dim_label = $table_dim_labels{$table_id, $dim_id};
			$Sheet->write(0, $col, $table_dim_label);
		}
	}
	# Expression dimension
	for my $expr_id (0..$table_expr_size - 1) {
		my $col = $table_rank + $expr_id;
		my $table_expr_label = $table_expr_labels{$table_id, $expr_id};
		$Sheet->write(0, $col, $table_expr_label);
	}
	
	# Get table data from csv and insert into sheet
	# Note that multiple records are read into one 'observation' which includes all expressions in the table
	$retcode = open CSV, "<${tmpdir}/${table_name}.csv";
	if (!defined($retcode)) {
		logmsg error, $script_name, "unable to open ${tmpdir}/${table_name}.csv, retcode=${retcode}";
		exit 1;
	}
	my $record_counter = 0; # value of 0 will skip header row in csv
	my $observation_counter = 0; # counter ignoring expression dimension
	while (<CSV>) {
		chomp;
		my $line = $_;
		logmsg info, $script_name, $table_name, "line=$line" if $verbosity >= 3;
		++$record_counter;
		next if $record_counter == 1; # skip csv header line containing column names

		my @fields = split(",", $line);
		my $expr_id = $fields[$table_rank] + 0;
		my $value = $fields[$table_rank + 1];
		my $value_missing = 0;
		if ( !defined($value) || $value eq '') {
			$value_missing = 1;
		}
		else {
			$value += 0; # force to numeric
		}

		logmsg info, $script_name, $table_name, "expr_id=${expr_id} value=${value}" if $verbosity >= 3;
		
		if ($expr_id == 0) {
			# New observation
			++$observation_counter;
			my $row = $observation_counter;
			logmsg info, $script_name, $table_name, "observation_counter=${observation_counter} row=${row}" if $verbosity >= 3;
			# Classification dimensions for this observation
			if ($table_rank > 0) {
				for my $dim_id (0..$table_rank - 1) {
					my $type_id = $table_dim_types{$table_id, $dim_id};
					my $col = $dim_id;
					my $enum_id = $fields[$dim_id] + 0;
					my $enum_label = $enum_labels{$type_id, $enum_id};
					$Sheet->write($row, $col, $enum_label);
				}
			}
		}
		# Expression dimension
		if (!$value_missing) {
			my $row = $observation_counter;
			my $col = $table_rank + $expr_id;
			logmsg info, $script_name, $table_name, "row=${row} col=${col} value=${value}" if $verbosity >= 3;
			$Sheet->write($row, $col, $value);
		}
	}
	close CSV;
}

rmdir $tmpdir;

exit 0; # success