# Script to run and assemble a batch of scenarios

#use strict;
use warnings;

my $script_name = 'ompp_batch';
my $script_version = '1.0';

use Getopt::Long::Descriptive;
use Cwd;
use File::Path qw(make_path remove_tree);
use File::Copy;
use Capture::Tiny qw/capture tee capture_merged tee_merged/;
use File::Basename;

# add the directory of this script to search path to use Perl modules
use lib dirname (__FILE__);

# use portions of the common.pm module of shared functions	
use common qw(
				run_sqlite_script
				run_sqlite_statement
		    );

my ($opt, $usage) = describe_options(
	$script_name.' %o',
	[ 'help|h'    => 'print usage message and exit' ],
	[ 'version|v' => 'print version and exit' ],
	[ 'spec_file|s=s' => 'the batch specification script' ],
	[ 'model_dir|m=s' => 'model directory (default is ..)' ],
	[ 'model_name|n=s' => 'model name (default is model directory)' ],
	[ 'output_dir|o=s' => 'output directory (default is ../output/<spec_name>' ],
	[ 'runs|r=s' => 'runs to launch, e.g. 1,2,4,8-10' ],
	[ 'exe64' => 'use 64-bit model executable' ],
	[ 'assemble|a' => 'assemble existing runs' ],
	[ 'clean|c' => 'remove output directory and its contents' ],
);

if ($opt->version) {
	print $script_name.' version '.$script_version."\n";
	exit 0;
}

if ($opt->help) {
	print $usage->text;
	exit 0;
}

my $curr_dir = getcwd;

if (! $opt->spec_file) {
	print "spec_file is required";
	exit -1;
}
my $spec_file = $opt->spec_file;
-f $spec_file or die "spec_file=${spec_file} not found, stopped";
#print "spec_file=${spec_file}\n";

# The specification name is the root name of spec_file
my $spec_name = $spec_file;
# Remove leading path portion
$spec_name =~ s/.*[\/\\]//;
# remove trailing .pl if present
$spec_name =~ s/[.]pl$//;
#print "spec_name=${spec_name}\n";

# The default model_dir is the parent folder of the current directory.
# e.g. if cwd is GMM/batch, model_dir will be '..'
my $model_dir = '..';
if ($opt->model_dir) {
	$model_dir = $opt->model_dir;
}
#print "model_dir=${model_dir}\n";

# The default model_name is the name of the parent folder of the current directory.
# e.g. if cwd is GMM/batch, model_name will be GMM
#print "curr_dir=${curr_dir}\n";
$curr_dir =~ /(\w+)[\/\\]\w+$/;
my $model_name = $1;
if ($opt->model_name) {
	$model_name = $opt->model_name;
}
#print "model_name=${model_name}\n";

# The default output_dir is a folder in the sister folder 'output'
# of the current directory named spec_name.
# e.g. if cwd is GMM/batch, and spec_name is 'batch1', output_dir will be ../output/batch1
my $output_dir = "../output/${spec_name}";
if ($opt->output_dir) {
	$model_dir = $opt->output_dir;
}
#print "output_dir=${output_dir}\n";
if (! -d $output_dir) {
	make_path $output_dir;
}

# Flag indicating to assemble runs
my $assemble = 0;
if ($opt->assemble) {
	$assemble = 1;
}
#print "assemble=${assemble}\n";

# Flag indicating to clean result_dir
my $clean = 0;
if ($opt->clean) {
	$clean = 1;
}
#print "assemble=${assemble}\n";

# Array of runs to perform
my @runs;
if (defined($opt->runs)) {
	my @parts = split(',', $opt->runs);
	# check for validity and expand range specifiers like 5-8
	for my $part (@parts) {
		if ($part =~ /^\d+$/) {
			push @runs, $part;
		}
		elsif ($part =~ /^(\d+)-(\d+)$/) {
			push @runs, ($1..$2);
		}
		else {
			print "invalid -runs ".$opt->runs;
			exit -1;
		}
	}
}
#print "runs=@{runs}\n";

my $exe64 = 0;
if ($opt->exe64) {
	$exe64 = 1;
}
#print "exe64=${exe64}\n";


