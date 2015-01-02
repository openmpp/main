# Copyright (c) 2013-2014 OpenM++
# This code is licensed under MIT license (see LICENSE.txt for details)

# Script to export the output tables of a SQLite scenario to Excel in pivot table format

use strict;
use Win32::OLE qw(in with);
use Win32::OLE::Const 'Microsoft Excel';
use Win32::OLE::Const 'Microsoft ActiveX Data Objects';
use File::Spec;
use File::Temp qw(tempdir);
use Capture::Tiny qw/capture tee capture_merged tee_merged/;

my $version = 0.2;

if ( $#ARGV == -1 ) {
	print "excel_export version $version\n";
	print "usage: excel_export [-lang lang_code] in_slqite out_xlsx\n";
	print "  example: excel_export RiskPaths.sqlite Base(tbl).xlsx\n";
	print "  example: excel_export -lang FR RiskPaths.sqlite Base(tbl).xlsx\n";
	exit;
}

# Control verbosity of log output (0, 1, 2)
my $verbosity = 0;

my $script_name = "excel_export";
my $lang_code_requested = "";
if (@ARGV[0] eq "-lang") {
	shift @ARGV;
	$lang_code_requested = shift @ARGV;
}
if ( $#ARGV != 1 ) {
	# must have two arguments
	print "excel_export version $version\n";
	print "usage: excel_export [-lang lang_code] in_slqite out_xlsx\n";
	print "  example: excel_export RiskPaths.sqlite Base(tbl).xlsx\n";
	print "  example: excel_export -lang FR RiskPaths.sqlite Base(tbl).xlsx\n";
	exit;
}

my $db = shift @ARGV;
my $workbook_xlsx = shift @ARGV;
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
	exit;
}

if (!-s $db) {
	logmsg error, $script_name, "SQLite database ${db} not found\n";
}
if (-s $workbook_xlsx) {
	unlink $workbook_xlsx || logmsg error, $script_name, "unable to remove existing ${workbook_xlsx}\n";
}

# Get already active Excel application or open new
my $Excel = Win32::OLE->GetActiveObject('Excel.Application')
		   || Win32::OLE->new('Excel.Application', 'Quit');

if (!$Excel) {
	logmsg error, $script_name, "Excel not available";
	exit 1;
}
		   
# Extract csv versions of tables to a temporary folder
my $tmpdir = tempdir(CLEANUP => 1);
#my $tmpdir = tempdir();
# Change slashes from \ to / for sqlite3
$tmpdir =~ s@[\\]@/@g;

logmsg info, "excel_export", "extracting tables to ${tmpdir}" if $verbosity >= 1;
$failure = ompp_tables_to_csv($db, $tmpdir);
if ($failure) {
	logmsg error, $script_name, "unable to extract tables";
	exit 1
};
 
# Turn off those many irritating are-you-sure? Excel dialogs
$Excel->{DisplayAlerts} = 0;

if (-s $workbook_xlsx) {
	$failure = unlink $workbook_xlsx;
	if ($failure) {
		logmsg error, "unable to remove existing ${workbook_xlsx}";
		exit 1;
	}
}
# make xlsx file absolute, otherwise Excel saves in 'Documents' folder
$workbook_xlsx = File::Spec->rel2abs($workbook_xlsx);

Win32::OLE->Option(Warn => 3);

# Create a new Excel workbook
my $Book = $Excel->Workbooks->Add();
$Book->SaveAs($workbook_xlsx);

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
my $model_id;
my $model_name;
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
my $lang_code;
chomp $result;
(my $lang_code) = split(/[|]/, $result);
logmsg info, $script_name, "lang_code=${lang_code}" if $verbosity >= 1;

# Get table_ids, table_names, table_ranks from table_dic
logmsg info, $script_name, "Get table_ids, table_names, table_ranks from table_dic" if $verbosity >= 1;
$sql = "
  Select table_id, table_name, table_rank
  From table_dic
  Where ( model_id = ${model_id} )
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
  Select table_id, descr As table_description
  From table_dic_txt
  Where ( model_id = ${model_id} And lang_id = ${lang_id} )
  ;
