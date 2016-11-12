# Copyright (c) 2013-2016 OpenM++
# This code is licensed under MIT license (see LICENSE.txt for details)

# Script to test multiple models in ompp, ompp-linux, and modgen

use strict;
use warnings;

my $script_name = "test_models";
my $script_version = '1.2';

use Getopt::Long::Descriptive;

my ($opt, $usage) = describe_options(
	$script_name.' %o model...',
	[ 'help|h'    => 'print usage message and exit' ],
	[ 'version|v' => 'print version and exit' ],
	[ 'models_root|m=s'  => 'directory containing models (default is ../models)',
		{ default => '../models' } ],
	[ 'newref' => 'replace existing reference files' ],
	[ 'nocleanup' => 'leave intermediate build files' ],
	[ 'significant_digits=i' => 'significant digits (default 7)',
		{ default => 7 } ],
	[ 'nounroundedtables' => 'suppress creation of unrounded versions of tables' ],
	[ 'noompp' => 'skip OpenM++ build and run' ],
	[ 'ompp_config=s' => 'OpenM++ config: Debug or Release(default)',
		{ default => 'Release' } ],
	[ 'ompp_platform=s' => 'OpenM++ platform: Win32(default) or x64',
		{ default => 'Win32' } ],
	[ 'nomodgen' => 'skip Modgen build and run' ],
	[ 'modgen_version=s' => 'Modgen version: 11 or 12 or 12.1(default)',
		{ default => '12.1' } ],
	[ 'modgen_config=s' => 'Modgen config: Debug or Release(default)',
		{ default => 'Release' } ],
	[ 'modgen_platform=s' => 'Modgen platform: Win32(default) or x64',
		{ default => 'Win32' } ],
);

if ($opt->version) {
	print $script_name.' version '.$script_version."\n";
	exit 0;
}

if ($opt->help) {
	print $usage->text;
	exit 0;
}

use Cwd qw(abs_path);
my $script_path = abs_path($0);
$script_path =~ s@[^/\\]*$@@;
#print "script_path=".$script_path."\n";


#####################
# Common settings
#####################

# Verbosity of report:
#  0 - errors only
#  1 - timing and success
#  2 - chatty
my $verbosity = 3;

# Number of significant digits to retain in the output csv files (0 means full precision)
my $significant_digits = $opt->significant_digits;

# Also create an non-rounded version of each output csv table
my $unrounded_tables = 1;
$unrounded_tables = 0 if $opt->nounroundedtables;


#####################
# ompp settings for Windows
#####################

# ompp compiler version for Windows (4 possible values), without leading path
my $omc_exe = 'omc.exe';
#my $omc_exe = 'omcD.exe';
#my $omc_exe = 'omc64.exe';
#my $omc_exe = 'omc64D.exe';

my $ompp_config = $opt->ompp_config;
if ($ompp_config ne 'Debug' && $ompp_config ne 'Release') {
	print "Invalid ompp_config=$ompp_config must be Debug or Release";
	exit 1;
} 

my $ompp_platform = $opt->ompp_platform;
if ($ompp_platform ne 'Win32' && $ompp_platform ne 'x64') {
	print "Invalid ompp_platform=$ompp_platform must be Win32 or x64";
	exit 1;
} 


#####################
# ompp-linux settings
#####################

#my $ompp_linux_configuration = "debug";
my $ompp_linux_configuration = "release";

#####################
# modgen settings
#####################

my $modgen_version = $opt->modgen_version;
if ($modgen_version ne '11' && $modgen_version ne '12' && $modgen_version ne '12.1') {
	print "Invalid modgen_version=$modgen_version must be 11, 12, or 12.1";
	exit 1;
} 

my $modgen_config = $opt->modgen_config;
if ($modgen_config ne 'Debug' && $modgen_config ne 'Release') {
	print "Invalid modgen_config=$modgen_config must be Debug or Release";
	exit 1;
} 

my $modgen_platform = $opt->modgen_platform;
if ($modgen_platform ne 'Win32' && $modgen_platform ne 'x64') {
	print "Invalid modgen_platform=$modgen_platform must be Win32 or x64";
	exit 1;
} 

#####################
# There are no configuration settings in subsequent code in this script
#####################

use strict;

