# Copyright (c) 2013-2015 OpenM++
# This code is licensed under MIT license (see LICENSE.txt for details)

# Script to test multiple models in ompp, ompp-lilnux, and modgen

# usage: perl test_models.pl [-newref] [-nomodgen] [-noompp] models
# If models is not specified, all models will be processed.

chdir "../models" || die "Invoke test_models from Perl folder";
use Cwd qw(getcwd);
my $models_root = getcwd;


#####################
# Common settings
#####################

# Verbosity of report:
#  0 - errors only
#  1 - timing and success
#  2 - chatty
my $verbosity = 1;

# Number of significant digits to retain in the output csv files
my $significant_digits = 8;

#####################
# ompp settings
#####################

# ompp compiler version (4 possible values)
my $omc_exe = 'omc.exe';
#my $omc_exe = 'omcD.exe';
#my $omc_exe = 'omc64.exe';
#my $omc_exe = 'omc64D.exe';
	
#my $ompp_configuration = "Debug";
my $ompp_configuration = "Release";

#my $ompp_platform = "Win32";
my $ompp_platform = "x64";

#####################
# ompp-linux settings
#####################

#my $ompp_linux_configuration = "debug";
my $ompp_linux_configuration = "release";

#####################
# modgen settings
#####################

#my $modgen_configuration = "Debug";
my $modgen_configuration = "Release";

my $modgen_platform = "Win32";
#my $modgen_platform = "x64";

#####################
# file locations
#####################

my $modgen_exe = "C:\\Program Files (x86)\\StatCan\\Modgen 11\\Modgen.exe";
my $modgen_devenv_exe = "C:\\Program Files (x86)\\Microsoft Visual Studio 10.0\\Common7\\IDE\\devenv.exe";
# MSBuild command line reference:
# http://msdn.microsoft.com/en-us/library/ms164311.aspx
my $msbuild_exe = "C:\\Program Files (x86)\\MSBuild\\12.0\\Bin\\MSBuild.exe";
my $create_db_sqlite_sql = "${models_root}/../sql/sqlite/create_db_sqlite.sql";


#####################
# There are no configuration settings in subsequent code in this script
#####################

use strict;

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
use File::Path qw(make_path remove_tree);
use File::Compare;

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
	require "../perl/common_windows.pm";
}

###############
# Determine flavours and options
###############

my $new_ref = 0;
my $do_modgen = 1;
my $do_ompp = 1;

# Check for and process -newref flag (replace reference by current)
if ($#ARGV >= 0) {
	if ( $ARGV[0] eq "-newref") {
		shift @ARGV;
		$new_ref = 1;
	}
}

# Check for and process -nomodgen flag
if ($#ARGV >= 0) {
	if ( $ARGV[0] eq "-nomodgen") {
		shift @ARGV;
		$do_modgen = 0;
	}
}

# Check for and process -noompp flag
if ($#ARGV >= 0) {
	if ( $ARGV[0] eq "-noompp") {
		shift @ARGV;
		$do_ompp = 0;
	}
}

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
		if ( ! -e $modgen_exe ) {
			logmsg error, "Missing Modgen compiler: $modgen_exe";
			exit 1;
		}
		if ( ! -e $modgen_devenv_exe ) {
			logmsg error, "Missing Visual Studio devenv for Modgen: $modgen_devenv_exe";
			exit 1;
		}
		push @flavours, 'modgen';
		my $modgen_version = modgen_version($modgen_exe);
		push @flavours_tombstone, "version=${modgen_version} platform=${modgen_platform} configuration=${modgen_configuration}";
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
		push @flavours_tombstone, "compiler=${omc_exe} platform=${ompp_platform} configuration=${ompp_configuration}";
	}
}
else { # linux
	push @flavours, 'ompp-linux';
	push @flavours_tombstone, "configuration=${ompp_linux_configuration}";
}
	
my $version = "1.1";

logmsg info, "=========================";
logmsg info, " test_models.pl ${version} ";
logmsg info, "=========================";
logmsg info, " ";
logmsg info, "Testing model folders: ".join(", ", @model_dirs);

