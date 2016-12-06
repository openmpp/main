# Copyright (c) 2013-2014 OpenM++
# This code is licensed under MIT license (see LICENSE.txt for details)

# Common shared components for ompp Perl utilities, e.g. test_models.pl and friends 

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
#
# Windows with ActivePerl:
# (1) open command prompt
# (2) find and run vsvars32.bat (to get path to nmake into environment)
#     Normally some place like
#     "C:\Program Files (x86)\Microsoft Visual Studio 10.0\Common7\Tools\vsvars32.bat"
# (3) perl -MCPAN -e "install Capture::Tiny"
#
# Linux with generic Perl
# yum install 'perl(Capture::Tiny)'

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

# Try several locations to find sqlite3

# Try Linux with sqlite3 in the PATH
use File::Which qw(which);
use File::Basename 'dirname';
use Cwd 'abs_path';
my $sqlite3_exe = 'sqlite3';
if (!defined(which($sqlite3_exe))) {
	# Try Windows with sqlite3 in the PATH
	$sqlite3_exe = 'sqlite3.exe';
}
if (!defined(which($sqlite3_exe))) {
	# Try absolute path using OM_ROOT environment variable.
	$sqlite3_exe = $ENV{'OM_ROOT'}.'/bin/sqlite3.exe';
	if (-e $sqlite3_exe) {
		$sqlite3_exe = abs_path($sqlite3_exe);
	}
}
if (!defined(which($sqlite3_exe))) {
	# Try absolute path to Windows executable in ompp bin relative to this script location.
	$sqlite3_exe = File::Spec->catfile(dirname($0), "../bin/sqlite3.exe");
	if (-e $sqlite3_exe) {
		$sqlite3_exe = abs_path($sqlite3_exe);
	}
}
if (!defined(which($sqlite3_exe))) {
	# Look in bin subdir of OM_ROOT for Windows executable.
	$sqlite3_exe = File::Spec->catfile(dirname($0), "%OM_ROOT%/bin/sqlite3.exe");
	if (-e $sqlite3_exe) {
		$sqlite3_exe = abs_path($sqlite3_exe);
	}
}
if (!defined(which($sqlite3_exe))) {
	# Try absolute path to Linux executable in ompp bin relative to this script location.
	$sqlite3_exe = File::Spec->catfile(dirname($0), "../bin/sqlite3");
	if (-e $sqlite3_exe) {
		$sqlite3_exe = abs_path($sqlite3_exe);
	}
}
if (!defined(which($sqlite3_exe))) {
	# Too bad, so sad...
	logmsg error, "Cannot find sqlite3.";
	exit 1;
}
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
# arg3 - flag to create non-rounded version of csv (optional)
# returns - 0 for success, otherwise non-zero
sub ompp_tables_to_csv
{
	my $db = shift(@_);
	my $dir = shift(@_);
	my $retval;
	my $do_rounding = 0;
	my $do_unrounded_file = 0;
	my $round_prec = 0;
	if ($#_ >= 0) {
		$round_prec = shift(@_);
		if ($round_prec > 0) {
			$do_rounding = 1;
		}
	}
	if ($#_ >= 0) {
		$do_unrounded_file = shift(@_);
	}
	
	# Change slashes from \ to / for sqlite3
	$dir =~ s@[\\]@/@g;

	if (! -d $dir) {
		if (!mkdir $dir) {
			logmsg error, "unable to create directory ${dir}";
			return 1;
		}
	}

	# Get all output table names and ranks
	my $temp = run_sqlite_statement $db, "Select table_name, table_rank From table_dic Order By table_name;", $retval;
	if ($retval != 0) {
		return $retval;
	}

	# create array of table names and hash table_name ==> rank
	my %ranks;
	my @tables;
	for my $record (split(/\n/, $temp)) {
		(my $col1, my $col2) = split(/\|/, $record);
		push @tables, $col1;
		$ranks{$col1} = $col2;
	}
	
	# Create SQLite script in temporary file to extract all result tables
	my $temp_sql = "${dir}/ompp_tables_to_csv.sql";
	if (!open TEMP_SQL, ">${temp_sql}") {
		logmsg error, "unable to create directory ${dir}";
		return 1;
	};
	print TEMP_SQL ".mode csv\n";
	print TEMP_SQL ".headers on\n";
	for my $table (@tables) {
		my $rank = $ranks{$table};
		my $order_clause = "Order By ";
		for my $dim (0..$rank) {
			$order_clause .= 'Dim'.$dim;
			$order_clause .= ',' if $dim < $rank;
		}
		#print "table=${table} order=${order}\n";
		print TEMP_SQL ".output ${dir}/in_${table}.csv\n";
		print TEMP_SQL "Select * From ${table} ${order_clause};\n";
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
	for my $table (@tables) {
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
		if ($do_unrounded_file) {
			my $out_csv_unrounded = "${dir}/_${table}.csv";
			if (!open OUT_CSV_UNROUNDED, ">${out_csv_unrounded}") {
				logmsg error, "unable to open ${out_csv_unrounded}";
				return 1;
			};
		}
		my $header_line = 1;
		while (<IN_CSV>) {
			if ($header_line) {
				print OUT_CSV;
				print OUT_CSV_UNROUNDED if $do_unrounded_file;
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
				my $unrounded_value = 0.0 + $value;
				if ($do_rounding) {
					$value = $value + 0.0;
					# standard rounding
					# $value = sprintf("%.${round_prec}g", $value);
					
					# 2-stage rounding
					$value = sprintf("%.15g", $value);
					$value = sprintf("%.${round_prec}g", $value);
				
					# hierarchical rounding
					#for (my $j = 15; $j >= $round_prec; $j--) {
					#	$value = sprintf("%.*g", $j, $value);
					#}
					$value = 0.0 + $value;
				}
				# Windows Perl does 7.836e-007 and Linux Perl 7.836e-07, so make uniform
				$value =~ s/e([-+])0(\d\d)/e\1\2/;
				$unrounded_value =~ s/e([-+])0(\d\d)/e\1\2/ if $do_rounding;
				print OUT_CSV join(',', @fields).','.$value."\n";
				print OUT_CSV_UNROUNDED join(',', @fields).','.$unrounded_value."\n" if $do_unrounded_file;
			}
			else {
				print OUT_CSV $line."\n";
				print OUT_CSV_UNROUNDED $line."\n" if $do_unrounded_file;
			}
		}
		close IN_CSV;
		close OUT_CSV;
		close OUT_CSV_UNROUNDED if $do_unrounded_file;
		unlink $in_csv;
	}
	
	# Remove temporary file
	unlink $temp_sql;

	# Success
	return 0;
}


# Get the user macro from a property file
# arg0 - the path of the property file Model.props
# arg1 - the user macro (property) name
# returns - the name, or empty string if failure
sub get_user_macro
{
	my $model_props = shift(@_);
	my $user_macro = shift(@_);
	my $value = '';
	
	if (!open MODEL_PROPS, "<${model_props}") {
		return '';
	}
	while (<MODEL_PROPS>) {
		chomp;
		my $line = $_;
		if ( $line =~ /<${user_macro}>(.*)<\/${user_macro}>/ ) {
			$value = $1;
			last;
		}
	}
	close MODEL_PROPS;
	return $value;
}

# Create a Modgen .scex file
# arg0 - the output file name
# arg1 - the ompp Base(Framework).odat file
# arg2 - the ompp ompp_framework.ompp file
# arg3 - the Modgen property file Model.props
# argN - remaining arguments are .dat files
# returns - 0 for success, otherwise non-zero
sub modgen_create_scex
{
	my $scex_file      = shift(@_);
	my $framework_odat_file = shift(@_);
	my $framework_ompp_file = shift(@_);
	my $model_props = shift(@_);

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
	
	# Override values based on Model.props
	my $members = get_user_macro($model_props, 'MEMBERS');
	$General{"Subsamples"} = $members;
	my $threads = get_user_macro($model_props, 'THREADS');
	$General{"Threads"} = $threads;

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
		if ( $line =~ /^\s*int\s+SimulationSeed\s*=\s*(\d+)/ ) {
			$General{"StartingSeed"} = $1;
		}
		if ( $line =~ /^\s*long\s+long\s+SimulationCases\s*=\s*(\d+)/ ) {
			$General{"Cases"} = $1;
		}
		if ( $line =~ /^\s*double\s+SimulatedPopulation\s*=\s*(\d+[.]?\d+)/ ) {
			$General{"Population"} = $1;
		}
		if ( $line =~ /^\s*Time\s+SimulationEnd\s*=\s*(\d+[.]?\d+)/ ) {
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



# Create digest for each file in list
# args - list of file names (text files)
# returns - multi-line string, as "file_name, digest"
sub create_digest
{
	my $result;
	use Digest::MD5;
	for my $file (@_) {
		return 1 if ! -f $file;
		my $ctx = Digest::MD5->new;
		open(FILE, $file);
		# binmode is deliberately not used for cross-platform comparability.
		# Argument list must be text files.
		#binmode(FILE);
		$ctx->addfile(*FILE);
		my $check_sum = $ctx->hexdigest;
		$result .= "${file}, ${check_sum}\n";
		close(FILE, $file);
	}
	return $result;
}

# Report differences between two digest files
# arg0 - digest file #1
# arg1 - digest file #2
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
		# trim all trailing white space
		$_ =~ s/\s*$//;
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
		# trim all trailing white space
		$_ =~ s/\s*$//;
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

# Normalise event trace for comparability
# arg0 - the input event trace file
# arg1 - the output normalized event trace file
# returns - 0 for success, otherwise non-zero
sub normalize_event_trace
{
	my $input_event_trace = shift(@_);
	my $output_event_trace = shift(@_);
	
	if (!open IN, "<${input_event_trace}") {
		logmsg error, "error opening >${input_event_trace}";
		return 1;
	}

	if (!open OUT, ">${output_event_trace}") {
		logmsg error, "error opening >${output_event_trace}";
		return 1;
	}

	my %times;
	my %functions;
	my %entity_ids;
	my %entity_kinds;
	while (<IN>) {
		chomp;
		my $line = $_;
		my $is_checksum_line = ($line =~ /^Case seed : ([\d.]+)\t-\tCase sample: (\d+)\t-\tCheckSum : ([\d.]+)$/);
		if ($is_checksum_line) {
			my $case_seed = $1;
			my $case_sample = $2;
			my $checksum = $3;
			# retain 11 digits of precision in the checksum
			my $checksum_formatted = sprintf("%.10e", $checksum);
			# Windows Perl does 7.836e-007 and Linux Perl 7.836e-07, so make uniform
			$checksum_formatted =~ s/e([-+])0(\d\d)/e\1\2/;
			printf OUT "Seed: %15d Sample: %2d Checksum: %s\n", $case_seed, $case_sample, $checksum_formatted;
			next;
		}
		my $is_trace_line = ($line =~ /^(\w+) - actor_id=(\d+) - case_seed=(\d+) -  : event=(.*) - time=(.*)$/);
		if (!$is_trace_line) {
			# pass through anything unrecognised
			printf OUT "${line}\n";
			next;
		}
		# is trace line
		my $entity_kind = $1;
		my $entity_id = $2;
		my $case_seed = $3;
		my $function = $4;
		my $time = $5;
		my $key = sprintf("%s %10d", $function, $entity_id);
		
		# Replace very large times (or inf) with a fixed smaller value, to avoid
		# reporting spurious precision-related differences due to magnitude of large times
		if ($time eq "inf" || $time > 99999.0) {
			$time = 99999.0;
		}
		# Reformat time at lower precision
		$time = sprintf("%13.6f", $time);
		
		my $is_event = 0;
		if ($function =~ /(\w+)[.](\w+)/) {
			$is_event = 1;
			$function = $2;
		}
		elsif ($function =~ /scheduled - (\d+)/) {
			$is_event = 1;
			$function = "self-scheduling event - attribute $1";
		}
		
		if ($is_event) {
			# Dump future events sorted by key (function + entity_id)
			foreach $key (sort keys(%times)) {
				my $time = $times{$key};
				my $function = $functions{$key};
				my $entity_id = $entity_ids{$key};
				my $entity_kind = $entity_kinds{$key};
				print OUT "                 ${time} ${function} (${entity_kind} ${entity_id})\n";
			}
			%times = ();
			%functions = ();
			%entity_ids = ();
			%entity_kinds = ();
		}
		else {
			# Record information for future event in maps
			$times{$key} = $time;
			$functions{$key} = $function;
			$entity_ids{$key} = $entity_id;
			$entity_kinds{$key} = $entity_kind;
		}
		
		if ($is_event) {
			print OUT "${time} ${function} (${entity_kind} ${entity_id})\n";
		}
		
	}
	
	close IN;
	close OUT;
	
	return 0;
}

return 1;