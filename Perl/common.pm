# Copyright (c) 2013-2021 OpenM++ Contributors
# This code is licensed under the MIT license (see LICENSE.txt for details)

# Common shared components for ompp Perl utilities, e.g. test_models.pl and friends 

use strict;

# Strings used as line prefixes in output
sub warning {
	return "  warning => ";
}
sub change {
	return "DIFFERS ===> ";
}
sub error {
	return "ERROR =====> ";
}
sub info {
	return "             ";
}
sub diagnostic {
	return "diagnostic   ";
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
        my $part = shift(@_);
        # do not add to prefix if empty
        next if $part eq '';
        # append ': ' to end of string
        # if any trailing spaces, insert before them
        $part =~ s/( *)$/:$1 /;
        $prefix .= $part;
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
if (defined(which($sqlite3_exe))) {
	# make absolute path to sqlite3
    # intermediate variable $thing required to avoid compile error on Linux
    my $thing = which($sqlite3_exe); 
	$sqlite3_exe = abs_path($thing);
}
if ( ! -x $sqlite3_exe ) {
	# Too bad, so sad...
	logmsg error, "Cannot find sqlite3.";
	exit 1;
}
sub sqlite3_exe {
	return $sqlite3_exe;
}

# Run SQL in a file on a SQLite database
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

# Extract all model output tables from a ompp SQLite database to a folder
# arg0 - the SQLite database
# arg1 - the destination folder
# arg2 - the number of significant digits to output (optional)
# arg3 - flag to create original    tables in subfolder original (optional)
# arg4 - flag to create transformed tables in subfolder transformed (optional)
# returns - 0 for success, otherwise non-zero
#
# Note: Implementation uses DB compatibility views
sub ompp_tables_to_csv
{
	my $db = shift(@_);
	my $dir = shift(@_);
	my $retval;
	my $rounding_on = 0;
	my $do_original = 0;
	my $do_transformed = 0;
	my $round_prec = 0;
	if ($#_ >= 0) {
		$round_prec = shift(@_);
		if ($round_prec > 0) {
			$rounding_on = 1;
		}
	}
	if ($#_ >= 0) {
		$do_original = shift(@_);
	}
	if ($#_ >= 0) {
		$do_transformed = shift(@_);
	}
	
	# Change slashes from \ to / for sqlite3
	$dir =~ s@[\\]@/@g;

    my $dir_original = "${dir}/original";
    my $dir_transformed = "${dir}/transformed";

    my @outdirs;
    push @outdirs, $dir;
    push @outdirs, $dir_original    if $do_original;
    push @outdirs, $dir_transformed if $do_transformed;

    # create output directories
    for my $fldr (@outdirs) {
        if (! -d $fldr && ! mkdir $fldr) {
            if (!mkdir $fldr) {
                logmsg error, "unable to create directory ${fldr}";
                return 1;
            }
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
		if (-z $in_csv) {
			# remove and skip zero-size files caused by table suppression
			unlink $in_csv;
			next;
		}

		if (!open IN_CSV, "<${in_csv}") {
			logmsg error, "unable to open ${in_csv}";
			return 1;
		};
		my $out_csv = "${dir}/${table}.csv";
		if (!open OUT_CSV, ">${out_csv}") {
			logmsg error, "unable to open ${out_csv}";
			return 1;
		};
		if ($do_original) {
			my $out_csv_original = "${dir_original}/${table}.csv";
			if (!open OUT_CSV_ORIGINAL, ">${out_csv_original}") {
				logmsg error, "unable to open ${out_csv_original}";
				return 1;
			};
		}
		if ($do_transformed) {
			my $out_csv_transformed = "${dir_transformed}/${table}.csv";
			if (!open OUT_CSV_TRANSFORMED, ">${out_csv_transformed}") {
				logmsg error, "unable to open ${out_csv_transformed}";
				return 1;
			};
		}
		my $header_line = 1;
		while (<IN_CSV>) {
			s/\s*$//; # workaround to trailing \r\n in sqlite3 3.22.0 on Ubuntu
			#chomp;
			if ($header_line) {
				print OUT_CSV $_."\n";
				print OUT_CSV_ORIGINAL $_."\n" if $do_original;
				print OUT_CSV_TRANSFORMED $_."\n" if $do_transformed;
				$header_line = 0;
				next;
			}
			my $line = $_;
			if (length($line) && substr($line, -1, 1) ne ',') {
				# value field is not empty, process it
				my @fields = split /,/, $line;
				# get the last (value) field
				my $value = pop @fields;
				my $original_value = 0.0 + $value;
				my $transformed_value = $value;
				if ($rounding_on) {
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
                $transformed_value = apply_transform($value, $round_prec) if $do_transformed;
				# Windows Perl does 7.836e-007 and Linux Perl 7.836e-07, so make uniform
				$value =~ s/e([-+])0(\d\d)/e\1\2/;
				$original_value =~ s/e([-+])0(\d\d)/e\1\2/ if $do_original;
				$transformed_value =~ s/e([-+])0(\d\d)/e\1\2/ if $do_transformed;
				print OUT_CSV join(',', @fields).','.$value."\n";
				print OUT_CSV_ORIGINAL join(',', @fields).','.$original_value."\n" if $do_original;
				print OUT_CSV_TRANSFORMED join(',', @fields).','.$transformed_value."\n" if $do_transformed;
			}
			else {
				print OUT_CSV $line."\n";
				print OUT_CSV_ORIGINAL $line."\n" if $do_original;
				print OUT_CSV_TRANSFORMED $line."\n" if $do_transformed;
			}
		}
		close IN_CSV;
		close OUT_CSV;
		close OUT_CSV_ORIGINAL if $do_original;
		close OUT_CSV_TRANSFORMED if $do_transformed;
		unlink $in_csv;
	}
	
	# Remove temporary file
	unlink $temp_sql;

	# Success
	return 0;
}

# Transform numeric value for alternative digest
# arg0 - the original value
# arg1 - the digits of precision for rounding
# returns - the transformed value
sub apply_transform
{
	my $value = shift(@_);
	my $round_prec  = shift(@_);
    
    my $transformed_value = $value * 1.12345678987654321;
    if ($round_prec > 0) {
        # 2-stage rounding
        $transformed_value = sprintf("%.15g", $transformed_value);
        $transformed_value = sprintf("%.${round_prec}g", $transformed_value);
        $transformed_value = 0.0 + $transformed_value;
    }
    return $transformed_value;
}


# Get the property from an xml property file (e.g. .targets file)
# arg0 - the path of the property file Model.props
# arg1 - the user macro (property) name
# returns - the name, or empty string if not found
sub get_property
{
	my $xml_file = shift(@_);
	my $property = shift(@_);
	my $value = '';
	
	if (!open XML_FILE, "<${xml_file}") {
		return '';
	}
	while (<XML_FILE>) {
		chomp;
		my $line = $_;
		if ( $line =~ /<${property}>(.*)<\/${property}>/ ) {
			$value = $1;
			last;
		}
	}
	close XML_FILE;
	return $value;
}

# Create a Modgen .scex file
# arg0 - the output file name
# arg1 - the ompp Base(Framework).odat file
# arg2 - the ompp ompp_framework.ompp file
# arg3 - optional run ini file
# arg4 - working directory for relative paths in .dat arguments (with trailing slash)
# arg5 - flag to copy parameters to output DB
# argN - remaining arguments are .dat files
# returns - 0 for success, otherwise non-zero
sub modgen_create_scex
{
	my $scex_file      = shift(@_);
	my $framework_odat_file = shift(@_);
	my $framework_ompp_file = shift(@_);
	my $run_ini = shift(@_);
	my $working_dir = shift(@_);
	my $copy_params = shift(@_);

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
		StartingSeed => 1,
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
    my $members = $General{'Subsamples'};
    my $threads = $General{'Threads'};
	
    $General{"CopyParameters"} = $copy_params;

	# Parse Base ompp framework code file for .scex scenario information
	if (!open FRAMEWORK_OMPP, "<${framework_ompp_file}") {
		logmsg error, 'modgen_create_scex', "unable to open ${framework_ompp_file}";
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
		logmsg error, 'modgen_create_scex', "unable to open ${framework_odat_file}";
		return 1;
	}
	while (<FRAMEWORK_ODAT>) {
		chomp;
		my $line = $_;
		if ( $line =~ /^\s*long\s+long\s+SimulationSeed\s*=\s*(\d+)/ ) {
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

    # Propagate selected values from model ini to scex, if present
    # Values from the Parameter section override values from framework.odat
    #logmsg diagnostic, 'modgen_create_scex', "run_ini=${run_ini}";
    if ('' ne $run_ini && -f $run_ini) {
        #logmsg diagnostic, 'modgen_create_scex', "processing ini file";
        if (!open MODEL_INI, "<${run_ini}") {
            logmsg error, 'modgen_create_scex', "unable to open ${run_ini}";
            return 1;
        }
        my $section = '';
        while (<MODEL_INI>) {
            my $key = '';
            my $value = '';
            chomp;
            my $line = $_;
            if ( $line =~ /\[(\w+)\]/ ) {
                # section detected
                $section = $1;
                #logmsg diagnostic, 'modgen_create_scex', "section=${section}";
                next;
            }
            if ( $line =~ /^\s*(\w+)\s*=\s*(\w+)/ ) {
                $key = $1;
                $value = $2;
                #logmsg diagnostic, 'modgen_create_scex', "key=${key} value=${value}";
            }
            else {
                # Ignore any line which is not a simple key = value pair
                # since this is all we are looking for in this context.
                next;
            }
            if ($section eq 'OpenM' && $key eq 'SubValues') {
                $General{"Subsamples"} = $value;
            }
            if ($section eq 'OpenM' && $key eq 'Threads') {
                $General{"Threads"} = $value;
            }
            if ($section eq 'Parameter' && $key eq 'SimulationSeed') {
                $General{"StartingSeed"} = $value;
            }
            if ($section eq 'Parameter' && $key eq 'SimulationCases') {
                $General{"Cases"} = $value;
            }
            if ($section eq 'Parameter' && $key eq 'SimulationEnd') {
                $General{"SimulationEnd"} = $value;
            }
        }
        close MODEL_INI;
    }
    
	# The XML structure of the .scex file is simple,
	# so the approach here is to generate it directly rather than use XML::Tiny or XML::LibXML, etc.

	if (!open SCEX, ">${scex_file}") {
		logmsg error, 'modgen_create_scex', "unable to open ${scex_file}";
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
		print SCEX "    <Input>${file}=${working_dir}${file}</Input>\n";
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
# arg2 - alternate digest file #1
# arg3 - alternate digest file #2
# arg4 - folder of files #1
# arg5 - folder of files #2
# arg6 - significant digits for sameness
# returns - 4-element list with not in 1, not in 2, differs, same.
#    Each element is a comma-separated string of file names
sub digest_differences {
	my $in_digest1 = shift(@_);
	my $in_digest2 = shift(@_);
	my $in_digest1a = shift(@_);
	my $in_digest2a = shift(@_);
	my $in_folder1 = shift(@_);
	my $in_folder2 = shift(@_);
    my $significant_digits = shift(@_);
    
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
    
    # populate alternate digest1 if supplied and valid
    my %digest1a;
    if (-e $in_digest1a) {
        if (!open IN_DIGEST1A, "<${in_digest1a}") {
            logmsg error, "unable to open ${in_digest1a}";
            return 1;
        };
        while (<IN_DIGEST1A>) {
            # trim all trailing white space
            $_ =~ s/\s*$//;
            (my $key, my $value) = split ', ';
            $digest1a{$key} = $value;
        }
        close IN_DIGEST1A;
    }

    # populate alternate digest2 if supplied and valid
    my %digest2a;
    if (-e $in_digest2a) {
        if (!open IN_DIGEST2A, "<${in_digest2a}") {
            logmsg error, "unable to open ${in_digest2a}";
            return 1;
        };
        while (<IN_DIGEST2A>) {
            # trim all trailing white space
            $_ =~ s/\s*$//;
            (my $key, my $value) = split ', ';
            $digest2a{$key} = $value;
        }
        close IN_DIGEST2A;
    }

	# Create amalgamated map of union of files in digest1 and digest2
	my %all_files = (%digest1, %digest2);
	
	
	my $not_in_1 = '';
	my $not_in_2 = '';
	my $differs = '';
    my $same = '';
	foreach my $file (sort(keys %all_files)) {
        my $is_csv = ($file =~ /[.]csv$/i);
		if (!exists $digest1{$file}) {
			$not_in_1 .= ',' if length($not_in_1);
			$not_in_1 .= $file;
		}
		elsif (!exists $digest2{$file}) {
			$not_in_2 .= ',' if length($not_in_2);
			$not_in_2 .= $file;
		}
		else {
			my $value1 = $digest1{$file};
			my $value2 = $digest2{$file};
			if ( $value1 eq $value2) {
                #logmsg diagnostic, 'digest_differences', "   same by method 1: ${file}";
				$same .= ',' if length($same);
				$same .= $file;
			}
            elsif ($is_csv) {
                #logmsg diagnostic, 'digest_differences', "differs by method 1: ${file}";
                # differ with main digests
                # try alternate digests (method 2)
                my $value1a = $digest1a{$file};
                my $value2a = $digest2a{$file};
                if (defined($value1a) && defined($value1a) && $value1a eq $value2a) {
                    # alternate digests equal, so consider equal
                    #logmsg diagnostic, 'digest_differences', "   same by method 2: ${file}";
                    $same .= ',' if length($same);
                    $same .= $file;
                }
                else {
                    #logmsg diagnostic, 'digest_differences', "differs by method 2: ${file}";
                    my $file1 = "${in_folder1}/${file}";
                    my $file2 = "${in_folder2}/${file}";
                    if (-e $file1 && -e $file2 && same_values_csv($file1, $file2, $significant_digits)) {
                        #logmsg diagnostic, 'digest_differences', "   same by method 3: ${file}";
                        # passed cell-by-cell comparison
                        $same .= ',' if length($same);
                        $same .= $file;
                    }
                    else {
                        #logmsg diagnostic, 'digest_differences', "differs by method 3: ${file}";
                        $differs .= ',' if length($differs);
                        $differs .= $file;
                    }
                }
            }
            else { # elsif ($is_csv)
                # not a .csv, method 1 detects difference without ambiguity
                $differs .= ',' if length($differs);
                $differs .= $file;
            }
		}
	}
	
	return ($not_in_1, $not_in_2, $differs, $same);
}

# Normalise event trace for comparability
# arg0 - the input event trace file
# arg1 - the output normalized event trace file
# arg2 - the format for time, eg 16.9f (no leading %)
# arg3 - offset to be added to the entity id
# returns - 0 for success, otherwise non-zero
sub normalize_event_trace
{
	my $input_event_trace = shift(@_);
	my $output_event_trace = shift(@_);
    my $time_format = '%'.shift(@_);
    my $entity_id_offset = shift(@_);
	
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
		my $entity_id = $2 + $entity_id_offset;
		my $case_seed = $3;
		my $function = $4;
		my $time = $5;
		my $key = sprintf("%s %10d", $function, $entity_id);
		
		# Replace very large times (or inf) with a fixed smaller value, to avoid
		# reporting spurious precision-related differences due to magnitude of large times
		if ($time eq "inf" || $time > 99999.0) {
			$time = 99999.0;
		}
		# Reformat time at specified precision
		$time = sprintf($time_format, $time);
		
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

# Find warning and error messages in a text file
# arg0 - name of input file
# returns - 4-element list with (error_count, error_messages, warning_count, warning_messages), where messages are new-line separated strings.
sub extract_errors_and_warnings {
	my $in_file = shift(@_);
    
    my $error_count = 0;
    my $error_messages = '';

    my $warning_count = 0;
    my $warning_messages = '';

	if (!open IN, "<${in_file}") {
		logmsg error, "error opening >${in_file}";
		return (1,'',0,'');
	}
    
	while (<IN>) {
		chomp;
		my $line = $_;
        
        # re tries to detect error/warning formats used in build chains used by ompp
        if ($line =~ /(^|\s)warning ?\w* ?:/i ) {
            $warning_messages .= "\n" if $warning_count > 0;
            $warning_messages .= $line;
            $warning_count++;
        }
        if ($line =~ /(^|\s)error ?\w* ?:/i ) {
            $error_messages .= "\n" if $error_count > 0;
            $error_messages .= $line;
            $error_count++;
        }
        
        # special stop processing conditions to avoid double-counting warnings and errors in msbuild output.
        last if $line =~ /^Done Building Project/;
        
    };
	close IN;
    
	return ($error_count, $error_messages, $warning_count, $warning_messages);
}

# Report if the values in two csv files are the same, within specified tolerance
# arg0 - csv file #1
# arg1 - csv file #2
# arg2 - digits of precision for sameness
# returns - true if all cells compare the same, otherwise false
sub same_values_csv {
	my $in_csv1 = shift(@_);
	my $in_csv2 = shift(@_);
	my $prec = shift(@_);
	my $result = 0;
    my $tol = 10 ** (-$prec); # comparison tolerance
	
	if (!open IN_CSV1, "<${in_csv1}") {
		logmsg error, "unable to open ${in_csv1}";
		return 0;
	};
    
	if (!open IN_CSV2, "<${in_csv2}") {
		logmsg error, "unable to open ${in_csv1}";
		return 0;
	};
    
    while (1) {
        my $line1 = <IN_CSV1>;
        my $line2 = <IN_CSV2>;
        
        if ($line1 ne $line2) {
            # Split into leading indices and value parts.
            # All tables have a measure dimension, so at least one comma is always present,
            # even for tables with no classification dimensions.
            # RE below excludes trailing newlines from capture group #2
            chomp $line1;
            $line1 =~ /^(.*),([^,]*)$/;
            my $indices1 = $1;
            my $value1 = $2;
            chomp $line2;
            $line2 =~ /^(.*),([^,]*)$/;
            my $indices2 = $1; 
            my $value2 = $2;
            if ($indices1 ne $indices2) {
                #logmsg diagnostic, 'same_values_csv', "indices differ indices1=${indices1} indices2=${indices2}";
                $result = 0;
                last;
            }
            elsif (!length($value1) || !length($value2)) { # perl idiom for detecting 0-length string
                # value1 and value2 differ
                # One value is missing (parsed as empty string)
                # Perl interprets '' in numeric context to be 0, so we deal with it as a special case
                # to avoid treating missing and 0 as the same (they are not!)
                #logmsg diagnostic, 'same_values_csv', "cell value differs indices=${indices1} one value null, other not";
                $result = 0;
                last;
            }
            else {
                # value1 and value2 differ
                my $diff = abs($value2 - $value1);
                if ($value1 != 0) {
                    # convert to proportional difference if possible
                    $diff = $diff / abs($value1);
                }
                ##logmsg diagnostic, 'same_values_csv', "indices=${indices1} value1=${value1} value2=${value2} diff=${diff} tol=${tol}";
                if ($diff > $tol) {
                    #logmsg diagnostic, 'same_values_csv', "cell value differs indices=${indices1} value1=${value1} value2=${value2} diff=${diff}";
                    $result = 0;
                    last;
                }
            }
        }
        
        if (eof(IN_CSV1) || eof(IN_CSV2)) {
            if(eof(IN_CSV1) && eof(IN_CSV1)) {
                # simultaneous eof with no differences found
                $result = 1;
            }
            else {
                # the two files have differing line counts
                #logmsg diagnostic, 'same_values_csv', "input files have differing line counts";
                $result = 0;
            }
            last;
        }
    }
    
	close IN_CSV1;
	close IN_CSV2;
    return $result;
}


return 1;