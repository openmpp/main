# Copyright (c) 2013-2014 OpenM++
# This code is licensed under MIT license (see LICENSE.txt for details)

# Common shared components for ompp Perl utilities, e.g. test_models.pl and friends 
#
# Inventory:
#
# $sqlite3_exe
#
# logmsg()
# logerrors()
# run_sqlite_statement()
# run_sqlite_script()
# ompp_tables_to_csv()
# create_digest()
#

use strict;

# Strings used as line prefixes in output
sub warning {
	return " Warning => ";
}
sub change {
	return " Change ==> ";
}
sub error {
	return " Error ===> ";
}
sub info {
	return "            ";
}


# Steps to install Capture::Tiny (this method requires internet connectivity, there are other ways)
# (1) open command prompt
# (2) find and run vsvars32.bat (to get path to nmake into environment)
#     Normally some place like
#     "C:\Program Files (x86)\Microsoft Visual Studio 10.0\Common7\Tools\vsvars32.bat"
# (3) perl -MCPAN -e "install Capture::Tiny"
use Capture::Tiny qw/capture tee capture_merged tee_merged/;

# Output a log message with one or more prefixes.
# arg0 - left margin prefix for warning/change/error/info
# arg1... - optional additional prefixes, each to be followed by ':'
# argN - the message, possibly multi-line
# returns - nothing
sub logmsg {
	my $prefix;

	# first argument is margin prefix, which has no trailing ":"
	$prefix = shift(@_);
	# build prefix using all arguments except last
	while (0 != $#_) {
		$prefix .= shift(@_).": ";
	}

	# prepend prefix to each line in msg, and print
	my $msg = shift(@_);
	for my $line (split(/\n/, $msg)) {
		print "${prefix}${line}\n";
	}
}

# Extract errors from multi-line string and pass to logmsg for output
# arg0 - multi-line string
# returns - nothing
sub logerrors {
	my @lines = split(/\n/, @_[0]);
	for (@lines) {
		if (/error/i) {
			my $line = $_;
			# trim leading white-space
			$line =~ s/^\s*//;
			# if line starts with path portion, remove it
			$line =~ s/^[A-Z]:[^:]*[\\]//;
			# if line is too long, truncate it
			if (length($line) > 130) {
				$line = substr($line, 0, 125)." ...";
			}
			logmsg info, "         ${line}";
		}
	}
}

# SQLite3 reference page
# http://sqlite.org/cli.html

# Global path to sqlite3 executable
use File::Basename 'dirname';
my $sqlite3_exe = File::Spec->catfile(dirname($0), "../bin/sqlite3.exe");
if ( ! -e $sqlite3_exe ) {
	logmsg error, "Missing SQLite: $sqlite3_exe";
	exit 1;
}
use Cwd 'abs_path';
$sqlite3_exe = abs_path($sqlite3_exe);
sub sqlite3_exe {
	return $sqlite3_exe;
}

# Run a SQL script on a SQLite database
# arg0 - the SQLite database
# arg1 - the file of SQL statements to run
# arg2 - the return value from SQLite3
# returns - multi-line string
sub run_sqlite_script {
	my $db = shift(@_);
	my $sql = shift(@_);
	(my $merged, my $retval) = capture_merged {
		my @args = (
			"${sqlite3_exe}",
			"${db}",
			".read ${sql}",
			);
		system(@args);
	};
	if ($retval != 0 ) {
		logmsg error, "run_sqlite_script failed";
		logmsg error, "script: $sql";
		logmsg error, "SQLite output follows";
		logerrors $merged;
	}
	# Place return value from sqlite3 into final argument
	@_[0] = $retval;
	return $merged;
}

# Run a SQL statement on a SQLite database
# arg0 - the SQLite database
# arg1 - the SQL statement
# arg2 - the return value from SQLite3
# returns - multi-line string
sub run_sqlite_statement {
	my $db = shift(@_);
	my $sql = shift(@_);
	
	(my $merged, my $retval) = capture_merged {
		my @args = (
			"${sqlite3_exe}",
			"${db}",
			"${sql}",
			);
		system(@args);
	};
	if ($retval != 0 ) {
		logmsg error, "run_sqlite_statement failed";
		logmsg error, "SQL: $sql";
		logmsg error, "SQLite output follows";
		logerrors $merged;
	}
	# Place return value from sqlite3 into final argument
	@_[0] = $retval;
	return $merged;
}