for (my $j = 0; $j <= $#flavours; $j++) {
	logmsg info, "@flavours[$j] settings: @flavours_tombstone[$j]";
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
		my $flavour = @flavours[$j];
		my $tombstone = @flavours_tombstone[$j];
		my $reference_dir = "${model_path}/test_models/reference/${flavour}";
		my $current_dir = "${model_path}/test_models/current/${flavour}";
		my $project_dir = "${model_path}/${flavour}";
		make_path $project_dir if -d "${project_dir}";
		# Skip this flavour if place-holder file skip_model found in project folder for this flavour
		next FLAVOUR if -e "${project_dir}/skip_model";
		logmsg info, $model_dir, $flavour if $verbosity == 0;
		if ($new_ref) {
			logmsg info, $model_dir, $flavour, "Deleting old reference directory" if $verbosity >= 2;
			remove_tree($reference_dir);
		}
		remove_tree $current_dir;
		# Folder for current model outputs
		my $current_outputs_dir = "${current_dir}/outputs";
		make_path $current_outputs_dir;

		# Folder for current generated code (holds a copy)
		my $current_generated_code_dir = "${current_dir}/generated_code";
		make_path $current_generated_code_dir;
		
		# Folders for reference generated code and outputs
		my $reference_outputs_dir = "${reference_dir}/outputs";
		my $reference_generated_code_dir = "${reference_dir}/generated_code";

		my $generated_code_dir = "${project_dir}/src";
		remove_tree $generated_code_dir;
		make_path $generated_code_dir;

		if ($flavour eq 'modgen') {

			#####################################
			# Build model (modgen)              #
			#####################################

			logmsg info, $model_dir, $flavour, "Modgen compile" if $verbosity >= 2;
			# Change current directory to model source code directory for Modgen compilation,
			# because Modgen only compiles code in current directory.
			chdir "${model_source_dir}";
			($merged, $retval) = capture_merged {
				my @args = (
					"${modgen_exe}",
					"-D",
					"${generated_code_dir}",
					"-EN",
					);
				system(@args);
			};
			my $modgen_log = "${project_dir}/modgen.log";
			open MODGEN_LOG, ">${modgen_log}";
			print MODGEN_LOG $merged;
			close MODGEN_LOG;
			if ($retval != 0) {
				logmsg error, $model_dir, $flavour, "Modgen compile failed - see ModgenLog.htm or modgen.log";
				logerrors $merged;
				next FLAVOUR;
			}
			# Save copy of generated C++ source code
			chdir $generated_code_dir;
			for (glob "*.h *.cpp") {
				copy "$_", "$current_generated_code_dir";
			}
			
			# Build the model
			logmsg info, $model_dir, $flavour, "C++ compile & build" if $verbosity >= 2;

			# Change working directory to where vxproj, etc. are located
			chdir "${project_dir}";

			# The property sheet containing user macros
			my $model_props = "Model.props";
			if ( ! -e $model_props ) {
				logmsg error, $model_dir, $flavour, "Missing property sheet with user macros: $model_props";
				next FLAVOUR;
			}
			
			# Determine executable model name from MODEL_NAME user macro in Model.props
			my $model_name = get_user_macro($model_props, 'MODEL_NAME');
			if ($model_name eq '') {
				logmsg error, $model_dir, $flavour, "Failed to get MODEL_NAME from ${model_props}";
				next FLAVOUR;
			}
			
			# Check for solution file in parent folder
			my $model_sln = "../ModgenModel.sln";
			if ( ! -e $model_sln ) {
				logmsg error, $model_dir, $flavour, "Missing solution file ${model_sln}";
				next FLAVOUR;
			}
			
			# Log file from devenv build
			my $build_log = "build.log";
			unlink $build_log;
			($merged, $retval) = capture_merged {
				my @args = (
					"${modgen_devenv_exe}",
					"${model_sln}",
					"/Rebuild", "${modgen_configuration}|${modgen_platform}",
					"/Out", "${build_log}",
					);
				system(@args);
			};
			if ($retval != 0) {
				logmsg error, $model_dir, $flavour, "C++ compile failed (${retval}) see ${build_log}";
				logerrors $merged;
				next FLAVOUR;
			}

			# Executable suffix for platform/configuration: nothing, D, 64, 64D
			my $build_suffix = "";
			if ($modgen_platform eq 'x64') {
				$build_suffix .= '64';
			}
			if ($modgen_configuration eq 'Debug') {
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
			
			#####################################
			# Create Base scenario file (modgen)#
			#####################################
			
			logmsg info, $model_dir, $flavour, "Create Base scenario file" if $verbosity >= 2;
			
			# Base scenario will be run in target_dir (where model executable is located)
			chdir "${target_dir}";
			
			# The odat Framework parameter file for the Base scenario.
			# Parameters in that file will be transformed into scenario settings in the Base.scex file
			my $Base_Framework_odat = "${parameters_dir}/Base/Base(Framework).odat";
			if ( ! -e $Base_Framework_odat ) {
				logmsg error, $model_dir, $flavour, "Missing ompp Base Framework parameters: $Base_Framework_odat";
				next FLAVOUR;
			}

			# The ompp Framework model code file for the model
			# 'use' statements in that file are used to determine scenario settings in the Base.scex file
			my $ompp_framework_ompp = "${model_source_dir}/ompp_framework.ompp";
			if ( ! -e $ompp_framework_ompp ) {
				logmsg error, $model_dir, $flavour, "Missing ompp framework model code file: $ompp_framework_ompp";
				next FLAVOUR;
			}

			# The name of the Modgen Base scenario file
			# (will be created by this script, in the same directory as the model executable)
			my $modgen_Base_scex = "Base.scex";
			unlink $modgen_Base_scex;

			# List of all the .dat files in the Base scenario
			# with relative paths (so .scex file works independnent of ompp installation folder.
			my @modgen_Base_dats;
			push @modgen_Base_dats, glob("../../parameters/Base/*.dat");
			push @modgen_Base_dats, glob("../../parameters/Fixed/*.dat");

			# Create the Modgen Base scenario file
			modgen_create_scex($modgen_Base_scex, $Base_Framework_odat, $ompp_framework_ompp, @modgen_Base_dats);
			if ( ! -e $modgen_Base_scex ) {
				logmsg error, $model_dir, $flavour, "Missing Base scenario file: $modgen_Base_scex";
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
			for (glob "src/*.h src/*.cpp") {
				copy "$_", "$current_generated_code_dir";
			}
			
			logmsg info, $model_dir, $flavour, "Run model" if $verbosity >= 2;

			# Delete output database to enable subsequent check for success
			my $modgen_Base_mdb = "Base(tbl).mdb";
			unlink $modgen_Base_mdb;

			($merged, $retval) = capture_merged {
				my @args = (
					"${model_exe}",
					"-sc",
					"${modgen_Base_scex}",
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
			if ( ! -e $modgen_Base_mdb ) {
				logmsg error, $model_dir, $flavour, "Missing Base scenario database: ${modgen_Base_mdb}";
				next FLAVOUR;
			}

			my $elapsed_seconds = time - $start_seconds;
			my $elapsed_formatted = int($elapsed_seconds/60)."m ".($elapsed_seconds%60)."s";
			logmsg info, $model_dir, $flavour, "Run time ${elapsed_formatted}" if $verbosity >= 1;

			# If present, copy event trace / case checksum file to results directory
			my $modgen_Base_debug_txt = "Base(debug).txt";
			if (-e $modgen_Base_debug_txt) {
				normalize_event_trace $modgen_Base_debug_txt, "${current_outputs_dir}/trace.txt";
			}
			
			logmsg info, $model_dir, $flavour, "Convert output tables to .csv (${significant_digits} digits of precision)" if $verbosity >= 2;
			if ( 0 != modgen_tables_to_csv($modgen_Base_mdb, $current_outputs_dir, $significant_digits)) {
				next FLAVOUR;
			}
			
		}
		elsif ($flavour eq 'ompp') {
		
			#####################################
			# Build model (ompp)                #
			#####################################

			# Change working directory to project directory for compilation.
			chdir ${project_dir};

			# The property sheet containing user macros
			my $model_props = "Model.props";
			if ( ! -e $model_props ) {
				logmsg error, $model_dir, $flavour, "Missing property sheet contining user macros: $model_props";
				next FLAVOUR;
			}
			
			# Determine executable model name from MODEL_NAME user macro in Model.props
			my $model_name = get_user_macro($model_props, 'MODEL_NAME');
			if ($model_name eq '') {
				logmsg error, $model_dir, $flavour, "failed to get MODEL_NAME from ${model_props}";
				next FLAVOUR;
			}
			
			# Determine scenario name from SCENARIO_NAME user macro in Model.props
			my $scenario_name = get_user_macro($model_props, 'SCENARIO_NAME');
			if ($scenario_name eq '') {
				logmsg error, $model_dir, $flavour, "failed to get SCENARIO_NAME from ${model_props}";
				next FLAVOUR;
			}
			
			# The solution file
			my $model_sln = "../OmppModel.sln";
			if ( ! -e $model_sln ) {
				logmsg error, $model_dir, $flavour, "Missing solution file: ${model_sln}";
				next FLAVOUR;
			}

			logmsg info, $model_dir, $flavour, "omc compile, C++ compile, build executable, publish model and Base scenario" if $verbosity >= 2;
			($merged, $retval) = capture_merged {
				my @args = (
					"${msbuild_exe}",
					"${model_sln}",
					"/nologo",
					"/verbosity:normal",
					"/clp:ForceNoAlign",
					"/p:OMC_EXE=${omc_exe}",
					"/p:Configuration=${ompp_configuration}",
					"/p:Platform=${ompp_platform}",
					"/p:ENABLE_SCENARIO_PARAMETERS=1",
					"/p:ENABLE_FIXED_PARAMETERS=1",
					"/p:GRID_COMPUTING=EMPTY",
					"/p:RUN_SCENARIO=0",
					"/p:RUN_EXPORT=0",
					"/p:RUN_EXCEL=0",
					"/t:Rebuild"
					);
				system(@args);
			};
			if ($retval != 0) {
				logmsg error, $model_dir, $flavour, "build failed";
				logerrors $merged;
				next FLAVOUR;
			}
			
			# Executable suffix for platform/configuration: nothing, D, 64, 64D
			my $build_suffix = "";
			if ($ompp_platform eq 'x64') {
				$build_suffix .= '64';
			}
			if ($ompp_configuration eq 'Debug') {
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

			# Data store generated by build
			my $model_sqlite = "${target_dir}/${model_name}.sqlite";
			if ( ! -e $model_sqlite ) {
				logmsg error, $model_dir, $flavour, "Missing data store: $model_sqlite";
				next FLAVOUR;
			}

			my $elapsed_seconds = time - $start_seconds;
			my $elapsed_formatted = int($elapsed_seconds/60)."m ".($elapsed_seconds%60)."s";
			logmsg info, $model_dir, $flavour, "Build time ${elapsed_formatted}" if $verbosity >= 1;
			$start_seconds = time;

			#####################################
			# Run model (ompp)                  #
			#####################################
			
			# Save copy of generated C++ source code
			for (glob "src/*.h src/*.cpp") {
				copy "$_", "$current_generated_code_dir";
			}
			
			# Change working directory to target_dir where the executable and data store
			# are located. This avoids having to specify the data connection string
			# explicitly when launching the model.
			chdir "${target_dir}";
			
			my $ompp_trace_txt = "Base_trace.txt";
			my $ompp_log_txt = "Base_log.txt";
			($merged, $retval) = capture_merged {
				my @args = (
					"${model_exe}",
					"-OpenM.LogToFile", "true",
					"-OpenM.LogFilePath", $ompp_log_txt,
					"-OpenM.TraceToFile", "true",
					"-OpenM.TraceFilePath", $ompp_trace_txt,
					);
				system(@args);
			};
			if ($retval != 0) {
				logmsg error, $model_dir, $flavour, "Run Base scenario failed";
				logerrors $merged;
				next FLAVOUR;
			}

			logmsg info, $model_dir, $flavour, "Convert output tables to .csv (${significant_digits} digits of precision)" if $verbosity >= 2;
			if ( 0 != ompp_tables_to_csv($model_sqlite, $current_outputs_dir, $significant_digits)) {
				next FLAVOUR;
			}
			
			# If present and non-empty copy event trace / case checksum file to results directory
			if (-s $ompp_trace_txt) {
				normalize_event_trace $ompp_trace_txt, "${current_outputs_dir}/trace.txt";
			}
			
			my $elapsed_seconds = time - $start_seconds;
			my $elapsed_formatted = int($elapsed_seconds/60)."m ".($elapsed_seconds%60)."s";
			logmsg info, $model_dir, $flavour, "Run time ${elapsed_formatted}" if $verbosity >= 1;
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
				logmsg error, $model_dir, $flavour, "failed to get MODEL_NAME from ${model_props}";
				next FLAVOUR;
			}
			
			# Determine scenario name from SCENARIO_NAME user macro in Model.props
			my $scenario_name = get_user_macro($model_props, 'SCENARIO_NAME');
			if ($scenario_name eq '') {
				logmsg error, $model_dir, $flavour, "failed to get SCENARIO_NAME from ${model_props}";
				next FLAVOUR;
			}

			my $make_defines;
			$make_defines .= ' RELEASE=1' if $ompp_linux_configuration eq 'release';
			
			for my $make_target ('cleanall', 'model', 'publish') {
				logmsg info, $model_dir, $flavour, "make ${make_target}" if $verbosity >= 2;
				($merged, $retval) = capture_merged {
					my @args = (
						"make",
						"${make_target}",
						"${make_defines}",
						);
					system(@args);
				};
				if ($retval != 0) {
					logmsg error, $model_dir, $flavour, "build failed";
					logerrors $merged;
					next FLAVOUR;
				}
			}
			
			# Executable model generated by build
			my $target_dir = "${project_dir}/bin/${ompp_linux_configuration}";
			my $model_exe = "${target_dir}/${model_name}";
			if ( ! -e $model_exe ) {
				logmsg error, $model_dir, $flavour, "Missing model executable: $model_exe";
				logerrors $merged;
				next FLAVOUR;
			}

			# Data store generated by build
			my $model_sqlite = "${target_dir}/${model_name}.sqlite";
			if ( ! -e $model_sqlite ) {
				logmsg error, $model_dir, $flavour, "Missing data store: $model_sqlite";
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
			for (glob "src/*.h src/*.cpp") {
				copy "$_", "$current_generated_code_dir";
			}

			# Change working directory to target_dir where the executable and data store
			# are located. This avoids having to specify the data connection string
			# explicitly when launching the model.
			chdir "${target_dir}";
			
			my $ompp_trace_txt = "Base_trace.txt";
			my $ompp_log_txt = "Base_log.txt";
			($merged, $retval) = capture_merged {
				my @args = (
					"${model_exe}",
					"-OpenM.LogToFile", "true",
					"-OpenM.LogFilePath", $ompp_log_txt,
					"-OpenM.TraceToFile", "true",
					"-OpenM.TraceFilePath", $ompp_trace_txt,
					);
				system(@args);
			};
			if ($retval != 0) {
				logmsg error, $model_dir, "ompp", "Run Base scenario failed";
				logerrors $merged;
				next FLAVOUR;
			}

			logmsg info, $model_dir, $flavour, "Convert output tables to .csv (${significant_digits} digits of precision)" if $verbosity >= 2;;
			if ( 0 != ompp_tables_to_csv($model_sqlite, $current_outputs_dir, $significant_digits)) {
				next FLAVOUR;
			}

			# If present and non-empty copy event trace / case checksum file to results directory
			if (-s $ompp_trace_txt) {
				normalize_event_trace $ompp_trace_txt, "${current_outputs_dir}/trace.txt";
			}
			
			my $elapsed_seconds = time - $start_seconds;
			my $elapsed_formatted = int($elapsed_seconds/60)."m ".($elapsed_seconds%60)."s";
			logmsg info, $model_dir, $flavour, "Run time ${elapsed_formatted}" if $verbosity >= 1;
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
		my $digests = create_digest(glob "*.csv *.txt");
		open DIGESTS_TXT, ">digests.txt";
		print DIGESTS_TXT $digests;
		close DIGESTS_TXT;

		# If no reference results, copy current results to reference as a base for subsequent comparisons
		if (! -d $reference_outputs_dir) {
			logmsg info, $model_dir, $flavour, "No reference outputs - create using current outputs." if $verbosity >= 2;
			make_path $reference_outputs_dir;
			chdir $current_outputs_dir;
			for (glob "*.*") {
				copy "$_", "$reference_outputs_dir";
			}
			# Also record generated code
			unlink $reference_generated_code_dir;
			make_path $reference_generated_code_dir;
			chdir $current_generated_code_dir;
			for (glob "*.*") {
				copy "$_", "$reference_generated_code_dir";
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
				logmsg info, $model_dir, $which_flavours, "${which_proper} outputs not compared due to missing ${flavour1} digest";
			}
			elsif (! -e $digests_txt_2) {
				logmsg info, $model_dir, $which_flavours, "${which_proper} outputs not compared due to missing ${flavour2} digest";
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