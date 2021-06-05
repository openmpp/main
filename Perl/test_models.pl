#!/usr/bin/perl
# Copyright (c) 2013-2021 OpenM++ Contributors
# This code is licensed under the MIT license (see LICENSE.txt for details)

# Script to test multiple models in ompp, ompp-linux, ompp-mac, and modgen

use strict;
use warnings;

my $script_name = "test_models";
my $script_version = '2.0';

use Getopt::Long::Descriptive;

my ($opt, $usage) = describe_options(

	$script_name.' %o model...',
    
    # Models
    
    [ 'models_root|m=s'  => 'directory containing models (default is .)',
    { default => '.' } ],

    # Actions

	[ 'newref' => 'replace Reference results with Current results' ],
	[ 'noompp' => 'skip OpenM++ build and run' ],
	[ 'nomodgen' => 'skip Modgen build and run' ],
	[ 'nocomp' => 'skip flavour comparison' ],

    # Verbosity
    
	[ 'allfiles' => 'report all different and orphaned files' ],
	[ 'timing' => 'report elapsed time of steps' ],
	[ 'nosteps' => 'skip reporting which step is being performed' ],
    
    # Build
    
	[ 'config=s' => 'build configuration: debug or release(default)',
		{ default => 'release' } ],
	[ 'mpi_processes=i' => 'build MPI version and run with n processes (default 0, means no MPI)',
		{ default => 0 } ],
	[ 'gencode' => 'keep a copy of the generated C++ code' ],
    
    # Run
    
	[ 'ini=s' => 'OpenM++ model ini file to pass to model (in model root, default is test_models.ini if present)',
		{ default => '' } ],
	[ 'clean' => 'remove all build files after run' ],
    
    # Digests
    
	[ 'significant_digits=i' => 'significant digits (default 7)',
		{ default => 7 } ],
    
    # Miscellaneous
    
	[ 'help|h'    => 'report usage message and exit' ],
	[ 'version|v' => 'report test_models version and exit' ],
    
    # Deprecated
    
	[ 'windows_platform=s' => 'Windows platform: x64(default) or Win32',
		{ default => 'x64' } ],
	[ 'modgen_platform=s' => 'Modgen platform: Win32(default) or x64',
		{ default => 'Win32' } ],
	[ 'nounroundedtables' => 'suppress creation of unrounded versions of tables' ],
);

if ($opt->version) {
	print $script_name.' version '.$script_version."\n";
	exit 0;
}

if ($opt->help) {
	print $usage->text;
	exit 0;
}


#####################
# Common settings
#####################

# Verbosity of report:
#  0 - errors only
#  1 - timing and success
#  2 - chatty
# Just used internally, and not rigourously maintained.  Deprecated pending removal.
my $verbosity = 3;

# report all unique files and all different files
my $report_all_files = 0;
$report_all_files = 1 if $opt->allfiles;

# report timing information
my $report_timing = 0;
$report_timing = 1 if $opt->timing;

# report steps
my $report_steps = 1;
$report_steps = 0 if $opt->nosteps;

# Number of significant digits to retain in the output csv files (0 means full precision)
my $significant_digits = $opt->significant_digits;

# Also create and keep non-rounded version of each output csv table
my $unrounded_tables = 1;
$unrounded_tables = 0 if $opt->nounroundedtables;

# Path to optional ini file (relative to model folder) to pass to ompp model
my $model_ini_opt = $opt->ini;

# Flavour comparison
my $do_flavour_comparison = 1;
$do_flavour_comparison = 0 if $opt->nocomp;

# MPI option
my $mpi_processes = $opt->mpi_processes;
my $use_mpi = 0;
$use_mpi = 1 if $mpi_processes > 0;

# Save generated code option
my $save_generated_code = 0;
$save_generated_code = 1 if $opt->gencode;


#####################
# ompp settings for Windows
#####################

# ompp compiler version for Windows (4 possible values), without leading path
my $omc_exe = 'omc.exe';
#my $omc_exe = 'omcD.exe';
#my $omc_exe = 'omc64.exe';
#my $omc_exe = 'omc64D.exe';

my $config = lc($opt->config);
if ($config ne 'debug' && $config ne 'release') {
	print "Invalid config=$config must be debug or release";
	exit 1;
} 

my $windows_platform = $opt->windows_platform;
if ($windows_platform ne 'Win32' && $windows_platform ne 'x64') {
	print "Invalid windows_platform=$windows_platform must be Win32 or x64";
	exit 1;
} 


#####################
# modgen settings
#####################

my $modgen_platform = $opt->modgen_platform;
if ($modgen_platform ne 'Win32' && $modgen_platform ne 'x64') {
	print "Invalid modgen_platform=$modgen_platform must be Win32 or x64";
	exit 1;
} 

#####################
# There are no configuration settings in subsequent code in this script
#####################

use strict;

use Config::Tiny;
use Cwd qw(getcwd);
use Cwd 'abs_path';
use File::Basename;
use File::Spec;

my $om_root = $ENV{'OM_ROOT'};
my $models_root = $opt->models_root;

# if OM_ROOT not set and --models option specified then set OM_ROOT as parent of models directory
if ( (! $om_root || $om_root eq '') && defined $models_root && $models_root ne '' && -d $models_root ) {
	$om_root = dirname(abs_path($models_root));
}

if ( ! $om_root || $om_root eq '') {
	# Try parent directory, assuming this script was invoked in the OM_ROOT/Perl directory
	my $save_dir = getcwd();
	chdir '..';
	$om_root = getcwd();
	chdir $save_dir;
}
else {
	-d $om_root || die "directory not found OM_ROOT='${om_root}'\n";
}

# Default location assumes script is invoked from OM_ROOT/Perl.
if ( ! $models_root || $models_root eq '' || ! -d $models_root ) {
	$models_root = File::Spec->catdir($om_root, 'models');
}
chdir $models_root || die "Folder ${models_root} not found";
$models_root = getcwd; # to ensure is absolute path

#####################
# file locations
#####################

my $create_db_sqlite_sql = "${om_root}/sql/create_db.sql";

# determine if running on Windows, Linux or MacOS
my $is_windows = 1;
my $is_linux = 0;
my $is_darwin = 0;
if ( $^O eq 'linux') {
	$is_windows = 0;
	$is_linux = 1;
	$is_darwin = 0;
}
if ( $^O eq 'darwin') {
	$is_windows = 0;
	$is_linux = 0;
	$is_darwin = 1;
}

# MSBuild command line reference:
# http://msdn.microsoft.com/en-us/library/ms164311.aspx
use File::Which qw(which);
use Cwd 'abs_path';