# Extract ompp output tables from a ompp SQLite database to a folder
# arg0 - the SQLite database
# arg1 - the destination folder
# arg2 - the number of significant digits to output (optional)
# returns - 0 for success, otherwise non-zero
sub ompp_tables_to_csv
{
	my $db = shift(@_);
	my $dir = shift(@_);
	my $retval;
	my $round_value = 0;
	my $round_prec = 0;
	if ($#_ == 0) {
		$round_prec = shift(@_) - 1;
		if ($round_prec > 0) {
			$round_value = 1;
		}
	}

	if (! -d $dir) {
		if (!mkdir $dir) {
			logmsg error, "unable to create directory ${dir}";
			return 1;
		}
	}

	# Get all of the output table names
	my $tables = run_sqlite_statement $db, "Select table_name From table_dic Order By table_name;", $retval;
	if ($retval != 0) {
		return $retval;
	}
	
	# Create SQLite script in temporary file to extract all result tables
	my $temp_sql = "${dir}/ompp_tables_to_csv.sql";
	if (!open TEMP_SQL, ">${temp_sql}") {
		logmsg error, "unable to create directory ${dir}";
		return 1;
	};
	print TEMP_SQL ".mode csv\n";
	print TEMP_SQL ".headers on\n";
	for my $table (split(/\n/, $tables)) {
		print TEMP_SQL ".output ${dir}/in_${table}.csv\n";
		print TEMP_SQL "Select * From ${table};\n";
	}
	close TEMP_SQL;
	
	# Run the script
	my $merged = run_sqlite_script $db, $temp_sql, $retval;
	if ($retval != 0) {
		# In case of failure, create log for debugging
		my $temp_sql_log = "${temp_sql}.log";
		return 3 if !open TEMP_SQL_LOG, ">${temp_sql_log}";
		print TEMP_SQL_LOG $merged;
		close TEMP_SQL_LOG;
		return $retval;
	}

	# Normalize the numeric value (last field in each line) for comparison purposes
	for my $table (split(/\n/, $tables)) {
		my $in_csv = "${dir}/in_${table}.csv";
		if (!open IN_CSV, "<${in_csv}") {
			logmsg error, "unable to open ${in_csv}";
			return 1;
		};
		my $out_csv = "${dir}/${table}.csv";
		if (!open OUT_CSV, ">${out_csv}") {
			logmsg error, "unable to open ${out_csv}";
			return 1;
		};
		my $header_line = 1;
		while (<IN_CSV>) {
			if ($header_line) {
				print OUT_CSV;
				$header_line = 0;
				next;
			}
			chomp;
			my $line = $_;
			if (length($line) && substr($line, -1, 1) ne ',') {
				# value field is not empty, process it
				my @fields = split /,/, $line;
				# get the last (value) field
				my $value = pop @fields;
				$value = $value + 0.0;
				if ($round_value) {
					$value = 0.0 + sprintf("%.${round_prec}e", $value);
				}
				push @fields, $value;
				$line = join(',', @fields);
			}
			print OUT_CSV $line."\n";
		}
		close IN_CSV;
		close OUT_CSV;
		unlink $in_csv;
	}
	
	# Remove temporary file
	unlink $temp_sql;

	# Success
	return 0;
}


# Run a SQL statement on a Jet (.mdb) database
# arg0 - the Jet database
# arg1 - the SQL statement
# arg2 - the return value from 
# returns - multi-line string
sub run_jet_statement {
	my $db = shift(@_);
	my $sql = shift(@_);
	my $result;

	use Win32::OLE;
	use Win32::OLE::Const 'Microsoft ActiveX Data Objects';

	my $sConnect = "Provider = Microsoft.Jet.OLEDB.4.0; Data source = ${db}";
	#my $sConnect = "Provider = Microsoft.ACE.OLEDB.12.0; Data source = ${db}";
	my $ADO_Conn = Win32::OLE->new('ADODB.Connection');	# creates a connection object
	my $ADO_RS = Win32::OLE->new('ADODB.Recordset');		# creates a recordset object
	$ADO_Conn->Open($sConnect);
	if (Win32::OLE->LastError()) {
		print "Fatal Error: ", Win32::OLE->LastError(), "\n";
		return 1;
	}

	$ADO_RS = $ADO_Conn->Execute($sql);
	if (Win32::OLE->LastError()) {
		print "Fatal Error: ", Win32::OLE->LastError(), "\n";
		return 1;
	}

	my $fields = $ADO_RS->Fields->count;
	while ( !$ADO_RS->EOF ) {
		for (my $field_ordinal = 0; $field_ordinal < $fields; $field_ordinal++) {
			my $value = $ADO_RS->Fields($field_ordinal)->value;
			$result .= $value;
			if ($field_ordinal < $fields - 1) {
				$result .= ","
			}
		}
		$result .= "\n";
		#my $value = $ADO_RS->Fields(0)->value;
		#$result .= "${value}\n";
		$ADO_RS->MoveNext;
	}
	
	return $result;
}