use Cwd qw(getcwd);

my $om_root = $ENV{'OM_ROOT'};
if ( $om_root eq '') {
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
my $models_root = $opt->models_root;
chdir $models_root || die "Folder ${models_root} not found";
$models_root = getcwd; # to ensure is absolute path

#####################
# file locations
#####################

# MSBuild command line reference:
# http://msdn.microsoft.com/en-us/library/ms164311.aspx
my $msbuild_exe = "C:\\Program Files (x86)\\MSBuild\\14.0\\Bin\\MSBuild.exe";
my $create_db_sqlite_sql = "${om_root}/sql/create_db.sql";


# determine if running on windows or linux
my $is_windows = 1;
my $is_linux = 0;
if ( $^O eq 'linux') {
	$is_windows = 0;
	$is_linux = 1;
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
				get_user_macro
				normalize_event_trace
		    );

if ($is_windows) {
	# use the common_windows.pm module of shared functions
	#require common_windows qw(
	#				modgen_version
	#				run_jet_statement
	#				modgen_tables_to_csv
	#			);
	require "${script_path}/common_windows.pm";
}

###############
# Determine flavours and options
###############

my $new_ref = ($opt->newref) ? 1 : 0;
my $do_cleanup = ($opt->nocleanup) ? 0 : 1;
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
		my $modgen_folder;
		$modgen_folder = 'Modgen 11' if $modgen_version eq '11';
		$modgen_folder = 'Modgen 12' if $modgen_version eq '12';
		# Modgen 12.1 has same folder as 12.0, but is treated differently (uses VS 2015 instead of VS 2013).
		$modgen_folder = 'Modgen 12' if $modgen_version eq '12.1';
		my $modgen_exe = "C:\\Program Files (x86)\\StatCan\\${modgen_folder}\\Modgen.exe";
		if ( ! -e $modgen_exe ) {
			logmsg error, "Missing Modgen compiler: $modgen_exe";
			exit 1;
		}
		push @flavours, 'modgen';
		my $modgen_version_string = modgen_version($modgen_exe);
		
		# Consistency checks of Modgen installed version and version specified in command argument (or default)
		(my $v1, my $v2, my $v3, my $v4) = split(',', $modgen_version_string);
		if ($modgen_version eq '12') {
			if (!($v1 == 12 && $v2 == 0)) {
				logmsg error, "Mismatch of modgen compiler - modgen_version=${modgen_version} and compiler is ${modgen_version_string} ";
				exit 1;
			}
		}
		if ($modgen_version eq '12.1') {
			if (!($v1 == 12 && $v2 == 1)) {
				logmsg error, "Mismatch of modgen compiler - modgen_version=${modgen_version} and compiler is ${modgen_version_string} ";
				exit 1;
			}
		}
		
		my $sb = stat($modgen_exe);
		my $exe_time_stamp = strftime "%Y-%m-%d %H:%M",localtime $sb->mtime;
		push @flavours_tombstone, "version=${modgen_version_string} (${exe_time_stamp}) platform=${modgen_platform} configuration=${modgen_config}";
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
		push @flavours, 'ompp';
		my $full_path = "${om_root}/bin/${omc_exe}";
		-e $full_path or die "Missing ${full_path}"; # shouldn't happen
		my $sb = stat($full_path);
		my $exe_time_stamp = strftime "%Y-%m-%d %H:%M",localtime $sb->mtime;
		push @flavours_tombstone, "compiler=${omc_exe} (${exe_time_stamp}) platform=${ompp_platform} configuration=${ompp_config}";
	}
}
else { # linux
	push @flavours, 'ompp-linux';
	my $omc_exe = 'omc';
	my $full_path = "${om_root}/bin/${omc_exe}";
	-e $full_path or die "Missing ${full_path}"; # shouldn't happen
	my $sb = stat($full_path);
	my $exe_time_stamp = strftime "%Y-%m-%d %H:%M",localtime $sb->mtime;
	push @flavours_tombstone, "compiler=${omc_exe} (${exe_time_stamp}) configuration=${ompp_linux_configuration}";
}
	
logmsg info, "=========================";
logmsg info, " test_models.pl ${script_version} ";
logmsg info, "=========================";
logmsg info, " ";
logmsg info, "Testing model folders: ".join(", ", @model_dirs);

