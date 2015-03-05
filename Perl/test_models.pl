# Copyright (c) 2013-2014 OpenM++
# This code is licensed under MIT license (see LICENSE.txt for details)

# Script to test multiple models in ompp 

use strict;

use File::Copy;
use File::Path qw(make_path remove_tree);
use File::Compare;
use Cwd qw(getcwd);
use Win32::Exe;

my $do_modgen = 1;
my $do_ompp = 1;

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
my @models;

if ($#ARGV >= 0) {
	# models listed explicitly on command line
	@models = @ARGV;
}
else {
	# no models listed on command line
	# Create model list by identifying all model subdirectories
	my @paths = glob "*/ompp";
	for my $path (@paths) {
		$path =~ s/\/.*//;
		push @models, $path;
	}
}

# ompp compiler version (4 possible values)
#my $omc_exe = 'omc.exe';
#my $omc_exe = 'omcD.exe';
my $omc_exe = 'omc64.exe';
#my $omc_exe = 'omc64D.exe';
	
my $ompp_configuration = "Debug";
#my $ompp_configuration = "Release";

my $ompp_platform = "Win32";
#my $ompp_platform = "x64";

# Number of significant digits to retain in the output csv files
my $significant_digits = 8;

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

logmsg info, "=========================";
logmsg info, " test_models.pl ${version} ";
logmsg info, "=========================";
logmsg info, " ";
logmsg info, "Testing models: ".join(", ", @models);
logmsg info, "ompp settings: compiler=${omc_exe} platform=${ompp_platform} configuration=${ompp_configuration}";