";
logmsg info, $script_name, "sql", $sql if $verbosity >= 2;
my $result = run_sqlite_statement $db, $sql, $failure;
exit 1 if $failure;
my @table_labels;
for my $record (split(/\n/, $result)) {
	(my $table_id, my $table_description) = split(/[|]/, $record);
	$table_labels[$table_id] = $table_description;
}

# Get table_expr_sizes from table_expr
logmsg info, $script_name, "Get table_expr_sizes from table_expr" if $verbosity >= 1;
$sql = "
  Select table_id, count(expr_id) As table_expr_size
  From table_expr
  Where ( model_id = ${model_id} )
  Group By table_id";
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
  Select table_id, expr_id AS expr_id, descr As table_expr_label
  From table_expr_txt
  Where (model_id = ${model_id} And lang_id = ${lang_id} )
  ;
";
logmsg info, $script_name, "sql", $sql if $verbosity >= 2;
my $result = run_sqlite_statement $db, $sql, $failure;
exit 1 if $failure;
my %table_expr_labels;
for my $record (split(/\n/, $result)) {
	(my $table_id, my $expr_id, my $table_expr_label) = split(/[|]/, $record);
	$table_expr_labels{$table_id, $expr_id} = $table_expr_label;
}

# Get table_dim_labels from table_dims_txt
logmsg info, $script_name, "Get table_dim_labels from table_dims_txt" if $verbosity >= 1;
$sql = "
  Select table_id, dim_name, descr As table_dim_label
  From table_dims_txt
  Where ( model_id = ${model_id} And lang_id = ${lang_id} )
  ;
";
logmsg info, $script_name, "sql", $sql if $verbosity >= 2;
my $result = run_sqlite_statement $db, $sql, $failure;
exit 1 if $failure;
my %table_dim_labels;
for my $record (split(/\n/, $result)) {
	(my $table_id, my $dim_name, my $table_dim_label) = split(/[|]/, $record);
	my $dim_id = substr($dim_name, 3) + 0;
	$table_dim_labels{$table_id, $dim_id} = $table_dim_label;
}

# Get table_dim_types and table_dim_margin from table_dims
logmsg info, $script_name, "Get table_dim_types and table_dim_margin from table_dims" if $verbosity >= 1;
$sql = "
  Select table_id, dim_pos As dim_id, mod_type_id As type_id, is_total As table_dim_margin
  From table_dims
  Where ( model_id = ${model_id} )
  ;
";
logmsg info, $script_name, "sql", $sql if $verbosity >= 2;
my $result = run_sqlite_statement $db, $sql, $failure;
exit 1 if $failure;
my %table_dim_types;
my %table_dim_margin;
for my $record (split(/\n/, $result)) {
	(my $table_id, my $dim_id, my $type_id, my $table_dim_margin) = split(/[|]/, $record);
	$table_dim_types{$table_id, $dim_id} = $type_id;
	$table_dim_margin{$table_id, $dim_id} = $table_dim_margin;
}

# Get enum_labels from type_enum_txt
logmsg info, $script_name, "Get enum_labels from type_enum_txt" if $verbosity >= 1;
$sql = "
  Select mod_type_id As type_id, enum_id, descr As enum_label
  From type_enum_txt
  Where ( model_id = ${model_id} And lang_id = ${lang_id} )
  ;
";
logmsg info, $script_name, "sql", $sql if $verbosity >= 2;
my $result = run_sqlite_statement $db, $sql, $failure;
exit 1 if $failure;
my %enum_labels;
for my $record (split(/\n/, $result)) {
	(my $type_id, my $enum_id, my $enum_label) = split(/[|]/, $record);
	$enum_labels{$type_id, $enum_id} = $enum_label;
}

# Create table_worksheet names by mangling table_names
my @table_worksheet_names;
for my $table_id (@table_ids) {
	my $table_worksheet_name = @table_names[$table_id];
	# TODO - mangle name to shorten
	$table_worksheet_names[$table_id] = $table_worksheet_name; 
}

