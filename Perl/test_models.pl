# Copyright (c) 2013-2014 OpenM++
# This code is licensed under MIT license (see LICENSE.txt for details)

# Script to test multiple models in ompp 

use strict;

use File::Copy;
use File::Path qw(make_path remove_tree);
use File::Compare;
use Cwd qw(getcwd);
use Win32::Exe;

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

chdir "../models" || die "Invoke test_models from Perl folder";
my $models_root = getcwd;
my @model_dirs;

if ($#ARGV >= 0) {
	# model folders listed explicitly on command line
	@model_dirs = @ARGV;
}
else {
	# no models listed on command line
	# Create model list by identifying all model subdirectories
	my @paths = glob "*/ompp";
	for my $path (@paths) {
		$path =~ s/\/.*//;
		push @model_dirs, $path;
	}
}

#####################
# Common settings for ompp and modgen
#####################

# Number of significant digits to retain in the output csv files
my $significant_digits = 8;


#####################
# ompp settings for all models
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
# modgen settings for all models
#####################

#my $modgen_configuration = "Debug";
my $modgen_configuration = "Release";

my $modgen_platform = "Win32";
#my $modgen_platform = "x64";


#####################
# There are no configuration settings in subsequent code in this script
#####################


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
# Check for compile errors, and bail if any.
if ($@) {
	print $@;
	exit;
}

	
my $version = "1.0";


# Steps to install Capture::Tiny (this method requires internet connectivity, there are other ways)
# (1) open command prompt
# (2) find and run vsvars32.bat (to get path to nmake into environment)
#     Normally some place like
#     "C:\Program Files (x86)\Microsoft Visual Studio 10.0\Common7\Tools\vsvars32.bat"
# (3) perl -MCPAN -e "install Capture::Tiny"
use Capture::Tiny qw/capture tee capture_merged tee_merged/;

my $modgen_exe = "C:\\Program Files (x86)\\StatCan\\Modgen 11\\Modgen.exe";
if ( ! -e $modgen_exe ) {
	logmsg error, "Missing Modgen compiler: $modgen_exe";
	exit 1;
}
my $modgen_version = Win32::Exe->new($modgen_exe)->version_info->get('FileVersion');

logmsg info, "=========================";
logmsg info, " test_models.pl ${version} ";
logmsg info, "=========================";
logmsg info, " ";
logmsg info, "Testing model folders: ".join(", ", @model_dirs);
logmsg info, " ";
logmsg info, "modgen settings: version=${modgen_version} platform=${modgen_platform} configuration=${modgen_configuration}" if $do_modgen;
logmsg info, "ompp settings: compiler=${omc_exe} platform=${ompp_platform} configuration=${ompp_configuration}" if $do_ompp;


my $modgen_devenv_exe = "C:\\Program Files (x86)\\Microsoft Visual Studio 10.0\\Common7\\IDE\\devenv.exe";
if ( ! -e $modgen_devenv_exe ) {
	logmsg error, "Missing Visual Studio devenv for Modgen: $modgen_devenv_exe";
	exit 1;
}

# MSBuild command line reference:
# http://msdn.microsoft.com/en-us/library/ms164311.aspx
my $msbuild_exe = "C:\\Program Files (x86)\\MSBuild\\12.0\\Bin\\MSBuild.exe";
if ( ! -e $msbuild_exe ) {
	logmsg error, "Missing MSBuild for ompp: $msbuild_exe";
	exit 1;
}

my $create_db_sqlite_sql = "${models_root}/../sql/sqlite/create_db_sqlite.sql";
if ( ! -e $create_db_sqlite_sql ) {
	logmsg error, "Missing SQL file used to create new data store for ompp: $create_db_sqlite_sql";
	exit 1;
}

