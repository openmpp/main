# Copyright (c) 2013-2014 OpenM++
# This code is licensed under MIT license (see LICENSE.txt for details)

# Script to test multiple models in ompp 

use strict;

my @models = (
	"WizardCaseBased",
	"WizardTimeBased",
	"Alpha1",
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

	logmsg info, "";

	my $model_dir = "${tests_root}/${model}";
	if (! -d $model_dir) {
		logmsg error, "${model}: Missing model";
		next MODEL;
	}
	
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

	# Generated SQL to insert model meta-data
	my $create_model_sql =  "${model_dir}/src/${model}_create_model.sql";
	if ( ! -e $create_model_sql ) {
		logmsg error, "${model}: Missing SQL output file for model metadata: $create_model_sql";
		exit 1;
	}

	# Generated SQL to insert parameter and table views
	my $optional_views_sql =  "${model_dir}/src/${model}_optional_views.sql";
	if ( ! -e $optional_views_sql ) {
		logmsg error, "${model}: Missing SQL output file for parameter and table views: $optional_views_sql";
		exit 1;
	}

	# Generated SQL to insert model parameters for Base scenario
	my $model_Base_sql = "${model_dir}/src/${model}_Base.sql";
	if ( ! -e $model_Base_sql ) {
		logmsg error, "${model}: Missing SQL output file for Base scenario: $model_Base_sql";
		exit 1;
	}
	
	my $model_sqlite = "${model_results_dir}/${model}.sqlite";
	unlink $model_sqlite;
	if ( -e $model_sqlite) {
		logmsg error, "${model}: Unable to remove previous data store - check if open";
		next MODEL;
	}
	
	logmsg info, "${model}: ompp: Create data store";
	run_sqlite_script $model_sqlite, $create_db_sqlite_sql, $retval;
	next MODEL if $retval != 0;

	logmsg info, "${model}: ompp: Write model metadata to data store";
	run_sqlite_script $model_sqlite, $create_model_sql, $retval;
	next MODEL if $retval != 0;

	logmsg info, "${model}: ompp: Write parameter and table views to data store";
	run_sqlite_script $model_sqlite, $optional_views_sql, $retval;
	next MODEL if $retval != 0;

	logmsg info, "${model}: ompp: Write Base scenario inputs to data store";
	run_sqlite_script $model_sqlite, $model_Base_sql, $retval;
	next MODEL if $retval != 0;

	# Executable model generated by build
	my $model_exe = "${tests_root}/${configuration}/${platform}/${model}.exe";
	if ( ! -e $model_exe ) {
		logmsg error, "${model}: Missing model executable: $model_exe";
		exit 1;
	}

	logmsg info, "${model}: ompp: Run Base scenario";
	
	# Change working directory to model results directory where data store
	# for model is located. This avoids having to specify the data connection string
	# explicitly when launching the model.
	chdir "${model_results_dir}";
	
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
	logmsg info, $model, "digest", create_digest(glob "*.csv");

	use File::Copy;
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