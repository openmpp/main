# Copyright (c) 2013-2014 OpenM++
# This code is licensed under MIT license (see LICENSE.txt for details)

# Script to test multiple models in ompp 

use strict;

my @models = (
#	"WizardCaseBased",
	"WizardTimeBased",
#	"Alpha1",
#	"Alpha2",
#	"IDMM",
#	"RiskPaths",
#	"PohemParms",
	);
	
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
		    );
# Check for compile errors, and bail if any.
if ($@) {
	print $@;
	exit;
}
	
my @configurations = {
	"Debug",
#	"Release",
};

my @platforms = {
	"Win32",
#	"x64",
};

my $configuration = "Debug";
#my $configuration = "Release";

my $platform = "Win32";
#my $platform = "x64";
	
my $version = "1.0";

use File::Copy;

# Create absolute directory paths
use Cwd qw(getcwd);
my $test_models_dir = getcwd;
chdir "..";
my $tests_root = getcwd;
	

logmsg info, "=========================";
logmsg info, " test_models.pl ${version} ";
logmsg info, "=========================";

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

my $create_db_sqlite_sql = "${tests_root}/../sql/sqlite/create_db_sqlite.sql";
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
	chdir $test_models_dir;

	logmsg info, " ";

	my $model_dir = "${tests_root}/${model}";
	if (! -d $model_dir) {
		logmsg error, "${model}: Missing model";
		next MODEL;
	}

	##########
	# Modgen 
	##########
	
	my $modgen_model_dir = "${model_dir}/Modgen";
	if ( ! -d "${modgen_model_dir}" ) {
		logmsg error, ${model}, "Missing folder ${modgen_model_dir}";
		next MODEL;
	}

	# Run Modgen
	# Modgen.exe output files are written to the Modgen sub folder using the -D option
	logmsg info, ${model}, "Modgen: Modgen compilation";
	# Change working directory to model source directory for Modgen compilation.
	chdir ${model_dir};
	($merged, $retval) = capture_merged {
		my @args = (
			"${modgen_exe}",
			"-D",
			"${modgen_model_dir}",
			"-EN",
			);
		system(@args);
	};
	if ($retval != 0) {
		logmsg error, ${model}, "Modgen compile failed";
		logerrors $merged;
		next MODEL;
	}

	# Copy custom.h, etc. to Modgen folder
	for my $file (glob "${model_dir}/*.h") {
		copy "$file", "${modgen_model_dir}";
	}
	
	# Build the model
	logmsg info, ${model}, "Modgen: C++ compilation";
	# Change working directory to Modgen sub-directory for C++ compilation.
	chdir "${modgen_model_dir}";
	($merged, $retval) = capture_merged {
		my @args = (
			"${modgen_devenv_exe}",
			"${model}.sln",
			"/BUILD",
			"Debug",
			);
		system(@args);
	};
	if ($retval != 0) {
		logmsg error, ${model}, "C++ compile failed";
		logerrors $merged;
		next MODEL;
	}

	my $modgen_model_exe = "${modgen_model_dir}/Debug/${model}.exe";
	if ( ! -e $modgen_model_exe ) {
		logmsg error, "${model}: Missing model executable: $modgen_model_exe";
		exit 1;
	}
	
	# Prepare the scenario
	# Copy Base .dat files to Modgen folder
	for my $file (glob "${model_dir}/Base/*.dat") {
		copy "$file", "${modgen_model_dir}";
	}

	my $modgen_Base_scex = "${modgen_model_dir}/Base.scex";
	if ( ! -e $modgen_Base_scex ) {
		logmsg error, "${model}: Missing Base scenario file: $modgen_Base_scex";
		exit 1;
	}
	
	logmsg info, ${model}, "Modgen: Run Base scenario";
	# Delete output database to enable check for success
	my $modgen_Base_mdb = "${modgen_model_dir}/Base(tbl).mdb";
	unlink $modgen_Base_mdb;
	# Change working directory to Modgen sub-directory for running model.
	chdir "${modgen_model_dir}";
	($merged, $retval) = capture_merged {
		my @args = (
			"${modgen_model_exe}",
			"-sc",
			"${modgen_Base_scex}"
			);
		system(@args);
	};
	# Modgen models return a code of 1 to indicate success
	#if ($retval != 1) {
	#	logmsg error, ${model}, "Modgen: Run model failed - retval=${retval}";
	#	logerrors $merged;
	#	next MODEL;
	#}
	if ( ! -e $modgen_Base_mdb ) {
		logmsg error, "${model}: Missing Base scenario database: ${modgen_Base_mdb}";
		next MODEL;
	}

	logmsg info, "${model}: Modgen: Convert output tables to .csv";
	# Prepare directory to hold results for model from this invocation of test_models
	my $modgen_model_results_dir = "${test_models_dir}/${model}_Modgen_current";
	if (! -d $modgen_model_results_dir) {
		mkdir $modgen_model_results_dir;
	}
	else {
		# Delete current contents
		unlink glob "${modgen_model_results_dir}/*.*";
	}
	if ( 0 != modgen_tables_to_csv($modgen_Base_mdb, $modgen_model_results_dir)) {
		next MODEL;
	}
	
	
	# TEMPORARY Just bail while developing Modgen stuff
	next MODEL;
	
	################
	# ompp version
	################

	# Folder for previously-known-good reference files
	my $model_reference_dir = "${test_models_dir}/${model}_${configuration}_${platform}_reference";

	# Prepare directory to hold results for model from this invocation of test_models
	my $model_results_dir = "${test_models_dir}/${model}_${configuration}_${platform}_current";
	if (! -d $model_results_dir) {
		mkdir $model_results_dir;
	}
	else {
		# Delete current contents
		unlink glob "${model_results_dir}/*.*";
	}
	
	# Change working directory to model directory for compilation.
	chdir ${model_dir};

	logmsg info, "${model}: ompp: Compile and build";
	($merged, $retval) = capture_merged {
		my @args = (
			"${msbuild_exe}",
			"${model}.sln",
			"/nologo",
			"/verbosity:normal",
			"/clp:ForceNoAlign",
			"/p:Configuration=${configuration}",
			"/p:Platform=${platform}",
			"/t:Rebuild"
			);
		system(@args);
	};
	if ($retval != 0) {
		logmsg error, "${model}: build failed";
		logerrors $merged;
		next MODEL;
	}

	# Executable model generated by build
	my $target_dir = "${tests_root}/${configuration}/${platform}";
	my $model_exe = "${target_dir}/${model}.exe";
	if ( ! -e $model_exe ) {
		logmsg error, "${model}: Missing model executable: $model_exe";
		exit 1;
	}

	# Data store generated by build
	my $model_sqlite = "${target_dir}/${model}.sqlite";
	if ( ! -e $model_sqlite ) {
		logmsg error, "${model}: Missing data store: $model_sqlite";
		exit 1;
	}

	logmsg info, "${model}: ompp: Run Base scenario";
	
	# Change working directory to target_dir where where executable and data store
	# are located. This avoids having to specify the data connection string
	# explicitly when launching the model.
	chdir "${target_dir}";
	
	($merged, $retval) = capture_merged {
		my @args = (
			"${model_exe}",
			"-Base.LogToConsole true",
			"-Base.LogNoMsgTime true",
			);
		system(@args);
	};
	if ($retval != 0) {
		logmsg error, "ompp: Run Base scenario failed";
		logerrors $merged;
		next MODEL;
	}

	logmsg info, "${model}: ompp: Convert output tables to .csv";
	if ( 0 != ompp_tables_to_csv($model_sqlite, $model_results_dir)) {
		next MODEL;
	}

	# Create digests of tables
	logmsg info, $model, "Create digests:\n";
	chdir "${model_results_dir}";
	logmsg info, $model, "digest", create_digest(glob "*.csv");

	# Create Reference results by copying Current results if missing.
	if (! -d $model_reference_dir) {
		logmsg info, "${model}: ompp: No Reference results - populate using Current results.";
		mkdir $model_reference_dir;
		chdir $model_results_dir;
		for (glob "*.*") {
			my $f = $_;
			copy "$model_results_dir/$f", "$model_reference_dir/$f";
		}
	}
	
}