# Get parameter_ids, parameter_names from parameter_dic
logmsg info, $script_name, "Get parameter_ids, parameter_names from parameter_dic" if $verbosity >= 1;
$sql = "
  Select parameter_id, parameter_name
  From parameter_dic
  Where ( model_id = ${model_id} )
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
my $run_id;
my $run_date_time;
my $pieces;
chomp $result;
(my $run_id, my $run_date_time, my $pieces) = split(/[|]/, $result);
logmsg info, $script_name, "run_id=${run_id} run_date_time=${run_date_time} pieces=${pieces}" if $verbosity >= 1;

# Get set_id and set_name from workset_lst
logmsg info, $script_name, "Get set_id and set_name from workset_lst" if $verbosity >= 1;
$sql = "
  Select set_id, set_name
  From workset_lst
  Where model_id = ${model_id} And set_id = (Select Min(WL.set_id) From workset_lst WL);
  ;
";
logmsg info, $script_name, "sql", $sql if $verbosity >= 2;
$result = run_sqlite_statement $db, $sql, $failure;
exit 1 if $failure;
my $set_id;
my $set_name;
chomp $result;
(my $set_id, my $set_name) = split(/[|]/, $result);
logmsg info, $script_name, "set_id=${set_id} set_name=${set_name}" if $verbosity >= 1;

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
	my $Sheet = $Book->ActiveSheet;
	$PreviousSheet = $Sheet;
	$Sheet->{Name} = 'Contents';
	$ContentsSheet = $Sheet;
	$Sheet->Cells(1, 1)->{Value} = 'Table Name';
	$Sheet->Cells(1, 2)->{Value} = 'Table Description';
	$Sheet->Cells(1, 3)->{Value} = 'Worksheet names';
	my $row = 2;
	for my $table_id (@table_ids) {
		my $table_name = $table_names[$table_id];
		my $table_description = $table_labels[$table_id];
		my $table_worksheet_name = $table_worksheet_names[$table_id];
		$Sheet->Cells($row, 1)->{Value} = $table_name;
		$Sheet->Cells($row, 2)->{Value} = $table_description;
		$Sheet->Cells($row, 3)->{Value} = $table_worksheet_name;
		++$row;
	}
	$Sheet->Columns("A:C")->Autofit;
}

{
	# Create the Scenario Information sheet
	$Book->Sheets->Add({after => $PreviousSheet});
	my $Sheet = $Book->ActiveSheet;
	$PreviousSheet = $Sheet;
	$Sheet->{Name} = "Scenario Information";
	$Sheet->Cells(1, 1)->{Value} = 'Directory';
	$Sheet->Cells(1, 2)->{Value} = '.';
	$Sheet->Cells(2, 1)->{Value} = 'Command Line';
	$Sheet->Cells(2, 2)->{Value} = "${model_name}.exe -sc ${set_name}.scex -s";
	$Sheet->Cells(3, 1)->{Value} = 'Full Report';
	$Sheet->Cells(3, 2)->{Value} = 'TRUE';
	$Sheet->Cells(4, 1)->{Value} = $case_based ? 'Cases' : 'Replicates';
	$Sheet->Cells(4, 2)->{Value} = $case_based ? $simulation_cases : $pieces;
	$Sheet->Cells(5, 1)->{Value} = $case_based ? 'Cases Requested' : 'Replicates Requested';
	$Sheet->Cells(5, 2)->{Value} = $case_based ? $simulation_cases : $pieces;
	$Sheet->Cells(6, 1)->{Value} = 'Language';
	$Sheet->Cells(6, 2)->{Value} = $lang_code;
	$Sheet->Cells(7, 1)->{Value} = 'coefficient of variation values (%)';
	$Sheet->Cells(7, 2)->{Value} = 'FALSE';
	$Sheet->Cells(8, 1)->{Value} = 'standard error values';
	$Sheet->Cells(8, 2)->{Value} = 'FALSE';
	$Sheet->Cells(9, 1)->{Value} = 'Simulation date and time';
	$Sheet->Cells(9, 2)->{Value} = $run_date_time;
	$Sheet->Cells(9, 2)->{NumberFormat} = 'yyyy/mm/dd h:mm:ss';
	$Sheet->Cells(10, 1)->{Value} = $case_based ? 'Subsamples:' : 'Replicates:';
	#$Sheet->Cells(10, 2)->{Value} = ''; # Deliberately empty
	$Sheet->Columns("A:B")->Autofit;
}

