# Copyright (c) 2013-2014 OpenM++
# This code is licensed under MIT license (see LICENSE.txt for details)

# Script to test multiple models in ompp 

use strict;

my @models = (
	"WizardCaseBased",
	"WizardTimeBased",
	"Alpha1",
	"Alpha2",
	"IDMM",
	"RiskPaths",
#	"PohemParms",
	);

my $configuration = "Debug";
#my $configuration = "Release";

my $platform = "Win32";
#my $platform = "x64";
	
my $version = "1.0";

# Strings used as line prefixes in output line
my $warning = " WARNING-> ";
my $change  = " CHANGE--> ";
my $error   = " ERROR---> ";
my $info    = "           ";

# Output a log message
sub logmsg {
	my $prefix = shift(@_);
	my $msg = shift(@_);
	print "${prefix}${msg}\n";
}

# Extract errors from multi-line string to log
sub logerrors {
	my @lines = split(/\n/, @_[0]);
	for (@lines) {
		if (/error/i) {
			my $line = $_;
			# trim leading whitespace
			$line =~ s/^\s*//;
			# if line starts with path portion, remove it
			$line =~ s/^[A-Z]:[^:]*[\\]//;
			# if line is too long, truncate it
			if (length($line) > 130) {
				$line = substr($line, 0, 125)." ...";
			}
			logmsg $info, "     ${line}";
		}
	}
}

# Find out where we are in the file system to avoid relative path anomalies
use Cwd qw(getcwd);
my $test_models_dir = getcwd;
chdir "..";
my $tests_root = getcwd;
	

logmsg $info, "=========================";
logmsg $info, " test_models.pl ${version} ";
logmsg $info, "=========================";

# Steps to install Capture::Tiny (this method requires internet connectivity, there are other ways)
# (1) open command prompt
# (2) find and run vsvars32.bat (to get path to nmake into environment)
#     Normally some place like
#     "C:\Program Files (x86)\Microsoft Visual Studio 10.0\Common7\Tools\vsvars32.bat"
# (3) perl -MCPAN -e "install Capture::Tiny"
use Capture::Tiny qw/capture tee capture_merged tee_merged/;

my $modgen_exe = "C:\\Program Files (x86)\\StatCan\\Modgen 11\\Modgen.exe";
if ( ! -e $modgen_exe ) {
	logmsg $error, "Missing Modgen compiler: $modgen_exe";
	exit 1;
}

my $modgen_devenv_exe = "C:\\Program Files (x86)\\Microsoft Visual Studio 10.0\\Common7\\IDE\\devenv.exe";
if ( ! -e $modgen_devenv_exe ) {
	logmsg $error, "Missing Visual Studio devenv for Modgen: $modgen_devenv_exe";
	exit 1;
}

# MSBuild command line reference:
# http://msdn.microsoft.com/en-us/library/ms164311.aspx
my $msbuild_exe = "C:\\Program Files (x86)\\MSBuild\\12.0\\Bin\\MSBuild.exe";
if ( ! -e $msbuild_exe ) {
	logmsg $error, "Missing MSBuild for ompp: $msbuild_exe";
	exit 1;
}

# SQLite3 reference page
# http://sqlite.org/cli.html
my $sqlite3_exe = "${tests_root}/../bin/sqlite3.exe";
if ( ! -e $sqlite3_exe ) {
	logmsg $error, "Missing SQLite for ompp: $sqlite3_exe";
	exit 1;
}

# Run a SQL script on a SQLite database
# arg0 - the SQLite database
# arg1 - the file of SQL statements to run
# arg2 - the log message associated with this action
# arg3 - the return value from SQLite3
# returns - a list of strings
sub run_sql_script {
	my $db = shift(@_);
	my $sql = shift(@_);
	my $msg = shift(@_);
	
	logmsg $info, $msg;
	(my $merged, my $retval) = capture_merged {
		my @args = (
			"${sqlite3_exe}",
			"${db}",
			".read ${sql}",
			);
		system(@args);
	};
	# Place return value from sqlite3 into 4th argument
	@_[0] = $retval;
	my @lines = split(/\n/, $merged);
	if ($retval != 0) {
		logmsg $error, $msg." failed - SQLite errors follow:";
		logerrors $merged;
	}
	return @lines;
}

# Run a SQL statement on a SQLite database
# arg0 - the SQLite database
# arg1 - the SQL statement
# arg2 - the return value from SQLite3
# returns - a list of strings
sub run_sql_statement {
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
	# Place return value from sqlite3 into 4th argument
	@_[0] = $retval;
	my @lines = split(/\n/, $merged);
	return @lines;
}

my $create_db_sqlite_sql = "${tests_root}/../sql/sqlite/create_db_sqlite.sql";
if ( ! -e $create_db_sqlite_sql ) {
	logmsg $error, "Missing SQL file used to create new data store for ompp: $create_db_sqlite_sql";
	exit 1;
}