# Create a Modgen .scex file
# arg0 - the output file name
# arg1 - the ompp Base(Framework).odat file
# arg2 - the ompp ompp_framework.ompp file
# argN - remaining arguments are .dat files
# returns - 0 for success, otherwise non-zero
sub modgen_create_scex
{
	my $scex_file      = shift(@_);
	my $framework_odat_file = shift(@_);
	my $framework_ompp_file = shift(@_);

	my @dat_files = @_;

	# Default values for scenario settings
	my %General = (
		# case-based
		Cases => 5000,
		# time-based
		SimulationEnd => 100,
		# common to both
		Subsamples => 1,
		PopulationScaling => 0,
		Population => 5000,
		Threads => 1,
		PartialReports => 0,
		StartingSeed => 16807,
		ComputationPriority => "THREAD_PRIORITY_BELOW_NORMAL",
		CancelPartialResults => "PARTIAL_NO",
		BackgroundMode => 0,
		SilentMode => 1,
		DistributedExecution => 0,
		DistributedSamples => 1,
		AccessTracking => 0,
		MaxTrackedCases => 50,
		TextTracking => 0,
		CopyParameters => 0,
		MemoryReports => 0,
	);

	# Parse Base ompp framework code file for .scex scenario information
	if (!open FRAMEWORK_OMPP, "<${framework_ompp_file}") {
		logmsg error, "unable to open ${framework_ompp_file}";
		return 1;
	}
	while (<FRAMEWORK_OMPP>) {
		chomp;
		my $line = $_;
		# Look for which scaling module has been used
		# Search pattern starts with 'use' to avoid picking up // commented lines
		if ( $line =~ /^use.*scaling_none/ ) {
			$General{"PopulationScaling"} = 0;
			$General{"Population"} = 0;
		}
		if ( $line =~ /^use.*scaling_endogenous/ ) {
			$General{"PopulationScaling"} = 1;
			$General{"Population"} = 0;
		}
		if ( $line =~ /^use.*scaling_exogenous/ ) {
			$General{"PopulationScaling"} = 1;
			$General{"Population"} = 0; # Will be replaced using value of parameter 
		}
	}
	close FRAMEWORK_OMPP;

	# Parse Base Framework parameters for .scex scenario information
	if (!open FRAMEWORK_ODAT, "<${framework_odat_file}") {
		logmsg error, "unable to open ${framework_odat_file}";
		return 1;
	}
	while (<FRAMEWORK_ODAT>) {
		chomp;
		my $line = $_;
		if ( $line =~ /SimulationSeed\s*=\s*(\d+)/ ) {
			$General{"StartingSeed"} = $1;
		}
		if ( $line =~ /SimulationCases\s*=\s*(\d+)/ ) {
			$General{"Cases"} = $1;
		}
		if ( $line =~ /SimulatedPopulation\s*=\s*(\d+[.]?\d+)/ ) {
			$General{"Population"} = $1;
		}
		if ( $line =~ /SimulationEnd\s*=\s*(\d+[.]?\d+)/ ) {
			$General{"SimulationEnd"} = $1;
		}
	}
	close FRAMEWORK_ODAT;

	# The XML structure of the .scex file is simple,
	# so the approach here is to generate it directly rather than use XML::Tiny or XML::LibXML, etc.

	if (!open SCEX, ">${scex_file}") {
		logmsg error, "unable to open ${scex_file}";
		return 1;
	}
	print SCEX "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	print SCEX "<Scenario ScenarioVersion=\"1.8\">\n";
	print SCEX "  <General\n";
	# Iterate all General attributes (sort for output readability)
	my @setting = sort keys %General;
	for my $setting (@setting){
		print SCEX "    $setting=\"".$General{$setting}."\"\n";
	}
	print SCEX "  >\n";
	print SCEX "  </General>\n";
	print SCEX "  <Inputs>\n";
	# Iterate .dat files
	for my $file (@dat_files) {
		print SCEX "    <Input>${file}=</Input>\n";
	}
	print SCEX "  </Inputs>\n";
	print SCEX "</Scenario>\n";
	close SCEX;
	return 0;
}