my $msbuild_exe = "";
if ($is_windows) {
	if (defined $ENV{MSBUILD_EXE}) {
		# provides a non-standard way to specify the msbuild.exe path
		$msbuild_exe = "$ENV{MSBUILD_EXE}";
	}
	if ( ! -x $msbuild_exe ) {
		if (defined $ENV{VSINSTALLDIR}) {
			# if VS or build tools installed then use current version location
			$msbuild_exe = "$ENV{VSINSTALLDIR}"."MSBuild\\Current\\Bin\\MSBuild.exe";
		}
	}
	if ( ! -x $msbuild_exe ) {
		# search msbuild.exe in the PATH
		my $msb_exe = which("MSBuild.exe");
		if (defined($msb_exe)) {
			$msbuild_exe = abs_path($msb_exe);
		}
	}
	if ( ! -x $msbuild_exe ) {
		# Use VS 2019 aka version 16 if present
		$msbuild_exe = "C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\MSBuild\\Current\\Bin\\MSBuild.exe";
	}
	if ( ! -x $msbuild_exe ) {
		# fall back to VS 2017 aka version 15 if present
		$msbuild_exe = "C:\\Program Files (x86)\\Microsoft Visual Studio\\2017\\Community\\MSBuild\\15.0\\bin\\MSBuild.exe";
	}
	if ( ! -x $msbuild_exe ) {
		die "Missing msbuild_exe: $msbuild_exe";
	}
}

###############
# Get needed components
###############

use File::Copy;
use File::Copy::Recursive qw(dircopy);
use File::Path qw(make_path remove_tree);
use File::Compare;
use File::stat;
use POSIX qw(strftime);

# Add directory of this script to @INC to resolve use and require statements
use File::Basename;
use File::Spec;
use lib File::Basename::dirname(File::Spec->rel2abs($0));

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
				create_digest
				digest_differences
				modgen_create_scex
				get_build_property
				normalize_event_trace
		    );

if ($is_windows) {
	# use the common_windows.pm module of shared functions
	#require common_windows qw(
	#				modgen_version
	#				run_jet_statement
	#				modgen_tables_to_csv
	#			);
	require 'common_windows.pm';
}

###############
# Determine flavours and options
###############

my $new_ref = ($opt->newref) ? 1 : 0;
my $do_clean = ($opt->clean) ? 1 : 0;
my $do_modgen = ($opt->nomodgen) ? 0 : 1;
my $do_ompp = ($opt->noompp) ? 0 : 1;

my @model_dirs;

if ($#ARGV >= 0) {
	# model folders listed explicitly on command line
	@model_dirs = @ARGV;
}
else {
	# no models listed on command line
	# Create model list by identifying all model subdirectories
	my @paths = glob "*/code";
	for my $path (@paths) {
		$path =~ s/\/.*//;
		push @model_dirs, $path;
	}
}

# combine available flavours with requested options, and perform basic checks
my @flavours;
my @flavours_tombstone;
if ($is_windows) {
	if ($do_modgen) {
		my $modgen_folder = 'Modgen 12';
		my $modgen_exe = "C:\\Program Files (x86)\\StatCan\\${modgen_folder}\\Modgen.exe";
		if ( ! -e $modgen_exe ) {
            # silently don't do modgen flavour if Modgen not installed
            $do_modgen = 0;
		}
        else {
            my $modgen_version_string = modgen_version($modgen_exe);
            
            # Check if supported Modgen version
            (my $v1, my $v2, my $v3, my $v4) = split(',', $modgen_version_string);
            if (!($v1 == 12 && $v2 == 1 && $v3==3)) {
                logmsg error, "Modgen version 12.1.3 is required for cross-compatible models";
	            $do_modgen = 0;
            }
			else {
	            my $sb = stat($modgen_exe);
	            my $exe_time_stamp = strftime "%Y-%m-%d %H:%M GMT",gmtime $sb->mtime;
	            push @flavours, 'modgen';
	            push @flavours_tombstone, "version=${modgen_version_string} (${exe_time_stamp}) platform=${modgen_platform} configuration=${config}";
			}
            
        }
	}
	
	if ($do_ompp) {
		if ( ! -e $create_db_sqlite_sql ) {
			logmsg error, "Missing SQL file used to create new data store for ompp: $create_db_sqlite_sql";
			exit 1;
		}
		if ( ! -e $msbuild_exe ) {
			logmsg error, "Missing MSBuild for ompp: $msbuild_exe";
			exit 1;
		}
		push @flavours, 'ompp-win';
		my $full_path = "${om_root}/bin/${omc_exe}";
		-e $full_path or die "Missing ${full_path}"; # shouldn't happen
		my $sb = stat($full_path);
		my $exe_time_stamp = strftime "%Y-%m-%d %H:%M GMT",gmtime $sb->mtime;
        my $mpi_info = '';
        if ($use_mpi) {
            $mpi_info .= ($mpi_processes == 1) ?
                " mpi-enabled"
              : " mpi_processes=${mpi_processes}";
        }
        push @flavours_tombstone, "compiler=${omc_exe} (${exe_time_stamp}) platform=${windows_platform} configuration=${config}${mpi_info}";
	}
}

if ($is_linux && $do_ompp) {
	push @flavours, 'ompp-linux';
	my $omc_exe = 'omc';
	my $full_path = "${om_root}/bin/${omc_exe}";
	-e $full_path or die "Missing ${full_path}"; # shouldn't happen
	my $sb = stat($full_path);
	my $exe_time_stamp = strftime "%Y-%m-%d %H:%M GMT",gmtime $sb->mtime;
	push @flavours_tombstone, "compiler=${omc_exe} (${exe_time_stamp}) configuration=${config}";
}
	
if ($is_darwin && $do_ompp) {
	push @flavours, 'ompp-mac';
	my $omc_exe = 'omc';
	my $full_path = "${om_root}/bin/${omc_exe}";
	-e $full_path or die "Missing ${full_path}"; # shouldn't happen
	my $sb = stat($full_path);
	my $exe_time_stamp = strftime "%Y-%m-%d %H:%M GMT",gmtime $sb->mtime;
	push @flavours_tombstone, "compiler=${omc_exe} (${exe_time_stamp}) configuration=${config}";
}

logmsg info, "=========================";
logmsg info, " test_models ${script_version} ";
logmsg info, "=========================";
logmsg info, " ";
logmsg info, "Testing: ".join(", ", @model_dirs);

for (my $flavour_counter = 0; $flavour_counter <= $#flavours; $flavour_counter++) {
	logmsg info, "$flavours[$flavour_counter] settings: $flavours_tombstone[$flavour_counter]";
}
logmsg info, " ";