MODEL: for my $model (@models) {
	my $retval; # return value from external commands
	my $merged; # output from external commands
	
	# Specify working directory explicitly because errors can bail back to next model
	# with the working directory in an unknown state.
	chdir $test_models_dir;

	logmsg $info, "";

	my $model_dir = "${tests_root}/${model}";
	if (! -d $model_dir) {
		logmsg $error, "${model}: Missing model";
		next MODEL;
	}
	
	# Prepare directory to hold results for model from this invocation of test_models
	my $model_results_dir = "${test_models_dir}/${model}_current";
	if (! -d $model_results_dir) {
		mkdir $model_results_dir;
	}
	else {
		# Delete current contents
		unlink glob "${model_results_dir}/*.*";
	}
	
	# Change working directory to model directory for compilation.
	chdir "${model_dir}";

	logmsg $info, "${model}: ompp: Compile and build";
	($merged, $retval) = capture_merged {
		my @args = (
			"${msbuild_exe}",
			"${model}.sln",
			"/nologo",
			"/verbosity:minimal",
			"/clp:ForceNoAlign",
			"/p:Configuration=${configuration}",
			"/p:Platform=${platform}",
			"/t:rebuild"
			);
		system(@args);
	};
	if ($retval != 0) {
		logmsg $error, "${model}: build failed";
		logerrors $merged;
		next MODEL;
	}

	# Generated SQL to insert model metadata
	my $create_model_sql =  "${model_dir}/src/${model}_create_model.sql";
	if ( ! -e $create_model_sql ) {
		logmsg $error, "${model}: Missing SQL output file for model metadata: $create_model_sql";
		exit 1;
	}

	# Generated SQL to insert parameter and table views
	my $optional_views_sql =  "${model_dir}/src/${model}_optional_views.sql";
	if ( ! -e $optional_views_sql ) {
		logmsg $error, "${model}: Missing SQL output file for parameter and table views: $optional_views_sql";
		exit 1;
	}

	# Generated SQL to insert model parameters for Base scenario
	my $model_Base_sql = "${model_dir}/src/${model}_Base.sql";
	if ( ! -e $model_Base_sql ) {
		logmsg $error, "${model}: Missing SQL output file for Base scenario: $model_Base_sql";
		exit 1;
	}
	
	my $model_sqlite = "${model_results_dir}/${model}.sqlite";
	unlink $model_sqlite;
	if ( -e $model_sqlite) {
		logmsg $error, "${model}: Unable to remove previous data store - check if open";
		next MODEL;
	}
	
	run_sql_script $model_sqlite, $create_db_sqlite_sql, "${model}: ompp: Create data store", $retval;
	next MODEL if $retval != 0;

	run_sql_script $model_sqlite, $create_model_sql, "${model}: ompp: Write model metadata to data store", $retval;
	next MODEL if $retval != 0;
	
	run_sql_script $model_sqlite, $optional_views_sql, "${model}: ompp: Write parameter and table views to data store", $retval;
	next MODEL if $retval != 0;

	run_sql_script $model_sqlite, $model_Base_sql, "${model}: ompp: Write Base scenario inputs to data store", $retval;
	next MODEL if $retval != 0;

	# Executable model generated by build
	my $model_exe = "${tests_root}/${configuration}/${platform}/${model}.exe";
	if ( ! -e $model_exe ) {
		logmsg $error, "${model}: Missing model executable: $model_exe";
		exit 1;
	}

	logmsg $info, "${model}: ompp: Run Base scenario";
	
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
		logmsg $error, "ompp: Run Base scenario failed";
		logerrors $merged;
		next MODEL;
	}
	
	logmsg $info, "${model}: ompp: Get table names from data store";
	my @tables = run_sql_statement $model_sqlite, "Select table_name From table_dic Order By table_name;", $retval;
	if ($retval != 0) {
		logmsg $error, "ompp: Get table names failed";
		next MODEL;
	}
	
	# Create and run SQLite3 script to extract tables
	my $temp_sql = "${model_results_dir}/temp.sql";
	if (!open TEMP_SQL, ">${temp_sql}") {
		logmsg $error, "ompp: Unable to open ${temp_sql} for writing.";
		next MODEL;
	}
	print TEMP_SQL ".mode csv\n";
	print TEMP_SQL ".headers on\n";
	for my $table (@tables) {
		print TEMP_SQL ".output ${model_results_dir}/${table}.csv\n";
		print TEMP_SQL "Select * From ${table};\n";
	}
	close TEMP_SQL;
	
	run_sql_script $model_sqlite, $temp_sql, "${model}: ompp: Create csv files from output tables", $retval;
	next MODEL if $retval != 0;

	unlink $temp_sql;

}