# Create the table sheets
for my $table_id (@table_ids) {

	my $table_name = $table_names[$table_id];
	my $table_worksheet_name = $table_worksheet_names[$table_id];
	my $table_rank = $table_ranks[$table_id];
	my $table_expr_size = $table_expr_sizes[$table_id];
	logmsg info, $script_name, "table_name=${table_name} table_rank=${table_rank} table_expr_size=${table_expr_size}" if $verbosity >= 1;
	
	# Create a worksheet for the output table
	$Book->Sheets->Add({after => $PreviousSheet});
	my $Sheet = $Book->ActiveSheet;
	$PreviousSheet = $Sheet;
	$Sheet->{Name} = $table_worksheet_name;
	
	# Column headers
	# Classification dimensions
	if ($table_rank > 0) {
		for my $dim_id (0..$table_rank - 1) {
			my $col = $dim_id + 1;
			my $table_dim_label = $table_dim_labels{$table_id, $dim_id};
			$Sheet->Cells(1, $col)->{Value} = $table_dim_label;
		}
	}
	# Expression dimension
	for my $expr_id (0..$table_expr_size - 1) {
		my $col = 1 + $table_rank + $expr_id;
		my $table_expr_label = $table_expr_labels{$table_id, $expr_id};
		$Sheet->Cells(1, $col)->{Value} = $table_expr_label;
	}
	
	# Get table data from csv and insert into sheet
	# Note that multiple records are read into one 'observation' which includes all expressions in the table
	$retcode = open CSV, "<${tmpdir}/${table_name}.csv";
	if ($retcode == undef) {
		logmsg error, $script_name, "unable to open ${tmpdir}/${table_name}.csv, retcode=${retcode}";
		exit 1;
	}
	my $record_counter = 0; # value of 0 will skip header row in csv
	my $observation_counter = 0; # counter ignoring expression dimension
	while (<CSV>) {
		chomp;
		my $line = $_;
		logmsg info, $script_name, $table_name, "line=$line" if $verbosity >= 2;
		++$record_counter;
		next if $record_counter == 1; # skip csv header line containing column names

		my @fields = split(",", $line);
		my $expr_id = $fields[$table_rank] + 0;
		my $value = $fields[$table_rank + 1];
		my $value_missing = 0;
		if ( $value eq '') {
			$value_missing = 1;
		}
		else {
			$value += 0; # force to numeric
		}

		logmsg info, $script_name, $table_name, "expr_id=${expr_id} value=${value}" if $verbosity >= 2;
		
		if ($expr_id == 0) {
			# New observation
			++$observation_counter;
			my $row = $observation_counter + 1;
			logmsg info, $script_name, $table_name, "observation_counter=${observation_counter} row=${row}" if $verbosity >= 2;
			# Classification dimensions for this observation
			if ($table_rank > 0) {
				for my $dim_id (0..$table_rank - 1) {
					my $type_id = $table_dim_types{$table_id, $dim_id};
					my $col = $dim_id + 1;
					my $enum_id = $fields[$dim_id] + 0;
					my $enum_label = $enum_labels{$type_id, $enum_id};
					$Sheet->Cells($row, $col)->{Value} = $enum_label;
				}
			}
		}
		# Expression dimension
		if (!$value_missing) {
			my $row = $observation_counter + 1;
			my $col = 1 + $table_rank + $expr_id;
			logmsg info, $script_name, $table_name, "row=${row} col=${col} value=${value}" if $verbosity >= 2;
			$Sheet->Cells($row, $col)->{Value} = $value;
		}
	}
	close CSV;
}

$ContentsSheet->Activate;
$Book->Save();
#$Book->Close();
rmdir $tmpdir;

exit 0; # success