MODEL:
for my $model_dir (@model_dirs) {
	my $retval; # return value from external commands
	my $merged; # output from external commands
	
	# Specify working directory explicitly because errors can bail back to next model
	# with the working directory in an unknown state.
	chdir $models_root;

	logmsg info, " ";

	my $model_path = "${models_root}/${model_dir}";
	if (! -d $model_path) {
		logmsg error, "${model_path}: Missing model";
		next MODEL;
	}
	
	# Location of model code
	my $code_dir = "${model_path}/code";

	# Location of parameter folder
	my $parameters_dir = "${model_path}/parameters";

	##########
	# Modgen 
	##########

	my $modgen_reference_dir = "${model_path}/test_models/reference/modgen";
	my $modgen_current_dir = "${model_path}/test_models/current/modgen";
	
	MODGEN: {

		if ($do_modgen != 1 ) {
			last MODGEN;
		}

		my $start_seconds = time;
		
		# Project directory for Modgen version
		my $project_dir = "${model_path}/modgen";
		if ( ! -d "${project_dir}" ) {
			logmsg error, $model_dir, "modgen", "Missing folder ${project_dir}";
			last MODGEN;
		}
		
		# Skip modgen processing if place-holder file skip_model found in project folder
		if ( -e "${project_dir}/skip_model" ) {
			logmsg info, $model_dir, "modgen", "Skip model";
			last MODGEN;
		}

		# Delete reference directory if requested by -newref argument
		if ( $new_ref == 1) {
			logmsg info, $model_dir, "modgen", "Deleting reference dir (-newref)";
			remove_tree($modgen_reference_dir);
		}
		
		# Folders for current generated code and outputs
		remove_tree $modgen_current_dir;
		my $current_outputs_dir = "${modgen_current_dir}/outputs";
		make_path $current_outputs_dir;
		my $current_generated_code_dir = "${modgen_current_dir}/generated_code";
		make_path $current_generated_code_dir;
		
		# Folders for reference generated code and outputs
		my $reference_outputs_dir = "${modgen_reference_dir}/outputs";
		my $reference_generated_code_dir = "${modgen_reference_dir}/generated_code";

		# Run Modgen
		# Modgen.exe output files are written to the Modgen/src sub folder using the -D option
		my $modgen_output_dir = "${project_dir}/src";
		remove_tree $modgen_output_dir;
		make_path $modgen_output_dir;
		
		logmsg info, $model_dir, "modgen", "Modgen compile";
		my $modgen_log = "${project_dir}/modgen.log";
		# Change working directory to model source directory for Modgen compilation.
		chdir "${model_dir}/code";
		($merged, $retval) = capture_merged {
			my @args = (
				"${modgen_exe}",
				"-D",
				"${modgen_output_dir}",
				"-EN",
				);
			system(@args);
		};
		open MODGEN_LOG, ">${modgen_log}";
		print MODGEN_LOG $merged;
		close MODGEN_LOG;
		if ($retval != 0) {
			logmsg error, $model_dir, "modgen", "Modgen compile failed - see ModgenLog.htm or modgen.log";
			logerrors $merged;
			last MODGEN;
		}
		
		# Build the model
		logmsg info, $model_dir, "modgen", "C++ compile, build executable";

		#logmsg info, $model_dir, "modgen", "project_dir=${project_dir}";

		# Save copy of generated C++ source code
		chdir "${project_dir}/src";
		for (glob "*.h *.cpp") {
			copy "$_", "$current_generated_code_dir";
		}

		# Change working directory to Modgen solution/project  directory for C++ compilation
		chdir "${project_dir}";

		# The property sheet containing user macros
		my $model_props = "Model.props";
		if ( ! -e $model_props ) {
			logmsg error, $model_dir, "modgen", "Missing property sheet contining user macros: $model_props";
			last MODGEN;
		}
		
		# Determine executable model name from MODEL_NAME user macro in Model.props
		my $model_name = get_user_macro($model_props, 'MODEL_NAME');
		if ($model_name eq '') {
			logmsg error, "failed to get MODEL_NAME from ${model_props}";
			last MODGEN;
		}

		# Log file from build
		my $build_log = "build.log";
		unlink $build_log;

		# Check for solution file
		my $model_sln = "model.sln";
		if ( ! -e $model_sln ) {
			logmsg error, $model_dir, "modgen", "Missing solution file in ${project_dir}";
			last MODGEN;
		}
		
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
			logmsg error, $model_dir, "modgen", "C++ compile failed (${retval}) see ${build_log}";
			logerrors $merged;
			last MODGEN;
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
			logmsg error, $model_dir, "modgen", "Missing model executable: $model_exe";
			logerrors $merged;
			last MODGEN;
		}

		# Base scenario will be run in target_dir (where model executable is located)
		chdir "${target_dir}";
		# Prepare the Modgen version of the Base scenario
		
		# The odat Framework parameter file for the Base scenario.
		# Parameters in that file will be transformed into scenario settings in the Base.scex file
		my $Base_Framework_odat = "${parameters_dir}/Base/Base(Framework).odat";
		if ( ! -e $Base_Framework_odat ) {
			logmsg error, $model_dir, "modgen", "Missing ompp Base Framework parameters: $Base_Framework_odat";
			last MODGEN;
		}

		# The ompp Framework model code file for the model
		# 'use' statements in that file are used to determine scenario settings in the Base.scex file
		my $ompp_framework_ompp = "${code_dir}/ompp_framework.ompp";
		if ( ! -e $ompp_framework_ompp ) {
			logmsg error, $model_dir, "modgen", "Missing ompp framework model code file: $ompp_framework_ompp";
			last MODGEN;
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
			logmsg error, $model_dir, "modgen", "Missing Base scenario file: $modgen_Base_scex";
			last MODGEN;
		}
		
		logmsg info, $model_dir, "modgen", "Run Base scenario";
		# Delete output database to enable subsequent check for success
		my $modgen_Base_mdb = "Base(tbl).mdb";
		unlink $modgen_Base_mdb;

		($merged, $retval) = capture_merged {
			my @args = (
				"${model_exe}",
				"-sc",
				"${modgen_Base_scex}",
				"-s", # silent mode
				);
			system(@args);
		};
		# Modgen models return a code of 1 to indicate success
		# But the code from Perl on a successful run is 256, for some reason.
		# So just shift out the low order bits.
		$retval >>= 8;
		if ($retval != 1) {
			logmsg error, $model_dir, "modgen", "Run model failed - retval=${retval}";
			logerrors $merged;
			last MODGEN;
		}
		if ( ! -e $modgen_Base_mdb ) {
			logmsg error, $model_dir, "modgen", "Missing Base scenario database: ${modgen_Base_mdb}";
			last MODGEN;
		}

		# If present, copy event trace / case checksum file to results directory
		my $modgen_Base_debug_txt = "Base(debug).txt";
		if (-e $modgen_Base_debug_txt) {
			#copy $modgen_Base_debug_txt, "${current_outputs_dir}/trace.txt";
			normalize_event_trace $modgen_Base_debug_txt, "${current_outputs_dir}/trace.txt";
		}

		logmsg info, $model_dir, "modgen", "Convert output tables to .csv (${significant_digits} digits of precision)";
		if ( 0 != modgen_tables_to_csv($modgen_Base_mdb, $current_outputs_dir, $significant_digits)) {
			last MODGEN;
		}
		
		# Create digests of outputs
		logmsg info, $model_dir, "modgen", "Create digests of current outputs";
		chdir "${current_outputs_dir}";
		my $digests = create_digest(glob "*.csv *.txt");
		open DIGESTS_TXT, ">digests.txt";
		print DIGESTS_TXT $digests;
		close DIGESTS_TXT;

		# If no reference results, copy current results as a starting point
		if (! -d $reference_outputs_dir) {
			logmsg info, $model_dir, "modgen", "No reference outputs - create using current outputs.";
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
		
		# Check for differences between current and reference outputs
		my $reference_digests = "${reference_outputs_dir}/digests.txt";
		my $current_digests = "${current_outputs_dir}/digests.txt";
        if (compare($current_digests, $reference_digests) == 0) {
			logmsg info, $model_dir, "modgen", "Current outputs identical to reference outputs";
        }
		else {
			my $different_files = digest_differences($reference_digests, $current_digests);
			logmsg warning, $model_dir, "modgen", "Current outputs differ: ${different_files}";
		}
		
		my $elapsed_seconds = time - $start_seconds;
		my $elapsed_formatted = int($elapsed_seconds/60)."m ".($elapsed_seconds%60)."s";
		logmsg info, $model_dir, "modgen", "Elapsed time = ${elapsed_formatted}";

	}

	########
	# ompp 
	########

	my $ompp_reference_dir = "${model_path}/test_models/reference/ompp";
	my $ompp_current_dir = "${model_path}/test_models/current/ompp";

	OMPP: {
		if ($do_ompp != 1) {
			last OMPP;
		}

		my $start_seconds = time;
		
		# Project directory for ompp version
		my $project_dir = "${model_path}/ompp";
		if ( ! -d "${project_dir}" ) {
			logmsg error, $model_dir, "ompp", "Missing folder ${project_dir}";
			last OMPP;
		}

		# Skip ompp processing if placeholder file skip_model found in project folder
		if ( -e "${project_dir}/skip_model" ) {
			logmsg info, $model_dir, "ompp", "Skip model";
			last OMPP;
		}

		# Delete reference directory if requested by -newref argument
		if ( $new_ref == 1) {
			logmsg info, $model_dir, "ompp", "Deleting reference dir (-newref)";
			remove_tree($ompp_reference_dir);
		}
		
		# Folders for current generated code and outputs
		remove_tree $ompp_current_dir;
		my $current_outputs_dir = "${ompp_current_dir}/outputs";
		make_path $current_outputs_dir;
		my $current_generated_code_dir = "${ompp_current_dir}/generated_code";
		make_path $current_generated_code_dir;
		
		# Folders for reference generated code and outputs
		my $reference_outputs_dir = "${ompp_reference_dir}/outputs";
		my $reference_generated_code_dir = "${ompp_reference_dir}/generated_code";

		# Change working directory to project directory for compilation.
		chdir ${project_dir};

		# The property sheet containing user macros
		my $model_props = "Model.props";
		if ( ! -e $model_props ) {
			logmsg error, $model_dir, "ompp", "Missing property sheet contining user macros: $model_props";
			last OMPP;
		}
		
		# Determine executable model name from MODEL_NAME user macro in Model.props
		my $model_name = get_user_macro($model_props, 'MODEL_NAME');
		if ($model_name eq '') {
			logmsg error, "failed to get MODEL_NAME from ${model_props}";
			last OMPP;
		}
		
		# Determine scenario name from SCENARIO_NAME user macro in Model.props
		my $scenario_name = get_user_macro($model_props, 'SCENARIO_NAME');
		if ($scenario_name eq '') {
			logmsg error, "failed to get SCENARIO_NAME from ${model_props}";
			last OMPP;
		}
		
		# The solution file
		my $model_sln = "Model.sln";
		if ( ! -e $model_sln ) {
			logmsg error, $model_dir, "ompp", "Missing solution file: $model_sln";
			last OMPP;
		}

		logmsg info, $model_dir, "ompp", "omc compile, C++ compile, build executable, publish model and Base scenario";
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
			logmsg error, $model_dir, "ompp", "build failed";
			logerrors $merged;
			last OMPP;
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
			logmsg error, $model_dir, "ompp", "Missing model executable: $model_exe";
			logerrors $merged;
			last OMPP;
		}

		# Data store generated by build
		my $model_sqlite = "${target_dir}/${model_name}.sqlite";
		if ( ! -e $model_sqlite ) {
			logmsg error, $model_dir, "ompp", "Missing data store: $model_sqlite";
			last OMPP;
		}

		# Save copy of generated C++ source code
		for (glob "src/*.h src/*.cpp") {
			copy "$_", "$current_generated_code_dir";
		}

		logmsg info, $model_dir, "ompp", "Run Base scenario";
		
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
			last OMPP;
		}

		# If present and non-empty copy event trace / case checksum file to results directory
		if (-s $ompp_trace_txt) {
			#copy $ompp_trace_txt, "${current_outputs_dir}/trace.txt";
			normalize_event_trace $ompp_trace_txt, "${current_outputs_dir}/trace.txt";
		}

		logmsg info, $model_dir, "ompp", "Convert output tables to .csv (${significant_digits} digits of precision)";
		if ( 0 != ompp_tables_to_csv($model_sqlite, $current_outputs_dir, $significant_digits)) {
			last OMPP;
		}
		
		# Create digests of outputs
		logmsg info, $model_dir, "ompp", "Create digests of current outputs";
		chdir "${current_outputs_dir}";
		my $digests = create_digest(glob "*.csv *.txt");
		open DIGESTS_TXT, ">digests.txt";
		print DIGESTS_TXT $digests;
		close DIGESTS_TXT;

		# If no reference results, copy current results as a starting point
		if (! -d $reference_outputs_dir) {
			logmsg info, $model_dir, "ompp", "No reference outputs - create using current outputs.";
			make_path $reference_outputs_dir;
			chdir $current_outputs_dir;
			for (glob "*.*") {
				copy "$_", "$reference_outputs_dir";
			}
			# Also record generated code
			make_path $reference_generated_code_dir;
			chdir $current_generated_code_dir;
			for (glob "*.*") {
				copy "$_", "$reference_generated_code_dir";
			}
		}
		
		# Check for differences between current and reference outputs
		my $reference_digests = "${reference_outputs_dir}/digests.txt";
		my $current_digests = "${current_outputs_dir}/digests.txt";
        if (compare("${current_outputs_dir}/digests.txt","${reference_outputs_dir}/digests.txt") == 0) {
			logmsg info, $model_dir, "ompp", "Current outputs identical to reference outputs";
        }
		else {
			my $different_files = digest_differences($reference_digests, $current_digests);
			logmsg warning, $model_dir, "ompp", "Current outputs differ: ${different_files}";
		}
		
		my $elapsed_seconds = time - $start_seconds;
		my $elapsed_formatted = int($elapsed_seconds/60)."m ".($elapsed_seconds%60)."s";
		logmsg info, $model_dir, "ompp", "Elapsed time = ${elapsed_formatted}";
	}
	
	########
	# compare modgen and ompp results
	########

	COMPARE: {
		# Check for differences in outputs between Modgen and ompp
		foreach my $which ('reference', 'current') {
			my $which_proper = ucfirst($which);
			my $modgen_digests_txt = "${model_path}/test_models/${which}/modgen/outputs/digests.txt";
			my $ompp_digests_txt = "${model_path}/test_models/${which}/ompp/outputs/digests.txt";
			if (! -e $modgen_digests_txt) {
				logmsg info, $model_dir, "modgen-ompp", "${which_proper} outputs not compared due to missing modgen digest";
			}
			elsif (! -e $ompp_digests_txt) {
				logmsg info, $model_dir, "modgen-ompp", "${which_proper} outputs not compared due to missing ompp digest";
			}
			else {
				if (compare($modgen_digests_txt, $ompp_digests_txt) == 0) {
					logmsg info, $model_dir, "modgen-ompp", "${which_proper} outputs identical";
				}
				else {
					my $different_files = digest_differences($modgen_digests_txt, $ompp_digests_txt);
					logmsg warning, $model_dir, "modgen-ompp", "${which_proper} outputs differ: ${different_files}";
				}
			}
		}
	}
	
}