# Extract Modgen output tables from a mdb Modgen database to a folder
# arg0 - the Modgen database
# arg1 - the destination folder
# arg2 - the number of significant digits to output (optional)
# returns - 0 for success, otherwise non-zero
sub modgen_tables_to_csv
{
	my $db = shift(@_);
	my $dir = shift(@_);
	my $round_value = 0;
	my $round_prec = 0;
	if ($#_ == 0) {
		$round_prec = shift(@_) - 1;
		if ($round_prec > 0) {
			$round_value = 1;
		}
	}
	my $retval;
	
	my $suppress_margins = 1;

	if (! -d $dir) {
		if (!mkdir $dir) {
			logmsg error, "unable to create directory ${dir}";
			return 1;
		}
	}
	
	use Win32::OLE;
	use Win32::OLE::Const 'Microsoft ActiveX Data Objects';
	my $sConnect = "Provider = Microsoft.Jet.OLEDB.4.0; Data source = ${db}";
	#my $sConnect = "Provider = Microsoft.ACE.OLEDB.12.0; Data source = ${db}";
	my $ADO_Conn = Win32::OLE->new('ADODB.Connection');	# creates a connection object

	$ADO_Conn->Open($sConnect);
	if (Win32::OLE->LastError()) {
		logmsg error, "OLE", Win32::OLE->LastError();
		return 1;
	}

	# Get all of the output table names
	my $ADO_RS = Win32::OLE->new('ADODB.Recordset');
	my $sql = "Select Name, Rank, AnalysisDimensionPosition, TableID From TableDic Where LanguageID = 0;";
	$ADO_RS = $ADO_Conn->Execute($sql);
	if (Win32::OLE->LastError()) {
		logmsg error, "OLE", Win32::OLE->LastError();
		return 1;
	}
	my $fields = $ADO_RS->Fields->count;
	# Iterate the recordset and create lists of table names and ranks
	my @tables;
	my @ranks;
	my @expr_positions;
	my @table_ids;
	while ( !$ADO_RS->EOF ) {
		push @tables, $ADO_RS->Fields(0)->value;
		push @ranks, $ADO_RS->Fields(1)->value;
		push @expr_positions, $ADO_RS->Fields(2)->value;
		push @table_ids, $ADO_RS->Fields(3)->value;
		$ADO_RS->MoveNext;
	}
	#logmsg info, "tables", join("\n", @tables);

	# Iterate the tables
	for my $j (0..$#tables) {
		my $table = @tables[$j];
		my $rank = @ranks[$j];
		my $expr_position = @expr_positions[$j];
		my $table_id = @table_ids[$j];
		
		if (!open CSV, ">${dir}/${table}.csv") {
			logmsg error, "error opening >${dir}/${table}.csv";
		}

		# For each classification dimension of the table, determine if it has a margin
		my $sql = "
		  Select Totals
		  From TableClassDimDic
		  Where LanguageID = 0 And TableID = ${table_id}
		  ;
		";
		$ADO_RS = $ADO_Conn->Execute($sql);
		if (Win32::OLE->LastError()) {
			logmsg error, "OLE", Win32::OLE->LastError();
			return 1;
		}
		my @has_margin;
		while ( !$ADO_RS->EOF ) {
			push @has_margin, $ADO_RS->Fields(0)->value;
			$ADO_RS->MoveNext;
		}
		#logmsg info, ${table}, "has_margin", join(",", @has_margin);

		# construct permuted dimension list which puts analysis dimension last
		# construct max index list at same time
		my $dim_list;
		my $max_dim_list;
		for (my $dim = 0; $dim < $rank; ++$dim) {
			if ($dim > 0) {
				$dim_list .= ", ";
				$max_dim_list .= ", ";
			}
			
			my $permuted_dim;
			if ($dim < $expr_position) {
				# dimensions before the analysis dimension are unchanged for permuted order
				$permuted_dim = $dim;
			}
			elsif ($dim == $rank - 1) {
				# analysis dimension is last dimension for permuted order
				$permuted_dim = $expr_position;
			}
			elsif ($dim >= $expr_position) {
				# skip over the analysis dimension for permuted order
				$permuted_dim = $dim + 1;
			}

			$dim_list .= "Dim${permuted_dim}";
			$max_dim_list .= "Max(Dim${permuted_dim})";
		}
		#logmsg info, ${table}, "dim_list", $dim_list;
		#logmsg info, ${table}, "max_dim_list", $max_dim_list;

		# Determine maximum value of each dimension (to identify margin index)
		my @max_dims;
		if ($rank > 0) {
			my $sql = "Select ${max_dim_list} From ${table};";
			$ADO_RS = $ADO_Conn->Execute($sql);
			if (Win32::OLE->LastError()) {
				logmsg error, "OLE", Win32::OLE->LastError();
				return 1;
			}
			my $fields = $ADO_RS->Fields->count;
			for (my $field_ordinal = 0; $field_ordinal < $fields; $field_ordinal++) {
				my $value = $ADO_RS->Fields($field_ordinal)->value;
				push @max_dims, $value;
			}
		}
		#logmsg info, ${table}, "max_dims", join(",", @max_dims);
		
		my $sql = "Select ";
		if ($rank > 0) {
			$sql .= "${dim_list}, ";
		}
		$sql .= "Value From ${table}";
		if ($rank > 0) {
			$sql .= " Order By ${dim_list}";
		}
		$sql .= ";";
		#logmsg info, "sql", $sql;
		
		$ADO_RS = $ADO_Conn->Execute($sql);
		my $fields = $ADO_RS->Fields->count;
		# First output line contains field names
		# Note that these are not the permuted names.
		# This is to generate csv's which look like those from ompp,
		# where the analysis dimension is always last.
		for (my $dim = 0; $dim < $rank; ++$dim) {
			print CSV "Dim${dim},";
		}
		print CSV "Value\n";

		# data lines
		while ( !$ADO_RS->EOF ) {
			my $out_line;
			my $suppress_line = 0;
			for (my $field_ordinal = 0; $field_ordinal < $fields; $field_ordinal++) {
				my $value = $ADO_RS->Fields($field_ordinal)->value;
				if (length($value) && $round_value && $field_ordinal == $fields - 1) {
					$value = 0 + sprintf("%.${round_prec}e", $value);
				}
				$suppress_line = 1 if $suppress_margins && $has_margin[$field_ordinal] && $value == $max_dims[$field_ordinal];
				$out_line .= "${value}";
				if ($field_ordinal < $fields - 1) {
					$out_line .= ",";
				}
			}
			print CSV "${out_line}\n" if ! $suppress_line;
			$ADO_RS->MoveNext;
		}
		close CSV;
	}

	# Success
	return 0;
}