# OM_ROOT is needed to locate the dbcopy.exe utility
my $om_root = $ENV{'OM_ROOT'};
$om_root ne '' or die 'environment variable OM_ROOT not defined, stopped';
my $dbcopy_exe = "${om_root}/bin/dbcopy.exe" or die;
-f $dbcopy_exe or die "dbcopy utility $dbcopy_exe not found, stopped";

# Starting database with Default scenario (from omc)
my $start_sqlite = "${model_dir}/output/${model_name}.sqlite";
-f $start_sqlite or die;

our @run_inis;
our @run_names;
our $post_assembly;

unless (my $return = do $spec_file) {
	warn "couldn't parse $spec_file: $@" if $@;
	warn "couldn't do $spec_file: $!"    unless defined $return;
	warn "couldn't run $spec_file"       unless $return;
}

# Perform checks on variables set in the batch specification file.
$#run_names >=0 or die 'no runs specified, stopped';
$#run_inis == $#run_names or die '@run_inis and @run_names are not the same length, stopped';

# if runs not specified, default is all runs
if ($#runs < 0) {
	@runs = (0..$#run_names);
}

#print "runs are numbered from 0 to $#run_names\n";

if ($assemble == 0 && $clean == 0) {
	# Launch runs
	my $model_exe = "${model_dir}/ompp/bin/${model_name}.exe";
	if ($exe64) {
		$model_exe = "${model_dir}/ompp/bin/${model_name}64.exe";
	}
	#print "model_exe=${model_exe}\n";
	-f $model_exe or die "model executable ${model_exe} not found, stopped";
	
	my $retval; # return value from external commands
	my $merged; # output from external commands
	my $run_id;  # run_id of current run
	my $run_name; # run_name of current run

	for my $run (@runs) {
		my $run_name = $run_names[$run];
		print "Launching run ${run} - ${run_name}\n";
		
		# Create ini file with parameter values
		my $ini_file = "${output_dir}/run_${run}.ini";
		open INI_TXT, '>'.$ini_file or die;
		print INI_TXT $run_inis[$run];
		close INI_TXT;

		my $log_file = "${output_dir}/run_${run}.log";
		
		# The output database for this batch
		my $database_sqlite = "${output_dir}/run_${run}.sqlite";
		copy $start_sqlite, $database_sqlite or die;
		
		($merged, $retval) = tee_merged {
			my @args = (
				"${model_exe}",
				"-OpenM.Database", "\"Database=${database_sqlite}; Timeout=86400; OpenMode=ReadWrite;\"",
				"-OpenM.LogToFile", "true",
				"-OpenM.LogFilePath", "${log_file}",
				"-ini", $ini_file,
				);
			system(@args);
		};
		$retval == 0 or die;

		# retrieve run_id from log (not actually used at present)
		$merged =~ /Run: ([\d]+)/;
		$run_id = $1;
		#print "run_id=$run_id\n";
		#unlink $ini_file;
	}
}
elsif ($assemble == 1) {
	# assemble results

	my $out_database_sqlite = "${output_dir}/runs.sqlite";
	# Create db for assembled results
	use File::Copy;
	copy $start_sqlite, $out_database_sqlite or die;

	for my $run (0..$#run_names) {
		my $run_name = $run_names[$run];
		my $in_database_sqlite = "${output_dir}/run_${run}.sqlite";
		if (-f $in_database_sqlite) {
			print "Merging results from run ${run} - ${run_name}\n";
			
			my $log_file = "${output_dir}/run_${run}_merge.log";
			
			my $retval; # return value from external commands
			my $merged; # output from external commands
			($merged, $retval) = tee_merged {
				my @args = (
					"${dbcopy_exe}",
					"-m", "${model_name}",
					"-dbcopy.To", "db2db",
					"-dbcopy.Database", "\"Database=${in_database_sqlite};Timeout=86400;OpenMode=ReadWrite;\"",
					"-dbcopy.ToDatabase", "\"Database=${out_database_sqlite};Timeout=86400;OpenMode=ReadWrite;\"",
					"-OpenM.LogFilePath", "${log_file}",
					"-OpenM.LogToFile", "true",
					);
				system(@args);
			};
		}
		else {
			print "Results missing for run ${run} - ${run_name}\n";
		}
	}

	# post_assembly contains optional code supplied in the batch specification script
	eval $post_assembly;
}
elsif ($clean == 1) {
	print "Cleaning batch ${output_dir}";
	if (-d $output_dir) {
		remove_tree $output_dir;
	}
}


