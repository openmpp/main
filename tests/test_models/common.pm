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
my $sqlite3_exe = "../../bin/sqlite3.exe";
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
# returns - 0 for success, otherwise non-zero
sub ompp_tables_to_csv
{
	my $db = shift(@_);
	my $dir = shift(@_);
	my $retval;

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
		while (<IN_CSV>) {
			# trim redundant .0 at end of line if present
			$_ =~ s/.0$//;
			print OUT_CSV $_;
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
# arg1 - StartingSeed
# arg2 - Subsamples (aka replicates for time-based models)
# arg3 - Cases
# arg4 - SimulationEnd
# argN - remaining arguments are .dat files
# returns - 0 for success, otherwise non-zero
sub modgen_create_scex
{
	my $scex_file     = shift(@_);

	my $StartingSeed  = shift(@_);
	my $Subsamples    = shift(@_);
	my $Cases         = shift(@_);
	my $SimulationEnd = shift(@_);

	# Defaults
	my %General_attributes = (
		# case-based
		Cases => 5000,
		# time-based
		SimulationEnd => 100,
		# common
		PopulationScaling => 0,
		Population => 5000,
		Threads => 1,
		Subsamples => 1,
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

	# Replace defaults in hash using values supplied in arguments
	$General_attributes{"StartingSeed"} = $StartingSeed;
	$General_attributes{"Subsamples"} = $Subsamples;
	$General_attributes{"Cases"} = $Cases;
	$General_attributes{"SimulationEnd"} = $SimulationEnd;

	# The XML structure of the .scex file is simple,
	# so the approach here is to generate it directly rather than use XML::Tiny or XML::LibXML, etc.

	if (!open SCEX, ">${scex_file}") {
		logmsg error, "unable to open ${scex_file}";
		return 1;
	}
	print SCEX "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	print SCEX "<Scenario ScenarioVersion=\"1.8\">\n";
	print SCEX "  <General\n";
	# Iterate all General attributes
	for my $key(keys %General_attributes){
		print SCEX "    $key=\"".$General_attributes{$key}."\"\n";
	}
	print SCEX "  >\n";
	print SCEX "  </General>\n";
	print SCEX "  <Inputs>\n";
	# Iterate .dat files
	for my $file (@_) {
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
# returns - 0 for success, otherwise non-zero
sub modgen_tables_to_csv
{
	my $db = shift(@_);
	my $dir = shift(@_);
	my $retval;

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
	my $sql = "Select Name, Rank From TableDic Where LanguageID = 0;";
	$ADO_RS = $ADO_Conn->Execute($sql);
	if (Win32::OLE->LastError()) {
		logmsg error, "OLE", Win32::OLE->LastError();
		return 1;
	}
	my $fields = $ADO_RS->Fields->count;
	# Iterate the recordset and create lists of table names and ranks
	my @tables;
	my @ranks;
	while ( !$ADO_RS->EOF ) {
		push @tables, $ADO_RS->Fields(0)->value;
		push @ranks, $ADO_RS->Fields(1)->value;
		$ADO_RS->MoveNext;
	}
	#logmsg info, "tables", join("\n", @tables);

	# Iterate the tables
	for my $j (0..$#tables) {
		my $table = @tables[$j];
		my $rank = @ranks[$j];
		if (!open CSV, ">${dir}/${table}.csv") {
			logmsg error, "error opening >${dir}/${table}.csv";
		}
		my $sql = "Select ";
		for (my $dim = 0; $dim < $rank; ++$dim) {
			$sql .= "Dim${dim}, ";
		}
		$sql .= "Value From ${table};";
		#logmsg info, "sql", $sql;
		$ADO_RS = $ADO_Conn->Execute($sql);
		my $fields = $ADO_RS->Fields->count;
		# First output line contains field names
		for (my $field_ordinal = 0; $field_ordinal < $fields; $field_ordinal++) {
			my $value = $ADO_RS->Fields($field_ordinal)->name;
			print CSV "${value}";
			if ($field_ordinal < $fields - 1) {
				print CSV ",";
			}
		}
		print CSV "\n";

		# data lines
		while ( !$ADO_RS->EOF ) {
			for (my $field_ordinal = 0; $field_ordinal < $fields; $field_ordinal++) {
				my $value = $ADO_RS->Fields($field_ordinal)->value;
				print CSV "${value}";
				if ($field_ordinal < $fields - 1) {
					print CSV ",";
				}
			}
			print CSV "\n";
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