for (my $j = 0; $j <= $#flavours; $j++) {
	logmsg info, "$flavours[$j] settings: $flavours_tombstone[$j]";
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

	##############
	# Process each flavour
	#############
	FLAVOUR:
	for (my $j = 0; $j <= $#flavours; $j++) {
		my $start_seconds = time;
		my $flavour = $flavours[$j];
		my $tombstone = $flavours_tombstone[$j];
		my $reference_dir = "${model_path}/test_models/reference/${flavour}";
		my $current_dir = "${model_path}/test_models/current/${flavour}";
		my $project_dir = "${model_path}/${flavour}";
		make_path $project_dir if -d "${project_dir}";
		# Skip this flavour if place-holder file skip_model found in project folder
		next FLAVOUR if -e "${project_dir}/skip_model";
		logmsg info, $model_dir, $flavour if $verbosity == 0;
		if ($new_ref) {
			logmsg info, $model_dir, $flavour, "Deleting reference directory" if $verbosity >= 2;
			remove_tree($reference_dir);
		}
		
		remove_tree $current_dir;
		
		# Folder for current model outputs
		my $current_outputs_dir = "${current_dir}/outputs";
		make_path $current_outputs_dir;

		# Folder for current generated code (holds a copy)
		my $current_generated_code_dir = "${current_dir}/generated_code";
		make_path $current_generated_code_dir;
		
		# Folder for current logs (holds a copy)
		my $current_logs_dir = "${current_dir}/logs";
		make_path $current_logs_dir;
		
		# File containing current tombstone information
		my $tombstone_txt = "${current_dir}/tombstone.txt";
		open TOMBSTONE_TXT, '>'.$tombstone_txt or die "Cannot open ${tombstone_txt}.";
		print TOMBSTONE_TXT $tombstone;
		close TOMBSTONE_TXT;
		
		# Folders for reference model outputs
		my $reference_outputs_dir = "${reference_dir}/outputs";

		my $generated_code_dir = "${project_dir}/build/src";
		remove_tree $generated_code_dir;
		make_path $generated_code_dir;

		if ($flavour eq 'modgen') {

			#####################################
			# Build model (modgen)              #
			#####################################

			# Build the model
			logmsg info, $model_dir, $flavour, "Modgen compile, C++ compile, build executable" if $verbosity >= 2;

			# Change working directory to where vcxproj, etc. are located
			chdir "${project_dir}" || die;

			# The property sheet containing user macros
			my $model_props = "Model.props";
			if ( ! -e $model_props ) {
				logmsg error, $model_dir, $flavour, "Missing property sheet with user macros: $model_props";
				next FLAVOUR;
			}
			
			# Determine executable model name from MODEL_NAME user macro in Model.props
			# Default is model folder name
			my $model_name = get_user_macro($model_props, 'MODEL_NAME');
			if ($model_name eq '') {
				$model_name = $model_dir;
			}

			# Determine scenario name from SCENARIO_NAME user macro in Model.props
			my $scenario_name = get_user_macro($model_props, 'SCENARIO_NAME');
			if ($scenario_name eq '') {
				logmsg error, $model_dir, $flavour, "failed to get SCENARIO_NAME from ${model_props}";
				next FLAVOUR;
			}
			
			# Project file
			my $model_vcxproj = 'model.vcxproj';
			
			# Log file from build
			my $build_log = "msbuild.log";
			unlink $build_log;
			($merged, $retval) = capture_merged {
				my @args = (
					"${msbuild_exe}",
					"${model_vcxproj}",
					"/nologo",
					"/verbosity:normal",
					"/fileLogger",
					"/flp:Verbosity=normal",
					"/p:OM_ROOT=${om_root}",
					"/p:Configuration=${modgen_config}",
					"/p:Platform=${modgen_platform}",
					"/p:MODGEN_VERSION=${modgen_version}",
					"/p:SCENARIO_NAME=${scenario_name}",
					"/p:RUN_SCENARIO=0",
					"/t:Rebuild",
					);
				system(@args);
			};
			open BUILD_LOG, ">${build_log}";
			print BUILD_LOG $merged;
			close BUILD_LOG;
			if ($retval != 0) {
				logmsg error, $model_dir, $flavour, "Build failed (${retval}) see ${build_log}";
				logerrors $merged;
				next FLAVOUR;
			}
			
			# Extract warning count 
			$merged =~ /^ *([0-9]+) Warning\(s\)/m;
			my $warning_count = $1;
			if ($warning_count) {
				logmsg warning, $model_dir, $flavour, "${warning_count} build warnings - see ${build_log}";
			}

			# Executable suffix for platform/configuration: nothing, D, 64, 64D
			my $build_suffix = "";
			if ($modgen_platform eq 'x64') {
				$build_suffix .= '64';
			}
			if ($modgen_config eq 'Debug') {
				$build_suffix .= 'D';
			}

			# Executable model generated by build
			my $target_dir = "${project_dir}/bin";
			my $model_exe = "${target_dir}/${model_name}${build_suffix}.exe";
			if ( ! -e $model_exe ) {
				logmsg error, $model_dir, $flavour, "Missing model executable: $model_exe";
				logerrors $merged;
				next FLAVOUR;
			}
			copy $build_log, $current_logs_dir;
			
			# Scenario generated by build (align with PUBLISH_DIR in .props)
			my $publish_dir = "${project_dir}/../output-modgen";
			my $publish_scex = "${publish_dir}/${scenario_name}.scex";
			if ( ! -f $publish_scex ) {
				logmsg error, $model_dir, $flavour, "Missing scenario file: ${publish_scex}";
				logerrors $merged;
				next FLAVOUR;
			}

			my $elapsed_seconds = time - $start_seconds;
			my $elapsed_formatted = int($elapsed_seconds/60)."m ".($elapsed_seconds%60)."s";
			logmsg info, $model_dir, $flavour, "Build time ${elapsed_formatted}" if $verbosity >= 1;
			$start_seconds = time;

			#####################################
			# Run model (modgen)                #
			#####################################
			
			# Save copy of generated C++ source code
			chdir $generated_code_dir;
			for (glob "*.h *.cpp") {
				copy "$_", "$current_generated_code_dir";
			}
			
			chdir "${target_dir}";
			logmsg info, $model_dir, $flavour, "Run model" if $verbosity >= 2;

			# Delete output database to enable subsequent check for success
			my $modgen_scenario_mdb = "${publish_dir}/${scenario_name}(tbl).mdb";
			unlink $modgen_scenario_mdb;

			($merged, $retval) = capture_merged {
				my @args = (
					"${model_exe}",
					"-sc",
					"${publish_scex}",
					"-s", # Modgen silent mode
					);
				system(@args);
			};
			# Modgen models return a code of 1 to indicate success
			# But return code from Perl requires shifting out 8 lower order bits to recover actual return code.
			$retval >>= 8;
			if ($retval != 1) {
				logmsg error, $model_dir, $flavour, "Run model failed - retval=${retval}";
				logerrors $merged;
				next FLAVOUR;
			}
			if ( ! -e $modgen_scenario_mdb ) {
				logmsg error, $model_dir, $flavour, "Missing scenario database: ${modgen_scenario_mdb}";
				next FLAVOUR;
			}

			$elapsed_seconds = time - $start_seconds;
			$elapsed_formatted = int($elapsed_seconds/60)."m ".($elapsed_seconds%60)."s";
			logmsg info, $model_dir, $flavour, "Run time ${elapsed_formatted}" if $verbosity >= 1;

			# Copy model run log file to logs directory
			my $modgen_scenario_log_txt = "${publish_dir}/${scenario_name}(log).txt";
			if (-s $modgen_scenario_log_txt) {
				copy $modgen_scenario_log_txt, $current_logs_dir;
			}
			
			# If present, copy event trace / case checksum file to results directory
			my $modgen_scenario_debug_txt = "${publish_dir}/${scenario_name}(debug).txt";
			if (-e $modgen_scenario_debug_txt) {
				normalize_event_trace $modgen_scenario_debug_txt, "${current_outputs_dir}/trace.txt";
			}
			
			logmsg info, $model_dir, $flavour, "Convert output tables to .csv (${significant_digits} digits of precision)" if $verbosity >= 2;
			if ( 0 != modgen_tables_to_csv($modgen_scenario_mdb, $current_outputs_dir, $significant_digits, $unrounded_tables)) {
				next FLAVOUR;
			}

			#####################################
			# Cleanup                           #
			#####################################
			if ($do_cleanup) {
				chdir ${project_dir};
				remove_tree "${project_dir}/src";
				remove_tree "${project_dir}/build";
				remove_tree "${project_dir}/bin";
			}
		}
		elsif ($flavour eq 'ompp') {
		
			#####################################
			# Build model (ompp)                #
			#####################################

			# Change working directory to project directory for compilation.
			chdir $project_dir || die;

			# The property sheet containing user macros
			my $model_props = "Model.props";
			if ( ! -e $model_props ) {
				logmsg error, $model_dir, $flavour, "Missing property sheet contining user macros: $model_props";
				next FLAVOUR;
			}
			
			# Determine executable model name from MODEL_NAME user macro in Model.props
			my $model_name = get_user_macro($model_props, 'MODEL_NAME');
			if ($model_name eq '') {
				$model_name = $model_dir;
			}
			
			# Determine scenario name from SCENARIO_NAME user macro in Model.props
			my $scenario_name = get_user_macro($model_props, 'SCENARIO_NAME');
			if ($scenario_name eq '') {
				logmsg error, $model_dir, $flavour, "failed to get SCENARIO_NAME from ${model_props}";
				next FLAVOUR;
			}
			
			# Determine fixed parameter folder from FIXED_PARAMETERS_FOLDER user macro in Model.props
			my $fixed_parameters_folder = get_user_macro($model_props, 'FIXED_PARAMETERS_FOLDER');
			# Use files in Fixed folder if not empty, independent of user macro ENABLE_FIXED_PARAMETERS
			# This works around a consequence of propagate_invariant, which copies WizardCaseBased/ompp/Model.props,
			# which does not use parameters/Fixed
			my $enable_fixed_parameters = '0';
			if ( -d $fixed_parameters_folder ) {
				$enable_fixed_parameters = '1';
			}
			
			# Project file
			my $model_vcxproj = 'model.vcxproj';
			
			logmsg info, $model_dir, $flavour, "omc compile, C++ compile, build executable, publish model and scenario" if $verbosity >= 2;
			my $build_log = "msbuild.log";
			unlink $build_log;
			($merged, $retval) = capture_merged {
				my @args = (
					"${msbuild_exe}",
					"${model_vcxproj}",
					"/nologo",
					"/verbosity:normal",
					"/clp:ForceNoAlign",
					"/p:OM_ROOT=${om_root}",
					"/p:OMC_EXE=${omc_exe}",
					"/p:Configuration=${ompp_config}",
					"/p:Platform=${ompp_platform}",
					"/p:GRID_COMPUTING=EMPTY",
					"/p:SCENARIO_NAME=${scenario_name}",
					"/p:ENABLE_FIXED_PARAMETERS=${enable_fixed_parameters}",
					"/p:RUN_SCENARIO=0",
					"/p:EXPORT_CSV=0",
					"/p:EXPORT_EXCEL=0",
					"/p:EXPORT_EXCEL_LAUNCH=0",
					"/t:Rebuild"
					);
				system(@args);
			};
			open BUILD_LOG, ">${build_log}";
			print BUILD_LOG $merged;
			close BUILD_LOG;
			if ($retval != 0) {
				logmsg error, $model_dir, $flavour, "build failed";
				logerrors $merged;
				next FLAVOUR;
			}
			
			# Extract warning count 
			$merged =~ /^ *([0-9]+) Warning\(s\)/m;
			my $warning_count = $1;
			if ($warning_count) {
				logmsg warning, $model_dir, $flavour, "${warning_count} build warnings - see ${build_log}";
			}
			
			# Executable suffix for platform/configuration: nothing, D, 64, 64D
			my $build_suffix = "";
			if ($ompp_platform eq 'x64') {
				$build_suffix .= '64';
			}
			if ($ompp_config eq 'Debug') {
				$build_suffix .= 'D';
			}

			# Executable model generated by build
			my $target_dir = "${project_dir}/bin";
			my $model_exe = "${target_dir}/${model_name}${build_suffix}.exe";
			if ( ! -e $model_exe ) {
				logmsg error, $model_dir, $flavour, "Missing model executable: $model_exe";
				logerrors $merged;
				next FLAVOUR;
			}

			# Database generated by build (align with PUBLISH_DIR in .props
			my $publish_dir = "${project_dir}/../output";
			my $publish_sqlite = "${publish_dir}/${model_name}_${scenario_name}.sqlite";
			if ( ! -f $publish_sqlite ) {
				logmsg error, $model_dir, $flavour, "Missing database: ${publish_sqlite}";
				logerrors $merged;
				next FLAVOUR;
			}

			copy $build_log, $current_logs_dir;
			
			my $elapsed_seconds = time - $start_seconds;
			my $elapsed_formatted = int($elapsed_seconds/60)."m ".($elapsed_seconds%60)."s";
			logmsg info, $model_dir, $flavour, "Build time ${elapsed_formatted}" if $verbosity >= 1;
			$start_seconds = time;

			#####################################
			# Run model (ompp)                  #
			#####################################
			
			# Save copy of generated C++ source code
			chdir $generated_code_dir;
			for (glob "*.h *.cpp") {
				copy "$_", "$current_generated_code_dir";
			}
			
			logmsg info, $model_dir, $flavour, "Run model" if $verbosity >= 2;
			# Change working directory to target_dir where the executable is located.
			chdir "${target_dir}";
			
			my $ompp_trace_txt = "${scenario_name}_trace.txt";
			my $ompp_log_txt = "${scenario_name}_log.txt";
			($merged, $retval) = capture_merged {
				my @args = (
					"${model_exe}",
					"-OpenM.LogToFile", "true",
					"-OpenM.LogFilePath", $ompp_log_txt,
					"-OpenM.TraceToFile", "true",
					"-OpenM.TraceFilePath", $ompp_trace_txt,
					"-OpenM.Database", "Database=${publish_sqlite}; Timeout=86400; OpenMode=ReadWrite;",
					);
				system(@args);
			};
			if ($retval != 0) {
				logmsg error, $model_dir, $flavour, "Run ${scenario_name} scenario failed";
				logerrors $merged;
				next FLAVOUR;
			}
			
			$elapsed_seconds = time - $start_seconds;
			$elapsed_formatted = int($elapsed_seconds/60)."m ".($elapsed_seconds%60)."s";
			logmsg info, $model_dir, $flavour, "Run time ${elapsed_formatted}" if $verbosity >= 1;

			logmsg info, $model_dir, $flavour, "Convert output tables to .csv (${significant_digits} digits of precision)" if $verbosity >= 2;
			if ( 0 != ompp_tables_to_csv($publish_sqlite, $current_outputs_dir, $significant_digits, $unrounded_tables)) {
				next FLAVOUR;
			}
		
			# Copy model run log file to logs directory
			if (-s $ompp_log_txt) {
				copy $ompp_log_txt, $current_logs_dir;
			}
			
			# If present and non-empty copy event trace / case checksum file to results directory
			if (-s $ompp_trace_txt) {
				normalize_event_trace $ompp_trace_txt, "${current_outputs_dir}/trace.txt";
			}
			
			#####################################
			# Cleanup                           #
			#####################################
			if ($do_cleanup) {
				chdir ${project_dir};
				remove_tree "${project_dir}/src";
				remove_tree "${project_dir}/build";
				remove_tree "${project_dir}/bin";
			}
		}
		elsif ($flavour eq 'ompp-linux') {
		
			#####################################
			# Build model (ompp-linux)          #
			#####################################
			
			# Change working directory to model path for make (location of makefile)
			chdir ${model_path};

			# The property sheet containing user macros
			my $model_props = "ompp/Model.props";
			if ( ! -e $model_props ) {
				logmsg error, $model_dir, $flavour, "Missing property sheet contining user macros: $model_props";
				next FLAVOUR;
			}
			
			# Determine executable model name from MODEL_NAME user macro in Model.props
			my $model_name = get_user_macro($model_props, 'MODEL_NAME');
			if ($model_name eq '') {
				$model_name = $model_dir;
			}
			
			# Determine scenario name from SCENARIO_NAME user macro in Model.props
			my $scenario_name = get_user_macro($model_props, 'SCENARIO_NAME');
			if ($scenario_name eq '') {
				logmsg error, $model_dir, $flavour, "failed to get SCENARIO_NAME from ${model_props}";
				next FLAVOUR;
			}

			my $make_defines = '';
			$make_defines .= ' RELEASE=1' if $ompp_linux_configuration eq 'release';
			$make_defines .= ' BIN_POSTFIX=D' if $ompp_linux_configuration eq 'debug';
			
			my $build_log = "${project_dir}/make.log";
			unlink $build_log;
			open BUILD_LOG, ">${build_log}";
			for my $make_target ('cleanall', 'model', 'publish') {
				logmsg info, $model_dir, $flavour, "make ${make_target} ${make_defines}" if $verbosity >= 2;
				($merged, $retval) = capture_merged {
					my @args = (
						"make",
						"${make_target}",
						"${make_defines}",
						);
					system(@args);
				};
				print BUILD_LOG $merged;
				if ($retval != 0) {
					logmsg error, $model_dir, $flavour, "build failed";
					logerrors $merged;
					close BUILD_LOG;
					next FLAVOUR;
				}
			}
			close BUILD_LOG;
			
			# suffix for configuration: nothing or D
			my $build_suffix = "";
			$build_suffix .= 'D' if $ompp_linux_configuration eq 'debug';
			
			# Executable model generated by build
			my $target_dir = "${project_dir}/bin";
			my $model_exe = "${target_dir}/${model_name}${build_suffix}";
			if ( ! -e $model_exe ) {
				logmsg error, $model_dir, $flavour, "Missing model executable: $model_exe";
				logerrors $merged;
				next FLAVOUR;
			}
			copy $build_log, $current_logs_dir;

			# Database generated by build (align with PUBLISH_DIR in makefile, .props
			my $publish_dir = "${project_dir}/../output-linux";
			my $publish_sqlite = "${publish_dir}/${model_name}${build_suffix}_${scenario_name}.sqlite";
			if ( ! -f $publish_sqlite ) {
				logmsg error, $model_dir, $flavour, "Missing database: ${publish_sqlite}";
				logerrors $merged;
				next FLAVOUR;
			}

			my $elapsed_seconds = time - $start_seconds;
			my $elapsed_formatted = int($elapsed_seconds/60)."m ".($elapsed_seconds%60)."s";
			logmsg info, $model_dir, $flavour, "Build time ${elapsed_formatted}" if $verbosity >= 1;
			$start_seconds = time;

			#####################################
			# Run model (ompp-linux)            #
			#####################################
			
			# Save copy of generated C++ source code
			my $linux_generated_code_dir = "${project_dir}/build/${ompp_linux_configuration}/src";
			chdir $linux_generated_code_dir;
			for (glob "*.h *.cpp") {
				copy "$_", "$current_generated_code_dir";
			}

			logmsg info, $model_dir, $flavour, "Run model" if $verbosity >= 2;
			# Change working directory to target_dir where the executable is located.
			chdir "${target_dir}";
			
			my $ompp_trace_txt = "${scenario_name}_trace.txt";
			my $ompp_log_txt = "${scenario_name}_log.txt";
			($merged, $retval) = capture_merged {
				my @args = (
					"${model_exe}",
					"-OpenM.LogToFile", "true",
					"-OpenM.LogFilePath", $ompp_log_txt,
					"-OpenM.TraceToFile", "true",
					"-OpenM.TraceFilePath", $ompp_trace_txt,
					"-OpenM.Database", "Database=${publish_sqlite}; Timeout=86400; OpenMode=ReadWrite;",
					);
				system(@args);
			};
			if ($retval != 0) {
				logmsg error, $model_dir, "ompp", "Run ${scenario_name} scenario failed";
				logerrors $merged;
				next FLAVOUR;
			}
			
			$elapsed_seconds = time - $start_seconds;
			$elapsed_formatted = int($elapsed_seconds/60)."m ".($elapsed_seconds%60)."s";
			logmsg info, $model_dir, $flavour, "Run time ${elapsed_formatted}" if $verbosity >= 1;

			logmsg info, $model_dir, $flavour, "Convert output tables to .csv (${significant_digits} digits of precision)" if $verbosity >= 2;;
			if ( 0 != ompp_tables_to_csv($publish_sqlite, $current_outputs_dir, $significant_digits, 1)) {
				next FLAVOUR;
			}

			# Copy model run log file to logs directory
			if (-s $ompp_log_txt) {
				copy $ompp_log_txt, $current_logs_dir;
			}
			
			# If present and non-empty copy event trace / case checksum file to results directory
			if (-s $ompp_trace_txt) {
				normalize_event_trace $ompp_trace_txt, "${current_outputs_dir}/trace.txt";
			}
			
			#####################################
			# Cleanup                           #
			#####################################
			if ($do_cleanup) {
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
		open SIGNIFICANT_DIGITS_TXT, ">${current_outputs_dir}/significant_digits.txt";
		print SIGNIFICANT_DIGITS_TXT "significant digits = ${significant_digits}\n";
		close SIGNIFICANT_DIGITS_TXT;

		# Create digests of outputs
		logmsg info, $model_dir, $flavour, "Create digests of current outputs" if $verbosity >= 2;
		chdir "${current_outputs_dir}";
		my $digests = create_digest(glob "[a-zA-Z]*.csv *.txt");
		open DIGESTS_TXT, ">digests.txt";
		print DIGESTS_TXT $digests;
		close DIGESTS_TXT;
		
		# If no reference directory, copy current to reference
		if (! -d $reference_dir) {
			logmsg info, $model_dir, $flavour, "No reference outputs - create using current outputs." if $verbosity >= 2;
			dircopy $current_dir, $reference_dir;
		}
		
		# Report on version differences
		my $tombstone_reference_txt = "${reference_dir}/tombstone.txt";
		my $tombstone_current_txt = "${current_dir}/tombstone.txt";
		if (! -e $tombstone_reference_txt || ! -e $tombstone_current_txt) {
			logmsg error, $model_dir, $flavour, "Tombstone file(s) missing";
		}
		else {
			if (compare($tombstone_reference_txt, $tombstone_current_txt) != 0) {
				logmsg info, $model_dir, $flavour, "Current build differs from reference build:" if $verbosity >= 1;
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
		
		# Report on differences between current and reference outputs
		my $digests_txt_1 = "${reference_outputs_dir}/digests.txt";
		my $digests_txt_2 = "${current_outputs_dir}/digests.txt";
		my $different_files = digest_differences($digests_txt_1, $digests_txt_2);
		if ($different_files eq '') {
			logmsg info, $model_dir, $flavour, "Current outputs identical to reference outputs" if $verbosity >= 1;
        }
		else {
			logmsg warning, $model_dir, $flavour, "Current outputs differ: ${different_files}";
		}
	} # flavour

	########
	# compare results between 2 flavours
	########

	COMPARE: {
		my $flavour1 = 'modgen';
		my $flavour2 = 'ompp';
		if ($is_linux) {
			$flavour1 = 'ompp';
			$flavour2 = 'ompp-linux';
		}
		my $which_flavours = $flavour2.' vs. '.$flavour1;
		# Check for differences in outputs between Modgen and ompp
		foreach my $which ('reference', 'current') {
			my $which_proper = ucfirst($which);
			my $digests_txt_1 = "${model_path}/test_models/${which}/${flavour1}/outputs/digests.txt";
			my $digests_txt_2 = "${model_path}/test_models/${which}/${flavour2}/outputs/digests.txt";
			if (! -e $digests_txt_1) {
				logmsg info, $model_dir, $which_flavours, "${which_proper} outputs not compared due to missing ${flavour1} digest" if $verbosity >= 2;
			}
			elsif (! -e $digests_txt_2) {
				logmsg info, $model_dir, $which_flavours, "${which_proper} outputs not compared due to missing ${flavour2} digest" if $verbosity >= 2;
			}
			else {
				my $different_files = digest_differences($digests_txt_1, $digests_txt_2);
				if ($different_files eq '') {
					logmsg info, $model_dir, $which_flavours, "${which_proper} outputs identical";
				}
				else {
					logmsg warning, $model_dir, $which_flavours, "${which_proper} outputs differ: ${different_files}";
				}
			}
		}
	}
	
	logmsg info, " " if $verbosity >= 1;
	
} # model_dir