my $modgen_exe = "C:\\Program Files (x86)\\StatCan\\Modgen 11\\Modgen.exe";
if ( ! -e $modgen_exe ) {
	logmsg error, "Missing Modgen compiler: $modgen_exe";
	exit 1;
}
my $modgen_version = Win32::Exe->new($modgen_exe)->version_info->get('FileVersion');
logmsg info, "Modgen version = ${modgen_version}";


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
for my $model (@models) {
	my $retval; # return value from external commands
	my $merged; # output from external commands
	
	# Specify working directory explicitly because errors can bail back to next model
	# with the working directory in an unknown state.
	chdir $models_root;

	logmsg info, " ";

	my $model_dir = "${models_root}/${model}";
	if (! -d $model_dir) {
		logmsg error, "${model}: Missing model";
		next MODEL;
	}

	my $modgen_reference_dir = "${model_dir}/test_models/reference/modgen";
	my $modgen_current_dir = "${model_dir}/test_models/current/modgen";
	my $ompp_reference_dir = "${model_dir}/test_models/reference/ompp";
	my $ompp_current_dir = "${model_dir}/test_models/current/ompp";

	##########
	# Modgen 
	##########
	
	MODGEN: {

		if ($do_modgen != 1 ) {
			last MODGEN;
		}

		my $start_seconds = time;
		
		# Project directory for Modgen version
		my $project_dir = "${model_dir}/modgen";
		if ( ! -d "${project_dir}" ) {
			logmsg error, $model, "modgen", "Missing folder ${project_dir}";
			last MODGEN;
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
		# Modgen.exe output files are written to the Modgen sub folder using the -D option
		logmsg info, $model, "modgen", "Modgen compile";
		my $modgen_log = "${project_dir}/modgen.log";
		# Change working directory to model source directory for Modgen compilation.
		# (This is actually the parent directory.)
		chdir "${model_dir}/code";
		($merged, $retval) = capture_merged {
			my @args = (
				"${modgen_exe}",
				"-D",
				"${project_dir}",
				"-EN",
				);
			system(@args);
		};
		open MODGEN_LOG, ">${modgen_log}";
		print MODGEN_LOG $merged;
		close MODGEN_LOG;
		if ($retval != 0) {
			logmsg error, $model, "modgen", "Modgen compile failed - see ModgenLog.htm or modgen.log";
			logerrors $merged;
			last MODGEN;
		}
		
		# Build the model
		logmsg info, $model, "modgen", "C++ compile, build executable";

		# Change working directory to Modgen project directory for C++ compilation
		# running the model, and processing outputs.
		chdir "${project_dir}";
		#logmsg info, $model, "modgen", "project_dir=${project_dir}";

		# Save copy of generated C++ source code
		for (glob "*.h *.cpp") {
			copy "$_", "$current_generated_code_dir";
		}

		# Log file from build
		my $build_log = "build.log";
		unlink $build_log;

		# Check for solution file
		my $model_sln = "${model}.sln";
		if ( ! -e $model_sln ) {
			logmsg error, $model, "modgen", "Missing solution file in ${project_dir}";
			last MODGEN;
		}
		
		($merged, $retval) = capture_merged {
			my @args = (
				"${modgen_devenv_exe}",
				"${model_sln}",
				"/Rebuild", "Debug",
				"/Out", "${build_log}",
				);
			system(@args);
		};
		if ($retval != 0) {
			logmsg error, $model, "modgen", "C++ compile failed - see ${build_log}";
			logerrors $merged;
			last MODGEN;
		}

		# Directory location and executable name may differ among versions of Modgen new model wizard
		my $modgen_model_exe = glob "*.exe Debug/*.exe"; # Glob not reliable immediately after file creation
		if ( -e "${model}.exe" ) {
			$modgen_model_exe = "${model}.exe";
		}
		elsif  ( -e "Debug/${model}.exe" ) {
			$modgen_model_exe = "Debug/${model}.exe";
		}
		else {
			logmsg error, $model, "modgen", "Missing model executable";
			last MODGEN;
		}

		# Prepare the Modgen version of the Base scenario
		
		# The name of the Modgen Base scenario file
		# (will be created by this script)
		my $modgen_Base_scex = "Base.scex";
		unlink $modgen_Base_scex;

		# The odat Framework parameter file for the Base scenario.
		# Parameters in that file will be transformed into scenario settings in the Base.scex file
		my $Base_Framework_odat = "../parameters/Base/Base(Framework).odat";
		if ( ! -e $Base_Framework_odat ) {
			logmsg error, $model, "modgen", "Missing ompp Base Framework parameters: $Base_Framework_odat";
			last MODGEN;
		}

		# The ompp Framework model code file for the model
		# 'use' statements in that file are used to determine scenario settings in the Base.scex file
		my $ompp_framework_ompp = "../code/ompp_framework.ompp";
		if ( ! -e $ompp_framework_ompp ) {
			logmsg error, $model, "modgen", "Missing ompp framework model code file: $ompp_framework_ompp";
			last MODGEN;
		}

		# List of all the .dat files in the Base scenario
		# with relative paths.
		my @modgen_Base_dats;
		push @modgen_Base_dats, glob("../parameters/Base/*.dat");
		push @modgen_Base_dats, glob("../parameters/Fixed/*.dat");

		# Create the Modgen Base scenario file
		modgen_create_scex($modgen_Base_scex, $Base_Framework_odat, $ompp_framework_ompp, @modgen_Base_dats);
		if ( ! -e $modgen_Base_scex ) {
			logmsg error, $model, "modgen", "Missing Base scenario file: $modgen_Base_scex";
			last MODGEN;
		}
		
		logmsg info, $model, "modgen", "Run Base scenario";
		# Delete output database to enable subsequent check for success
		my $modgen_Base_mdb = "Base(tbl).mdb";
		unlink $modgen_Base_mdb;

		($merged, $retval) = capture_merged {
			my @args = (
				"${modgen_model_exe}",
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
			logmsg error, $model, "modgen", "Run model failed - retval=${retval}";
			logerrors $merged;
			last MODGEN;
		}
		if ( ! -e $modgen_Base_mdb ) {
			logmsg error, $model, "modgen", "Missing Base scenario database: ${modgen_Base_mdb}";
			last MODGEN;
		}

		# If present, copy event trace / case checksum file to results directory
		my $modgen_Base_debug_txt = "Base(debug).txt";
		if (-e $modgen_Base_debug_txt) {
			#copy $modgen_Base_debug_txt, "${current_outputs_dir}/trace.txt";
			normalize_event_trace $modgen_Base_debug_txt, "${current_outputs_dir}/trace.txt";
		}

		logmsg info, $model, "modgen", "Convert output tables to .csv (${significant_digits} digits of precision)";
		if ( 0 != modgen_tables_to_csv($modgen_Base_mdb, $current_outputs_dir, $significant_digits)) {
			last MODGEN;
		}
		
		# Create digests of outputs
		logmsg info, $model, "modgen", "Create digests of current outputs";
		chdir "${current_outputs_dir}";
		my $digests = create_digest(glob "*.csv *.txt");
		open DIGESTS_TXT, ">digests.txt";
		print DIGESTS_TXT $digests;
		close DIGESTS_TXT;

		# If no reference results, copy current results as a starting point
		if (! -d $reference_outputs_dir) {
			logmsg info, $model, "modgen", "No reference outputs - create using current outputs.";
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
			logmsg info, $model, "modgen", "Current outputs identical to reference outputs";
        }
		else {
			my $different_files = digest_differences($reference_digests, $current_digests);
			logmsg warning, $model, "modgen", "Current outputs differ: ${different_files}";
		}
		
		my $elapsed_seconds = time - $start_seconds;
		my $elapsed_formatted = int($elapsed_seconds/60)."m ".($elapsed_seconds%60)."s";
		logmsg info, $model, "modgen", "Elapsed time = ${elapsed_formatted}";

	}

	########
	# ompp 
	########
	OMPP: {
		if ($do_ompp != 1) {
			last OMPP;
		}

		my $start_seconds = time;
		
		# Project directory for ompp version
		my $project_dir = "${model_dir}/ompp";
		if ( ! -d "${project_dir}" ) {
			logmsg error, $model, "ompp", "Missing folder ${project_dir}";
			last MODGEN;
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
			logmsg error, $model, "ompp", "Missing property sheet contining user macros: $model_props";
			last OMPP;
		}
		# Parse model property sheet and extract selected user macros
		my $model_name;
		my $scenario_name;
		open MODEL_PROPS, "<${model_props}";
		while (<MODEL_PROPS>) {
			chomp;
			my $line = $_;
			if ($line =~ /<MODEL_NAME>(.*)<\/MODEL_NAME>/) {
				$model_name = $1;
			}
			if ($line =~ /<SCENARIO_NAME>(.*)<\/SCENARIO_NAME>/) {
				$scenario_name = $1;
			}
		}
		close DIGESTS_TXT;
		if ( $model_name eq '' ) {
			logmsg error, $model, "ompp", "Missing model name in $model_props";
			last OMPP;
		}
		if ( $scenario_name eq '' ) {
			logmsg error, $model, "ompp", "Missing scenario name in $model_props";
			last OMPP;
		}
		
		# The solution file
		my $model_sln = "Model.sln";
		if ( ! -e $model_sln ) {
			logmsg error, $model, "ompp", "Missing solution file: $model_sln";
			last OMPP;
		}

		logmsg info, $model, "ompp", "omc compile, C++ compile, build executable, publish model and Base scenario";
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
			logmsg error, $model, "ompp", "build failed";
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
		my $model_exe = "${target_dir}/${model}${build_suffix}.exe";
		if ( ! -e $model_exe ) {
			logmsg error, $model, "ompp", "Missing model executable: $model_exe";
			logerrors $merged;
			last OMPP;
		}

		# Data store generated by build
		my $model_sqlite = "${target_dir}/${model_name}.sqlite";
		if ( ! -e $model_sqlite ) {
			logmsg error, $model, "ompp", "Missing data store: $model_sqlite";
			last OMPP;
		}

		# Save copy of generated C++ source code
		for (glob "src/*.h src/*.cpp") {
			copy "$_", "$current_generated_code_dir";
		}

		logmsg info, $model, "ompp", "Run Base scenario";
		
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
			logmsg error, $model, "ompp", "Run Base scenario failed";
			logerrors $merged;
			last OMPP;
		}

		# If present and non-empty copy event trace / case checksum file to results directory
		if (-s $ompp_trace_txt) {
			#copy $ompp_trace_txt, "${current_outputs_dir}/trace.txt";
			normalize_event_trace $ompp_trace_txt, "${current_outputs_dir}/trace.txt";
		}

		logmsg info, $model, "ompp", "Convert output tables to .csv (${significant_digits} digits of precision)";
		if ( 0 != ompp_tables_to_csv($model_sqlite, $current_outputs_dir, $significant_digits)) {
			last OMPP;
		}
		
		# Create digests of outputs
		logmsg info, $model, "ompp", "Create digests of current outputs";
		chdir "${current_outputs_dir}";
		my $digests = create_digest(glob "*.csv *.txt");
		open DIGESTS_TXT, ">digests.txt";
		print DIGESTS_TXT $digests;
		close DIGESTS_TXT;

		# If no reference results, copy current results as a starting point
		if (! -d $reference_outputs_dir) {
			logmsg info, $model, "ompp", "No reference outputs - create using current outputs.";
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
			logmsg info, $model, "ompp", "Current outputs identical to reference outputs";
        }
		else {
			my $different_files = digest_differences($reference_digests, $current_digests);
			logmsg warning, $model, "ompp", "Current outputs differ: ${different_files}";
		}
		
		# Check for differences in current outputs between Modgen and ompp
		my $modgen_digests_txt = "${modgen_current_dir}/outputs/digests.txt";
		my $ompp_digests_txt = "${ompp_current_dir}/outputs/digests.txt";
		if (-e $modgen_digests_txt and -e $ompp_digests_txt) {
			if (compare($modgen_digests_txt, $ompp_digests_txt) == 0) {
				logmsg info, $model, "ompp", "Modgen comparison", "Current outputs identical";
			}
			else {
				my $different_files = digest_differences($modgen_digests_txt, $ompp_digests_txt);
				logmsg warning, $model, "ompp", "Modgen comparison", "Current outputs differ: ${different_files}";
			}
		}
		else {
			logmsg warning, $model, "ompp", "Modgen comparison", "Current outputs not compared due to missing digest";
		}
		
		my $elapsed_seconds = time - $start_seconds;
		my $elapsed_formatted = int($elapsed_seconds/60)."m ".($elapsed_seconds%60)."s";
		logmsg info, $model, "ompp", "Elapsed time = ${elapsed_formatted}";
	}
}