# Create digest for each file in list
# args - list of file names
# returns - multi-line string, as "file_name, digest"
sub create_digest
{
	my $result;
	use Digest::MD5;
	for my $file (@_) {
		return 1 if ! -f $file;
		my $ctx = Digest::MD5->new;
		open(FILE, $file);
		binmode(FILE);
		$ctx->addfile(*FILE);
		my $check_sum = $ctx->hexdigest;
		$result .= "${file}, ${check_sum}\n";
		close(FILE, $file);
	}
	return $result;
}

# Report differences between two digest files
# arg0 - digest file #1
# arg0 - digest file #2
# returns - comma-separated string of file names whose digests differ
sub digest_differences {
	my $in_digest1 = shift(@_);
	my $in_digest2 = shift(@_);
	my $result = '';
	
	# read the digest1 information into a map
	my %digest1;
	if (!open IN_DIGEST1, "<${in_digest1}") {
		logmsg error, "unable to open ${in_digest1}";
		return 1;
	};
	while (<IN_DIGEST1>) {
		chomp;
		(my $key, my $value) = split ', ';
		$digest1{$key} = $value;
	}
	close IN_DIGEST1;
	
	# read the digest2 information into a map
	my %digest2;
	if (!open IN_DIGEST2, "<${in_digest2}") {
		logmsg error, "unable to open ${in_digest2}";
		return 1;
	};
	while (<IN_DIGEST2>) {
		chomp;
		(my $key, my $value) = split ', ';
		$digest2{$key} = $value;
	}
	close IN_DIGEST2;

	# Create amalgamated map of union of files in digest1 and digest2
	my %all_files = (%digest1, %digest2);
	
	foreach my $file (sort(keys %all_files)) {
		my $value1 = $digest1{$file};
		my $value2 = $digest2{$file};
		if ( $value1 ne $value2) {
			$result .= ', ' if length($result);
			$result .= $file;
		}
	}
	
	return $result;
}