MODEL:
for my $model_dir (@model_dirs) {
	my $retval; # return value from external commands
	my $merged; # output from external commands
	
	# Specify working directory explicitly because errors can bail back to next model
	# with the working directory in an unknown state.
	chdir $models_root;

	my $model_path = "${models_root}/${model_dir}";
	if (! -d $model_path) {
		logmsg error, "${model_path}: Missing model";
		next MODEL;
	}
	
	# Location of model source code
	my $model_source_dir = "${model_path}/code";

	# Location of parameter folder
	my $parameters_dir = "${model_path}/parameters";

	# Location of optional model ini file to pass to model (empty if none)
    my $model_ini = '';
	my $model_ini_path = '';
	if ($model_ini_opt ne '') {
        # --ini option was supplied on command line
        $model_ini = $model_ini_opt;
		$model_ini_path = "${model_path}/${model_ini}";
		if (! -f $model_ini_path) {
            logmsg error, "Model ini ${model_ini_path} not found";
            next MODEL;
        }
	}
    else {
        # check for presence of default ini file if none specified
        my $def_ini = "${model_path}/test_models.ini";
        if (-f $def_ini) {
            $model_ini_path = $def_ini;
            $model_ini = 'test_models.ini';
        }
        else {
            # Assume that model name is same as model folder
            # and if ini with that name exists, use it.
            my $def_ini = "${model_path}/${model_dir}.ini";
            if (-f $def_ini) {
                $model_ini_path = $def_ini;
                $model_ini = "${model_dir}.ini";
            }
        }
    }

    # Default values of members and threads
    # This is for reporting purposes and to construct tombstone files
    # The ini file itself is used to specify members and threads when running the model.
    my $members = 1;
    my $threads = 1;
    # Override default values if an ini file is being used
    if (-f $model_ini_path) {
        my $Config = Config::Tiny->new;
        $Config = Config::Tiny->read($model_ini_path);
        my $v1 = $Config->{OpenM}->{SubValues};
        $members = $v1 if defined $v1;
        my $v2 = $Config->{OpenM}->{Threads};
        $threads = $v2 if defined $v2;
    }
    
    # Folder for test_model outputs and persistence
    my $test_models_dir = "${model_path}/test_models";
	make_path $test_models_dir || die;

	##############
	# Process each flavour
	#############
    my $any_flavour_errors = 0; # to suppress flavour comparison for this model after all flavours have been attempted
	FLAVOUR:
	for (my $flavour_counter = 0; $flavour_counter <= $#flavours; $flavour_counter++) {
        if ($flavour_counter > 0) {
            # insert separator line between flavours for readability
            logmsg info, $model_dir, ' ';
        }
		my $flavour = $flavours[$flavour_counter];
		my $tombstone_info = $flavours_tombstone[$flavour_counter]." members=${members}";
		my $reference_dir = "${model_path}/test_models/reference/${flavour}";
		my $current_dir = "${model_path}/test_models/current/${flavour}";
		my $current_dir_short = "${model_dir}/test_models/current/${flavour}"; # for messages
        my $flavour_dir = $flavour; # name of folder containing intermediate files for the flavour
        $flavour_dir = 'ompp' if $flavour eq 'ompp-win';
		my $project_dir = "${model_path}/${flavour_dir}";
		# Skip this flavour if place-holder file test_models.suppress is found in project folder (is flavour folder)
		next FLAVOUR if -e "${project_dir}/test_models.suppress";
		logmsg info, $model_dir, $flavour if $verbosity == 0;
        
        # if --newref, remove Reference to force replacement by Current
		if ($new_ref && -d $reference_dir) {
			logmsg info, $model_dir, $flavour, "Deleting previous Reference information" if $verbosity >= 2;
			remove_tree($reference_dir);
		}
		
        # remove Current information for this flavour
        if (-d $current_dir && !remove_tree $current_dir) {
            logmsg error, $model_dir, $flavour, "Unable to remove ${current_dir}";
            $any_flavour_errors = 1;
            next FLAVOUR;
        }
		
		# Folder for current model outputs
		my $current_outputs_dir = "${current_dir}/outputs";
		make_path $current_outputs_dir || die;

		# Folder for current generated code (can hold a copy)
		my $current_generated_code_dir = "${current_dir}/generated_code";
        # remove any pre-existing generated code folder to avoid confusion.
        remove_tree $current_generated_code_dir;
		
		# logs for current
        
		my $current_logs_dir = "${current_dir}/logs";
        my $current_logs_dir_short = "${current_dir_short}/logs"; # for messages
		make_path $current_logs_dir || die;
      
        my $current_build_log = "${current_logs_dir}/build.log";
        my $current_build_log_short = "${current_logs_dir_short}/build.log";
        unlink $current_build_log; # remove previous version if present
        
        my $current_run_log = "${current_logs_dir}/run.log";
        my $current_run_log_short = "${current_logs_dir_short}/run.log";
        unlink $current_run_log; # remove previous version if present
        
        # trace for current
        my $current_trace_txt = "${current_outputs_dir}/trace.txt";
        unlink $current_trace_txt; # remove previous version if present
		
		# File containing current tombstone information
		my $tombstone_txt = "${current_dir}/tombstone.txt";
		open TOMBSTONE_TXT, '>'.$tombstone_txt or die "Cannot open ${tombstone_txt}.";
		print TOMBSTONE_TXT $tombstone_info;
		close TOMBSTONE_TXT;
        
		# Copy model ini file if used to outputs folder under fixed name
        # Will participate in digest mechanism
        if (-f $model_ini_path) {
            #copy $model_ini_path, "${current_outputs_dir}/0_MODEL_INI.txt";
            # perform copy line by line to normalize line terminators to current OS value
            # Note that both files are opened in text mode, not binmode
            open IN_FILE, '<'.$model_ini_path;
            open OUT_FILE, '>'."${current_outputs_dir}/0_MODEL_INI.txt";
            while (<IN_FILE>) {
                my $line = $_;
                $line =~ s/\s+$//; # trim all trailing whitespace from line
                print OUT_FILE "${line}\n"; # append OS-standard delimiter
            }
            close IN_FILE;
            close OUT_FILE;
        }
		
		# Folders for reference model outputs
		my $reference_outputs_dir = "${reference_dir}/outputs";

        # Ensure existence of empty generated code folder for omc
		my $generated_code_dir = "${project_dir}/src";
		remove_tree $generated_code_dir;
		make_path $generated_code_dir;

        # The following are for measuring and reporting elapsed time of various steps
		my $start_seconds = time;
        my $elapsed_seconds;
        my $elapsed_formatted;
        
		if ($flavour eq 'modgen') {

			#####################################
			# Build model (modgen)              #
			#####################################

			# Build the model
			logmsg info, $model_dir, $flavour, "Build model and prepare Default scenario" if $report_steps;

			# Change working directory to where vcxproj, etc. are located
			chdir "${project_dir}" || die;

			# The xml file containing user-configurable build properties (same as vcxproj file)
			my $model_props = "Model.vcxproj";
			if ( ! -e $model_props ) {
				logmsg error, $model_dir, $flavour, "Missing file with configurable properties: $model_props";
                $any_flavour_errors = 1;
				next FLAVOUR;
			}
			
			# Determine model name from MODEL_NAME property
			my $model_name = get_property($model_props, 'MODEL_NAME');
			if ($model_name eq '') {
				# Default is model folder name
				$model_name = $model_dir;
			}

			# Determine scenario name from SCENARIO_NAME property
			my $scenario_name = get_property($model_props, 'SCENARIO_NAME');
			if ($scenario_name eq '') {
				# Default is 'Default'
				$scenario_name = 'Default';
			}
			
			# Project file
			my $model_vcxproj = 'model.vcxproj';
			
            $start_seconds = time;
			($merged, $retval) = capture_merged {
				my @args = (
					"${msbuild_exe}",
					"${model_vcxproj}",
					"/nologo",
					"/verbosity:normal",
					"/fileLogger",
					"/flp:Verbosity=normal",
					"/p:OM_ROOT=${om_root}",
					"/p:Configuration=${config}",
					"/p:Platform=${modgen_platform}",
					"/p:SCENARIO_NAME=${scenario_name}",
					"/p:RUN_SCENARIO=false",
					"/t:Rebuild",
					);
				if (-f $model_ini_path) {
                    # supply name of ini file to Modgen build in MODEL_INI
                    # build process will extract members, threads, etc. and insert them into default scex,
                    # which will be used when model is run later in this script.
					push @args,
						(
                            "/p:MODEL_INI=${model_ini}",
						);
				}
				system(@args);
			};
			$elapsed_seconds = time - $start_seconds;
			$elapsed_formatted = int($elapsed_seconds/60)."m ".($elapsed_seconds%60)."s";
			logmsg info, $model_dir, $flavour, "Build time ${elapsed_formatted}" if $report_timing && $verbosity >= 1;
			$start_seconds = time;

			open BUILD_LOG, ">${current_build_log}" || die;
			print BUILD_LOG $merged;
			close BUILD_LOG;
            
            # extract errors and warnings from log
            # Note that extract_errors_and_warnings truncates processing the log 
            # contextually to avoid double-counting due to msbuild output
            my ($build_error_count, $build_error_messages, $build_warning_count, $build_warning_messages) =
                extract_errors_and_warnings $current_build_log;
			if ($build_error_count) {
				logmsg error, $model_dir, $flavour, "${build_error_count} build errors - see ${current_build_log_short}";
				logerrors $build_error_messages;
                $any_flavour_errors = 1;
				next FLAVOUR;
			}
			if ($build_warning_count) {
				logmsg warning, $model_dir, $flavour, "${build_warning_count} build warnings - see ${current_build_log_short}";
			}
            
            # handle non-zero return code from msbuild, if we got this far
			if ($retval != 0) {
				logmsg error, $model_dir, $flavour, "Build failed (${retval}) see ${current_build_log_short}";
                $any_flavour_errors = 1;
				next FLAVOUR;
			}
			
			# Executable suffix for platform/configuration: nothing, D, 64, 64D
			my $build_suffix = "";
			if ($modgen_platform eq 'x64') {
				$build_suffix .= '64';
			}
			if ($config eq 'debug') {
				$build_suffix .= 'D';
			}

			# Executable model generated by build
			my $target_dir = "${project_dir}/bin";
			my $model_exe = "${target_dir}/${model_name}${build_suffix}.exe";
            
            # handle missing executable, if we got this far
			if ( ! -e $model_exe ) {
				logmsg error, $model_dir, $flavour, "Missing model executable: $model_exe";
                $any_flavour_errors = 1;
				next FLAVOUR;
			}
			
			# Scenario generated by build (align with PUBLISH_DIR in .props)
			my $publish_dir = "${project_dir}/bin";
			my $publish_scex = "${publish_dir}/${scenario_name}.scex";
            
            # handle missing scenario file, if we got this far
			if ( ! -f $publish_scex ) {
				logmsg error, $model_dir, $flavour, "Missing scenario file: ${publish_scex}";
                $any_flavour_errors = 1;
				next FLAVOUR;
			}

			#####################################
			# Run model (modgen)                #
			#####################################
			
			# Save copy of generated C++ source code
            if ($save_generated_code) {
                make_path $current_generated_code_dir;
                chdir $generated_code_dir;
                for (glob "*.h *.cpp") {
                    copy "$_", "${current_generated_code_dir}";
                }
            }
			
			chdir "${target_dir}";
            my $run_txt = "Run model";
            $run_txt .= " using ${model_ini}" if $model_ini ne '';
			logmsg info, $model_dir, $flavour, $run_txt if $report_steps;

			# Delete output database to enable subsequent check for success
			my $modgen_scenario_mdb = "${publish_dir}/${scenario_name}(tbl).mdb";
			unlink $modgen_scenario_mdb;

			$start_seconds = time;
			($merged, $retval) = capture_merged {
				my @args = (
					"${model_exe}",
					"-sc",
					"${publish_scex}",
					"-s", # Modgen silent mode
					);
				system(@args);
			};
			$elapsed_seconds = time - $start_seconds;
			$elapsed_formatted = int($elapsed_seconds/60)."m ".($elapsed_seconds%60)."s";
			logmsg info, $model_dir, $flavour, "Run time ${elapsed_formatted}" if $report_timing && $verbosity >= 1;

			# Copy model run log file to logs directory
			my $modgen_scenario_log_txt = "${publish_dir}/${scenario_name}(log).txt";
			if (-s $modgen_scenario_log_txt) {
				copy $modgen_scenario_log_txt, $current_run_log;
			}
			
            # extract errors and warnings from run log
            my ($run_error_count, $run_error_messages, $run_warning_count, $run_warning_messages) =
                extract_errors_and_warnings $current_run_log;
			if ($run_error_count) {
				logmsg error, $model_dir, $flavour, "${run_error_count} run errors - see ${current_run_log_short}";
				logerrors $run_error_messages;
                $any_flavour_errors = 1;
				next FLAVOUR;
			}
			if ($run_warning_count) {
				logmsg warning, $model_dir, $flavour, "${run_warning_count} run warnings - see ${current_run_log_short}";
			}
            
			# Modgen models return a code of 1 to indicate success
			# But return code from Perl requires shifting out 8 lower order bits to recover actual return code.
			$retval >>= 8;
			if ($retval != 1) {
				logmsg error, $model_dir, $flavour, "Run model failed - retval=${retval}";
                $any_flavour_errors = 1;
				next FLAVOUR;
			}
			if ( ! -e $modgen_scenario_mdb ) {
				logmsg error, $model_dir, $flavour, "Missing scenario database: ${modgen_scenario_mdb}";
                $any_flavour_errors = 1;
				next FLAVOUR;
			}

			$start_seconds = time;
			logmsg info, $model_dir, $flavour, "Convert outputs (${significant_digits} digits of precision)" if $report_steps;

			# If present, normalize event trace / case checksum file to results directory
			my $modgen_scenario_debug_txt = "${publish_dir}/${scenario_name}(debug).txt";
			if (-e $modgen_scenario_debug_txt) {
				normalize_event_trace $modgen_scenario_debug_txt, $current_trace_txt;
			}
			
			if ( 0 != modgen_tables_to_csv($modgen_scenario_mdb, $current_outputs_dir, $significant_digits, $unrounded_tables)) {
                $any_flavour_errors = 1;
				next FLAVOUR;
			}
			$elapsed_seconds = time - $start_seconds;
			$elapsed_formatted = int($elapsed_seconds/60)."m ".($elapsed_seconds%60)."s";
			logmsg info, $model_dir, $flavour, "Convert time ${elapsed_formatted}" if $report_timing && $verbosity >= 1;

			#####################################
			# Clean                             #
			#####################################
			if ($do_clean) {
				chdir ${project_dir};
				remove_tree "${project_dir}/src";
				remove_tree "${project_dir}/build";
				remove_tree "${project_dir}/bin";
			}
		}
		elsif ($flavour eq 'ompp-win') {
		
			#####################################
			# Build model (ompp)                #
			#####################################

			# Change working directory to project directory for compilation.
			chdir $project_dir || die;

			# The xml file containing user-configurable build properties (same as vcxproj file)
			my $model_props = "Model.vcxproj";
			if ( ! -e $model_props ) {
				logmsg error, $model_dir, $flavour, "Missing property sheet contining user macros: $model_props";
                $any_flavour_errors = 1;
				next FLAVOUR;
			}
			
			# Determine model name from MODEL_NAME property
			my $model_name = get_property($model_props, 'MODEL_NAME');
			if ($model_name eq '') {
				# Default is model folder name
				$model_name = $model_dir;
			}

			# Determine scenario name from SCENARIO_NAME property
			my $scenario_name = get_property($model_props, 'SCENARIO_NAME');
			if ($scenario_name eq '') {
				# Default is 'Default'
				$scenario_name = 'Default';
			}
			
			# Determine fixed parameter folder from user-modifiable property FIXED_PARAMETERS_FOLDER
            # SFG TODO requires alignment to changed .props symbol, FIXED_NAME, etc.
			my $fixed_parameters_folder = get_property($model_props, 'FIXED_PARAMETERS_FOLDER');
			my $enable_fixed_parameters = 'true';
			if ($fixed_parameters_folder eq '') {
				# Default is disabled
				$enable_fixed_parameters = 'false';
			}
			
			# Prepare grid computing option
			my $grid_computing = 'EMPTY';
			if ($use_mpi) {
				$grid_computing = 'MPI'
			}
			
			# Project file
			my $model_vcxproj = 'model.vcxproj';
			
			logmsg info, $model_dir, $flavour, "Build and publish model and Default scenario" if $report_steps;
            
			$start_seconds = time;
			($merged, $retval) = capture_merged {
				my @args = (
					"${msbuild_exe}",
					"${model_vcxproj}",
					"/nologo",
					"/verbosity:normal",
					"/clp:ForceNoAlign", # disable truncation of long lines
					"/p:OM_ROOT=${om_root}",
					"/p:OMC_EXE=${omc_exe}",
					"/p:Configuration=${config}",
					"/p:Platform=${windows_platform}",
					"/p:GRID_COMPUTING=${grid_computing}",
					"/p:SCENARIO_NAME=${scenario_name}",
					"/p:ENABLE_FIXED_PARAMETERS=${enable_fixed_parameters}",
					"/p:OPEN_MODEL_WEB_UI=false",
					"/p:RUN_SCENARIO=false",
					"/t:Rebuild"
					);
				system(@args);
			};
			$elapsed_seconds = time - $start_seconds;
			$elapsed_formatted = int($elapsed_seconds/60)."m ".($elapsed_seconds%60)."s";
			logmsg info, $model_dir, $flavour, "Build time ${elapsed_formatted}" if $report_timing && $verbosity >= 1;
			$start_seconds = time;

			open BUILD_LOG, ">${current_build_log}" || die;
			print BUILD_LOG $merged;
			close BUILD_LOG;

            # extract errors and warnings from log
            # Note that extract_errors_and_warnings truncates processing the log 
            # contextually to avoid double-counting due to msbuild output
            my ($build_error_count, $build_error_messages, $build_warning_count, $build_warning_messages) =
                extract_errors_and_warnings $current_build_log;
			if ($build_error_count) {
				logmsg error, $model_dir, $flavour, "${build_error_count} build errors - see ${current_build_log_short}";
				logerrors $build_error_messages;
                $any_flavour_errors = 1;
				next FLAVOUR;
			}
			if ($build_warning_count) {
				logmsg warning, $model_dir, $flavour, "${build_warning_count} build warnings - see ${current_build_log_short}";
			}
            
            # handle non-zero return code from msbuild, if we got this far
			if ($retval != 0) {
				logmsg error, $model_dir, $flavour, "build failed - see ${current_build_log_short}";
                $any_flavour_errors = 1;
				next FLAVOUR;
			}
			
			# Executable suffix for platform/configuration: nothing, D
			my $build_suffix = "";
			if ($config eq 'debug') {
				$build_suffix .= 'D';
			}
			# MPI suffix for platform/configuration: nothing, _mpi
			my $mpi_suffix = "";
			if ($use_mpi) {
				$mpi_suffix .= '_mpi';
			}
			# Executable model generated by build
			my $target_dir = "${project_dir}/bin";
			my $model_exe = "${target_dir}/${model_name}${build_suffix}${mpi_suffix}.exe";

            # handle missing executable, if we got this far
			if ( ! -e $model_exe ) {
				logmsg error, $model_dir, $flavour, "Missing model executable: $model_exe";
                $any_flavour_errors = 1;
				next FLAVOUR;
			}

			# Database generated by build (align with PUBLISH_DIR in .props
			my $publish_dir = "${project_dir}/bin";
			my $publish_sqlite = "${publish_dir}/${model_name}.sqlite";
            
            # handle missing DB, if we got this far
			if ( ! -f $publish_sqlite ) {
				logmsg error, $model_dir, $flavour, "Missing database: ${publish_sqlite}";
                $any_flavour_errors = 1;
				next FLAVOUR;
			}
			
			#####################################
			# Run model (ompp)                  #
			#####################################
			
			# Save copy of generated C++ source code
            if ($save_generated_code) {
                make_path $current_generated_code_dir;
                chdir $generated_code_dir;
                for (glob "*.h *.cpp") {
                copy "$_", "${current_generated_code_dir}";
                }
            }
			
            my $run_txt = "Run model";
			$start_seconds = time;
            $run_txt .= " using ${model_ini}" if $model_ini ne '';
			logmsg info, $model_dir, $flavour, $run_txt if $report_steps;
			
			# Change working directory to target_dir where the executable is located.
			chdir "${target_dir}";
			
			my $ompp_trace_txt = "${scenario_name}_trace.txt";
			my $ompp_log_txt = "${scenario_name}_log.txt";
			($merged, $retval) = capture_merged {
				my @args;
				if ($use_mpi && $mpi_processes > 1) {
                    # note special case when mpi_processes==1 to test mpi build without mpiexec
					my $msmpi_bin = $ENV{'MSMPI_BIN'};
					if ($msmpi_bin eq '') {
						logmsg error, $model_dir, $flavour, "MPI requested but Microsoft MPI not installed";
                        $any_flavour_errors = 1;
						next FLAVOUR;
					}
					push @args,
						(
						"${msmpi_bin}mpiexec.exe",
						"-n", ${mpi_processes}
						);
				}
				push @args,
					(
					"${model_exe}",
					"-OpenM.LogToFile", "true",
					"-OpenM.LogFilePath", $ompp_log_txt,
					"-OpenM.TraceToFile", "true",
					"-OpenM.TraceFilePath", $ompp_trace_txt,
					"-OpenM.RunName", $scenario_name,
					"-OpenM.Database", "Database=${publish_sqlite}; Timeout=86400; OpenMode=ReadWrite;",
					"-OpenM.ProgressPercent", "25",
					);
				if (-f $model_ini_path) {
					push @args,
						(
						"-ini", $model_ini_path,
                        "-OpenM.IniAnyKey", "true" # just in case
						);
				}
				system(@args);
			};
			$elapsed_seconds = time - $start_seconds;
			$elapsed_formatted = int($elapsed_seconds/60)."m ".($elapsed_seconds%60)."s";
			logmsg info, $model_dir, $flavour, "Run time ${elapsed_formatted}" if $report_timing;

			# Copy model run log file to logs directory
			if (-s $ompp_log_txt) {
				copy $ompp_log_txt, $current_run_log;
			}
            
            # extract errors and warnings from run log
            my ($run_error_count, $run_error_messages, $run_warning_count, $run_warning_messages) =
                extract_errors_and_warnings $current_run_log;
			if ($run_error_count) {
				logmsg error, $model_dir, $flavour, "${run_error_count} run errors - see ${current_run_log_short}";
				logerrors $run_error_messages;
                $any_flavour_errors = 1;
				next FLAVOUR;
			}
			if ($run_warning_count) {
				logmsg warning, $model_dir, $flavour, "${run_warning_count} run warnings - see ${current_run_log_short}";
			}
            
            # handle non-zero return code from model, if we got this far
			if ($retval != 0) {
				logmsg error, $model_dir, $flavour, "Run ${scenario_name} scenario failed";
                $any_flavour_errors = 1;
				next FLAVOUR;
			}
			
			$start_seconds = time;
			logmsg info, $model_dir, $flavour, "Convert outputs (${significant_digits} digits of precision)" if $report_steps;
			if ( 0 != ompp_tables_to_csv($publish_sqlite, $current_outputs_dir, $significant_digits, $unrounded_tables)) {
                $any_flavour_errors = 1;
				next FLAVOUR;
			}
			$elapsed_seconds = time - $start_seconds;
			$elapsed_formatted = int($elapsed_seconds/60)."m ".($elapsed_seconds%60)."s";
			logmsg info, $model_dir, $flavour, "Convert time ${elapsed_formatted}" if $report_timing;
		
			# If present and non-empty event trace, normalize to results
			if (-s $ompp_trace_txt) {
				normalize_event_trace $ompp_trace_txt, $current_trace_txt;
			}
			
			#####################################
			# Clean                             #
			#####################################
			if ($do_clean) {
				chdir ${project_dir};
				remove_tree "${project_dir}/src";
				remove_tree "${project_dir}/build";
				remove_tree "${project_dir}/bin";
			}
		}
		elsif ($flavour eq 'ompp-linux' || $flavour eq 'ompp-mac') {
		
			########################################
			# Build model (ompp-linux or ompp-mac) #
			########################################
			
			# Change working directory to model path for make (location of makefile)
			chdir ${model_path};

            # makefile has no internal mechanism to specify these names, so use defaults
			my $model_name = $model_dir;
			my $scenario_name = 'Default';

            my $model_props = 'ompp/Model.vcxproj';
			# Determine fixed parameter folder from user-modifiable property FIXED_PARAMETERS_FOLDER
			# Determine scenario name from SCENARIO_NAME property
			$scenario_name = get_property($model_props, 'SCENARIO_NAME');
			if ($scenario_name eq '') {
				# Default is 'Default'
				$scenario_name = 'Default';
			}
			
			# Determine fixed parameter folder from user-modifiable property FIXED_PARAMETERS_FOLDER
			my $fixed_parameters_folder = get_property($model_props, 'FIXED_PARAMETERS_FOLDER');
			my $enable_fixed_parameters = 'true';
			if ($fixed_parameters_folder eq '') {
				# Default is disabled
				$enable_fixed_parameters = 'false';
			}
			# SFG TODO partial hack to account for fact that vcxproj is one level deeper than makefile
			# trim off leading '../' if present
			$fixed_parameters_folder =~ s@^../@@;
					
			my @make_defines;
			push @make_defines, "MODEL_NAME=${model_name}";
			push @make_defines, "SCENARIO_NAME=${scenario_name}";
			push @make_defines, "OMC_FIXED_PARAM_DIR=${fixed_parameters_folder}" if $enable_fixed_parameters eq 'true';
			push @make_defines, 'RELEASE=1' if $config eq 'release';
			push @make_defines, 'OM_MSG_USE==MPI' if $use_mpi;
			
			my $build_log = "${project_dir}/make.log";
			unlink $build_log;
			open BUILD_LOG, ">${build_log}" || die;
			logmsg info, $model_dir, $flavour, "Build and publish model and Default scenario" if $report_steps;
			$start_seconds = time;
			for my $make_target ('cleanall', 'model', 'publish-views') {
				($merged, $retval) = capture_merged {
					my @args = (
						"make",
						"${make_target}",
						);
					push @args, @make_defines;
					logmsg info, $model_dir, $flavour, join(' ', @args) if $verbosity >= 2;
					system(@args);
				};
				print BUILD_LOG $merged; # append make output to log
			}
			close BUILD_LOG;
			copy $build_log, $current_build_log;
			$elapsed_seconds = time - $start_seconds;
			$elapsed_formatted = int($elapsed_seconds/60)."m ".($elapsed_seconds%60)."s";
			logmsg info, $model_dir, $flavour, "Build time ${elapsed_formatted}" if $report_timing && $verbosity >= 1;
			$start_seconds = time;
			
            # extract errors and warnings from log
            my ($build_error_count, $build_error_messages, $build_warning_count, $build_warning_messages) =
                extract_errors_and_warnings $current_build_log;
			if ($build_error_count) {
				logmsg error, $model_dir, $flavour, "${build_error_count} build errors - see ${current_build_log_short}";
				logerrors $build_error_messages;
                $any_flavour_errors = 1;
				next FLAVOUR;
			}
			if ($build_warning_count) {
				logmsg warning, $model_dir, $flavour, "${build_warning_count} build warning(s) - see ${current_build_log_short}";
			}
            
			# suffix for configuration: nothing or D
			my $build_suffix = "";
			$build_suffix .= 'D' if $config eq 'debug';
			
			# Executable model generated by build
			my $target_dir = "${project_dir}/bin";
			my $model_exe = "${target_dir}/${model_name}${build_suffix}";

            # handle missing executable, if we got this far
			if ( ! -e $model_exe ) {
				logmsg error, $model_dir, $flavour, "Missing model executable: $model_exe";
                $any_flavour_errors = 1;
				next FLAVOUR;
			}

			# Database generated by build (align with PUBLISH_DIR in makefile, .props
			my $publish_dir = "${project_dir}/bin";
			my $publish_sqlite = "${publish_dir}/${model_name}.sqlite";

            # handle missing DB, if we got this far
			if ( ! -f $publish_sqlite ) {
				logmsg error, $model_dir, $flavour, "Missing database: ${publish_sqlite}";
                $any_flavour_errors = 1;
				next FLAVOUR;
			}


			######################################
			# Run model (ompp-linux or ompp-mac) #
			######################################
			
			# Save copy of generated C++ source code
			my $omc_generated_code_dir = "${project_dir}/build/${config}/src";
			
            if ($save_generated_code) {
                make_path $current_generated_code_dir;
                chdir $omc_generated_code_dir;
                for (glob "*.h *.cpp") {
                    copy "$_", "${current_generated_code_dir}";
                }
            }

            my $run_txt = "Run model";
            $run_txt .= " using $model_ini" if $model_ini ne '';
			logmsg info, $model_dir, $flavour, $run_txt if $report_steps;
			# Change working directory to target_dir where the executable is located.
			chdir "${target_dir}";
			
			$start_seconds = time;
			my $ompp_trace_txt = "${scenario_name}_trace.txt";
			my $ompp_log_txt = "${scenario_name}_log.txt";
			($merged, $retval) = capture_merged {
				my @args = (
					"${model_exe}",
					"-OpenM.LogToFile", "true",
					"-OpenM.LogFilePath", $ompp_log_txt,
					"-OpenM.TraceToFile", "true",
					"-OpenM.TraceFilePath", $ompp_trace_txt,
					"-OpenM.RunName", $scenario_name,
					"-OpenM.Database", "Database=${publish_sqlite}; Timeout=86400; OpenMode=ReadWrite;",
					"-OpenM.SubValues", $members,
					"-OpenM.Threads", $threads,
					);
				if ($model_ini_path ne '') {
					push @args,
						(
						"-ini", $model_ini_path,
                        "-OpenM.IniAnyKey", "true" # just in case
						);
				}
				system(@args);
			};
			$elapsed_seconds = time - $start_seconds;
			$elapsed_formatted = int($elapsed_seconds/60)."m ".($elapsed_seconds%60)."s";
			logmsg info, $model_dir, $flavour, "Run time ${elapsed_formatted}" if $report_timing;
            
			# Copy model run log file to logs directory
			if (-s $ompp_log_txt) {
				copy $ompp_log_txt, $current_run_log;
			}
			
            # extract errors and warnings from run log
            my ($run_error_count, $run_error_messages, $run_warning_count, $run_warning_messages) =
                extract_errors_and_warnings $current_run_log;
			if ($run_error_count) {
				logmsg error, $model_dir, $flavour, "${run_error_count} run errors - see ${current_run_log_short}";
				logerrors $run_error_messages;
                $any_flavour_errors = 1;
				next FLAVOUR;
			}
			if ($run_warning_count) {
				logmsg warning, $model_dir, $flavour, "${run_warning_count} run warnings - see ${current_run_log_short}";
			}
            
            # handle non-zero return code from model, if we got this far
			if ($retval != 0) {
				logmsg error, $model_dir, $flavour, "Run ${scenario_name} scenario failed";
                $any_flavour_errors = 1;
				next FLAVOUR;
			}

			logmsg info, $model_dir, $flavour, "Convert outputs (${significant_digits} digits of precision)" if $report_steps;
			$start_seconds = time;
			if ( 0 != ompp_tables_to_csv($publish_sqlite, $current_outputs_dir, $significant_digits, 1)) {
                $any_flavour_errors = 1;
				next FLAVOUR;
			}
			$elapsed_seconds = time - $start_seconds;
			$elapsed_formatted = int($elapsed_seconds/60)."m ".($elapsed_seconds%60)."s";
			logmsg info, $model_dir, $flavour, "Convert time ${elapsed_formatted}" if $report_timing && $verbosity >= 1;

			# If present and non-empty copy event trace / case checksum file to results directory
			if (-s $ompp_trace_txt) {
				normalize_event_trace $ompp_trace_txt, $current_trace_txt;
			}
			
			#####################################
			# Clean                             #
			#####################################
			if ($do_clean) {
				chdir ${project_dir};
				remove_tree "${project_dir}/src";
				remove_tree "${project_dir}/build";
				remove_tree "${project_dir}/bin";
			}
		}
		else {
			logmsg error, $model_dir, "logic error!";
			exit -1;
		}
		
		#######################################
		# Back to code common to all flavours #
		#######################################
		
		# Create file to record significant digits used in csv creation
        # Leading ! is in file name so that it will be in first files reported if different
        # Capitalized to make it stand out in report.
		open SIGNIFICANT_DIGITS_TXT, ">${current_outputs_dir}/0_SIGNIFICANT_DIGITS.txt";
		print SIGNIFICANT_DIGITS_TXT "significant digits = ${significant_digits}\n";
		close SIGNIFICANT_DIGITS_TXT;

		# Create digests of outputs
		$start_seconds = time;
		logmsg info, $model_dir, $flavour, "Create digests of current outputs" if $report_steps;
		chdir "${current_outputs_dir}";
		my $digests = create_digest(glob "[a-zA-Z]*.csv *.txt");
        $elapsed_seconds = time - $start_seconds;
        $elapsed_formatted = int($elapsed_seconds/60)."m ".($elapsed_seconds%60)."s";
        logmsg info, $model_dir, $flavour, "Digest time ${elapsed_formatted}" if $report_timing && $verbosity >= 1;
		open DIGESTS_TXT, ">digests.txt";
		print DIGESTS_TXT $digests;
		close DIGESTS_TXT;
		
		# If no reference directory, copy current to reference
		if (! -d $reference_dir) {
			logmsg info, $model_dir, $flavour, "No Reference outputs - create using Current outputs" if $verbosity >= 2;
			dircopy $current_dir, $reference_dir;
		}
		
		# Report on version differences based on tombstone info
		my $tombstone_reference_txt = "${reference_dir}/tombstone.txt";
		my $tombstone_current_txt = "${current_dir}/tombstone.txt";
		if (! -e $tombstone_reference_txt || ! -e $tombstone_current_txt) {
			logmsg error, $model_dir, $flavour, "Tombstone file(s) missing";
		}
		else {
			if (compare($tombstone_reference_txt, $tombstone_current_txt) != 0) {
				logmsg info, $model_dir, $flavour, "Current vs. Reference: tombstone info differs:" if $verbosity >= 1;
				open FILE, "<".$tombstone_reference_txt || die "unable to open ${tombstone_reference_txt}";
				my $tombstone_reference = <FILE>;
				close FILE;
				logmsg info, $model_dir, $flavour, "  Reference: ${tombstone_reference}" if $verbosity >= 1;
				open FILE, "<".$tombstone_current_txt || die "unable to open ${tombstone_current_txt}";
				my $tombstone_current = <FILE>;
				close FILE;
				logmsg info, $model_dir, $flavour, "  Current:   ${tombstone_current}" if $verbosity >= 1;
			}
		}
		
		# Report on differences between Current and Reference outputs
		my $digests_txt_1 = "${reference_outputs_dir}/digests.txt";
		my $digests_txt_2 = "${current_outputs_dir}/digests.txt";
		my ($not_in_1, $not_in_2, $differs, $same) = digest_differences($digests_txt_1, $digests_txt_2);
        
        # Report on files which are the same
        {

            # total number of files in union of Current and Reference)
            my $count_total = 
                  scalar(split(',',$not_in_1))
                + scalar(split(',',$not_in_2))
                + scalar(split(',',$differs))
                + scalar(split(',',$same));
            my @files = split(',',$same);
            my $count = @files;
            logmsg info, $model_dir, $flavour, sprintf 'Current vs. Reference: %3d the same (of %d)', $count, $count_total;
        }
        if ('' ne $not_in_2) {
            my @files = split(',',$not_in_2);
            my $count = @files;
            logmsg warning, $model_dir, $flavour, sprintf 'Current vs. Reference: %3d only in Reference', $count ;
            if ($report_all_files) {
                foreach my $file (@files) {
                    logmsg warning, $model_dir, $flavour, "Current vs. Reference: Only in reference: ${file}" ;
                }
            }
        }
        if ('' ne $not_in_1) {
            my @files = split(',',$not_in_1);
            my $count = @files;
            logmsg warning, $model_dir, $flavour, sprintf 'Current vs. Reference: %3d only in Current', $count ;
            if ($report_all_files) {
                foreach my $file (@files) {
                    logmsg warning, $model_dir, $flavour, "Current vs. Reference: Only in current:   ${file}" ;
                }
            }
        }
        if ('' ne $differs) {
            my @files = split(',',$differs);
            my $count = @files;
            logmsg change, $model_dir, $flavour, sprintf 'Current vs. Reference: %3d differ', $count ;
            my $reported = 0;
            my $remaining = $count;
            my $max_to_report = 5;
            foreach my $file (@files) {
                logmsg change, $model_dir, $flavour, "Current vs. Reference: DIFFERS:           ${file}" ;
                $reported++;
                $remaining--;
                if (!$report_all_files && $reported == $max_to_report && $remaining > 0) {
                    logmsg change, $model_dir, $flavour, "Current vs. Reference: DIFFERS:           ... and ${remaining} other(s)";
                    last;
                }
            }
        }
	} # flavour

	########
	# compare results between 2 flavours, for reference and current
	########

	COMPARE:
    if ($do_flavour_comparison) {
        if ($any_flavour_errors) {
			logmsg info, $model_dir, ' ', 'Suppressing flavour comparison due to previous build/run errors';
            next MODEL;
        }
        # The flavour pairs to compare, comma-separated.
        # Reeported as 'first vs. second'
        my @flavour_pairs;
        
        if ($is_windows) {
            @flavour_pairs = (
                'ompp-win,modgen',
                'ompp-win,ompp-linux',
                'ompp-win,ompp-macos',
            );
        }
		if ($is_linux) {
            @flavour_pairs = (
                'ompp-linux,ompp-win',
                'ompp-linux,ompp-macos',
                'ompp-linux,modgen',
            );
		}
		if ($is_darwin) {
            @flavour_pairs = (
                'ompp-mac,ompp-win',
                'ompp-mac,ompp-linux',
                'ompp-mac,modgen',
            );
		}
        
        my $comparison_header_done = 0;
        
        for my $flavour_pair (@flavour_pairs) {
            my ($flavour2, $flavour1) = split ',', $flavour_pair;
            #my $flavour_pair_formatted = $flavour2.' vs. '.$flavour1;
            # right pad with spaces to common length
            my $flavour_pair_formatted = sprintf '%-24s', $flavour2.' vs. '.$flavour1;
            # Check for differences in outputs between two flavours
            foreach my $model_version ('reference', 'current') {
                my $model_version_formatted = ($model_version eq 'reference') ? 'Reference:' : 'Current:  ';
                my $digests_txt_1 = "${model_path}/test_models/${model_version}/${flavour1}/outputs/digests.txt";
                my $digests_txt_2 = "${model_path}/test_models/${model_version}/${flavour2}/outputs/digests.txt";
                if (! -e $digests_txt_1) {
                    next;
                }
                elsif (! -e $digests_txt_2) {
                    next;
                }
                else {
                    if (!$comparison_header_done) {
                        $comparison_header_done = 1;
                        logmsg info, $model_dir, ' '; # insert separator line for readability
                        logmsg info, $model_dir, '    Flavour comparisons', ' ';
                    }
                    logmsg info, $model_dir, ' '; # insert separator line for readability
                    my ($not_in_1, $not_in_2, $differs, $same) = digest_differences($digests_txt_1, $digests_txt_2);
                    # Report on files which are the same
                    {

                        # total number of files in union of flavours 1 and 2
                        my $count_total = 
                              scalar(split(',',$not_in_1))
                            + scalar(split(',',$not_in_2))
                            + scalar(split(',',$differs))
                            + scalar(split(',',$same));
                        my @files = split(',',$same);
                        my $count = @files;
                        logmsg info, $model_dir, $flavour_pair_formatted, sprintf "${model_version_formatted} %3d the same (of %d)", $count, $count_total;
                    }
                    if ('' ne $not_in_2) {
                        my @files = split(',',$not_in_2);
                        my $count = @files;
                        logmsg warning, $model_dir, $flavour_pair_formatted, sprintf "${model_version_formatted} %3d only in ${flavour1}", $count ;
                        if ($report_all_files) {
                            foreach my $file (@files) {
                                logmsg warning, $model_dir, $flavour_pair_formatted, "${model_version_formatted} only in ${flavour1}: ${file}" ;
                            }
                        }
                    }
                    if ('' ne $not_in_1) {
                        my @files = split(',',$not_in_1);
                        my $count = @files;
                        logmsg warning, $model_dir, $flavour_pair_formatted, sprintf "${model_version_formatted} %3d only in ${flavour2}", $count;
                        if ($report_all_files) {
                            foreach my $file (@files) {
                                logmsg warning, $model_dir, $flavour_pair_formatted, "${model_version_formatted} only in ${flavour2}: ${file}" ;
                            }
                        }
                    }
                    if ('' ne $differs) {
                        my @files = split(',',$differs);
                        my $count = @files;
                        logmsg change, $model_dir, $flavour_pair_formatted, sprintf "${model_version_formatted} %3d differ", $count;
                        my $reported = 0;
                        my $remaining = $count;
                        my $max_to_report = 5;
                        foreach my $file (@files) {
                            logmsg change, $model_dir, $flavour_pair_formatted, "${model_version_formatted} DIFFERS: ${file}" ;
                            $reported++;
                            $remaining--;
                            if (!$report_all_files && $reported == $max_to_report && $remaining > 0) {
                                logmsg change, $model_dir, $flavour_pair_formatted, "${model_version_formatted} DIFFERS: ... and ${remaining} other(s)";
                                last;
                            }
                        }
                    }
                }
            }
        }
	}
	
	logmsg info, " " if $verbosity >= 1;
	
} # model_dir
