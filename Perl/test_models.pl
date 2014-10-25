# Copyright (c) 2013-2014 OpenM++
# This code is licensed under MIT license (see LICENSE.txt for details)

# Script to test multiple models in ompp 

use strict;

use File::Copy;
use File::Path qw(make_path);
use File::Compare;
use Cwd qw(getcwd);

chdir "../models" || die "Invoke test_models from Perl folder";
my $models_root = getcwd;

my @models;

if ($#ARGV >= 0) {
	@models = @ARGV;
}
else {
	# Create model list by identifying all model subdirectories
	my @paths = glob "*/ompp";
	for my $path (@paths) {
		$path =~ s/\/.*//;
		push @models, $path;
	}
}

my $configuration = "Debug";
#my $configuration = "Release";

my $platform = "Win32";
#my $platform = "x64";
	
my $do_modgen = 1;

my $do_ompp = 1;

# Number of significant digits to retain in the output csv files
my $significant_digits = 9;

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
logmsg info, "";
logmsg info, "Testing models: ".join(", ", @models);

my $modgen_exe = "C:\\Program Files (x86)\\StatCan\\Modgen 11\\Modgen.exe";
if ( ! -e $modgen_exe ) {
	logmsg error, "Missing Modgen compiler: $modgen_exe";
	exit 1;
}

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

	my $modgen_reference_dir = "${model_dir}/test_outputs/reference/modgen";
	my $modgen_current_dir = "${model_dir}/test_outputs/current/modgen";
	my $ompp_reference_dir = "${model_dir}/test_outputs/reference/ompp";
	my $ompp_current_dir = "${model_dir}/test_outputs/current/ompp";

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

		# Folder for reference results
		my $reference_results_dir = $modgen_reference_dir;

		# Folder for current results
		my $current_results_dir = $modgen_current_dir;
		if (! -d $current_results_dir) {
			make_path $current_results_dir;
		}
		else {
			# Delete current contents
			unlink glob "${current_results_dir}/*.*";
		}
		
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

		# The ompp Framework parameter file for the Base scenario.
		# Parameters in that file will be transformed into scenario settings in the Base.scex file
		my $ompp_Base_Framework_ompp = "../parameters/Base/Base(Framework).odat";
		if ( ! -e $ompp_Base_Framework_ompp ) {
			logmsg error, $model, "modgen", "Missing ompp Base Framework parameters: $ompp_Base_Framework_ompp";
			last MODGEN;
		}

		# List of all the .dat files in the Base scenario
		# with relative paths.
		my @modgen_Base_dats;
		push @modgen_Base_dats, glob("../parameters/Base/*.dat");
		push @modgen_Base_dats, glob("../parameters/Fixed/*.dat");

		# Create the Modgen Base scenario file
		modgen_create_scex($modgen_Base_scex, $ompp_Base_Framework_ompp, @modgen_Base_dats);
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
			copy $modgen_Base_debug_txt, "${current_results_dir}/trace.txt";
		}

		logmsg info, $model, "modgen", "Convert output tables to .csv (${significant_digits} digits of precision)";
		if ( 0 != modgen_tables_to_csv($modgen_Base_mdb, $current_results_dir, $significant_digits)) {
			last MODGEN;
		}
		
		# Create digests of outputs
		logmsg info, $model, "modgen", "Create digests of current outputs";
		chdir "${current_results_dir}";
		my $digests = create_digest(glob "*.csv *.txt");
		open DIGESTS_TXT, ">digests.txt";
		print DIGESTS_TXT $digests;
		close DIGESTS_TXT;

		# If no reference results, copy current results as a starting point
		if (! -d $reference_results_dir) {
			logmsg info, $model, "modgen", "No reference outputs - create using current outputs.";
			make_path $reference_results_dir;
			chdir $current_results_dir;
			for (glob "*.*") {
				copy "$_", "$reference_results_dir";
			}
		}
		
		# Check for differences between current and reference results
        if (compare("${current_results_dir}/digests.txt","${reference_results_dir}/digests.txt") == 0) {
			logmsg info, $model, "modgen", "Current outputs identical to reference outputs";
        }
		else {
			logmsg warning, $model, "modgen", "Current outputs differ from reference outputs";
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

		# Folder for reference results
		my $reference_results_dir = $ompp_reference_dir;

		# Folder for current results
		my $current_results_dir = $ompp_current_dir;
		if (! -d $current_results_dir) {
			make_path $current_results_dir;
		}
		else {
			# Delete current contents
			unlink glob "${current_results_dir}/*.*";
		}
		
		# Change working directory to project directory for compilation.
		chdir ${project_dir};

		# The property sheet containing  user macros
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
				"/p:Configuration=${configuration}",
				"/p:Platform=${platform}",
				"/p:GRID_COMPUTING=EMPTY",
				"/p:RUN_SCENARIO=0",
				"/p:EXCEL_EXPORT_SCENARIO=0",
				"/p:EXCEL_LAUNCH_SCENARIO=0",
				"/t:Rebuild"
				);
			system(@args);
		};
		if ($retval != 0) {
			logmsg error, $model, "ompp", "build failed";
			logerrors $merged;
			last OMPP;
		}

		# Executable model generated by build
		my $target_dir = "${project_dir}/${configuration}/${platform}";
		my $model_exe = "${target_dir}/${model}.exe";
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
			copy $ompp_trace_txt, "${current_results_dir}/trace.txt";
		}

		logmsg info, $model, "ompp", "Convert output tables to .csv (${significant_digits} digits of precision)";
		if ( 0 != ompp_tables_to_csv($model_sqlite, $current_results_dir, $significant_digits)) {
			last OMPP;
		}
		
		# Create digests of outputs
		logmsg info, $model, "ompp", "Create digests of current outputs";
		chdir "${current_results_dir}";
		my $digests = create_digest(glob "*.csv *.txt");
		open DIGESTS_TXT, ">digests.txt";
		print DIGESTS_TXT $digests;
		close DIGESTS_TXT;

		# If no reference results, copy current results as a starting point
		if (! -d $reference_results_dir) {
			logmsg info, $model, "ompp", "No reference outputs - create using current outputs.";
			make_path $reference_results_dir;
			chdir $current_results_dir;
			for (glob "*.*") {
				copy "$_", "$reference_results_dir";
			}
		}
		
		# Check for differences between current and reference outputs
        if (compare("${current_results_dir}/digests.txt","${reference_results_dir}/digests.txt") == 0) {
			logmsg info, $model, "ompp", "Current outputs identical to reference outputs";
        }
		else {
			logmsg warning, $model, "ompp", "Current outputs differ from reference outputs";
		}
		
		# Check for differences in current outputs between Modgen and ompp
		if (-e "${modgen_current_dir}/digests.txt" and -e "${ompp_current_dir}/digests.txt") {
			if (compare("${modgen_current_dir}/digests.txt","${ompp_current_dir}/digests.txt") == 0) {
				logmsg info, $model, "ompp", "Modgen comparison", "Outputs identical";
			}
			else {
				my $different_files = digest_differences("${modgen_current_dir}/digests.txt","${ompp_current_dir}/digests.txt");
				logmsg warning, $model, "ompp", "Modgen comparison", "Outputs differ: ${different_files}";
			}
		}
		else {
			logmsg warning, $model, "ompp", "Modgen comparison", "Outputs not compared due to missing digest";
		}
		
		my $elapsed_seconds = time - $start_seconds;
		my $elapsed_formatted = int($elapsed_seconds/60)."m ".($elapsed_seconds%60)."s";
		logmsg info, $model, "ompp", "Elapsed time = ${elapsed_formatted}";
	}
}