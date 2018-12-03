# Script to run and assemble a batch of scenarios

# Details:
#
# --batch
#
# The name of the batch. Used to construct the name of the batch specification
# input file and the merged output database.
#
# --model
#
# The name of the model.  Used to construct the name of the executable, and to
# find the starting scenario in the input database.
#
# The batch specification file
# 
# Input file named $in_dir/$batch_name.pl
# Assigns values to the following Perl variables:
# $batch_options - A Perl string containing batch-specific Perl code
#   to be executed before processing command-line options.
# @run_names - A Perl array containing the names to be used for each scenario in the batch.
# @run_inis - A Perl array containing multi-line strings, each in the format of an ompp ini file.
#   The contents of @run_inis are used to override default values of model parameters.
# $post_assembly - A Perl string containing batch-specific Perl code
#   to be executed after run assembly.
# The specification file returns the value 1 to indicate success.
#
# --mpi
#
# If set, specifies to use the mpi version of the model executable, which is
# $in_dir/$model.'_mpi.exe'
#
# --runs
#
# Specifies which runs in the batch are to be executed.
# Runs are numbered sequentially starting at 0.
# This optional argument allows splitting the batch into portions
# which can be run independently and subsequently assembled.
#  
# --assemble
#
# Merges the individual SQLite databases produced by each run into a new ompp SQLite database
# in the batch output directory, named <model_name>.sqlite.
#  
# --post_assemble
#
# Just does the batch-specific post-assembly step in $post_assembly, skipping runs and assembly.
#

#use strict;
use warnings;

my $script_name = 'ompp_batch';
my $script_version = '1.1';

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
	[ 'help|h'      => 'print usage message and exit' ],
	[ 'version|v'   => 'print version and exit' ],

	[ 'batch|b=s'   => 'name of the batch' ],
	[ 'model|m=s'   => 'name of the model' ],
	[ 'workset|w=s' => 'name of the starting workset (default is Default)' ],
	[ 'in_dir|i=s'  => 'input directory (default is current directory)' ],
	[ 'out_dir|o=s' => 'output directory (default is current directory)' ],
	[ 'pub_dir=s'   => 'model publishing directory (default is current directory)' ],
	[ 'prefix|p=s'  => 'run prefix (default is no prefix)' ],
	
	[ 'members=i'   => 'members' ],
	[ 'threads=i'   => 'threads' ],
	[ 'processes=i' => 'MPI processes' ],
	[ 'x64'         => 'append suffix 64 to name of executable' ],
	[ 'mpi'         => 'use mpi to name and launch model' ],

	[ 'runs|r=s'    => 'which runs to launch, e.g. 0,1,2,4,8-10' ],
	[ 'force|f'     => 'launch runs even if results present' ],

	[ 'list|l'      => 'just list runs in batch' ],
	[ 'assemble|a'  => 'just assemble existing runs' ],
	[ 'post_assemble|p' => 'just post-assemble existing runs' ],
	[ 'verbose'     => 'additional diagnostic messages' ],
);

if ($opt->version) {
	print $script_name.' version '.$script_version."\n";
	exit 0;
}

if ($opt->help) {
	print $usage->text;
	exit 0;
}

my $in_dir = '.';
$in_dir = $opt->in_dir if $opt->in_dir;
-d $in_dir or die "invalid input directory";
print "in_dir=${in_dir}\n" if $opt->verbose;

#####################
# Batch specification file
#####################

$opt->batch or die "batch name must be specified";
my $batch = $opt->batch;
print "batch=${batch}\n" if $opt->verbose;

my $spec_file = "${in_dir}/${batch}.pl";
-f $spec_file or die "batch specification file '${spec_file}' not found, stopped";
print "spec_file=${spec_file}\n" if $opt->verbose;

# Declare variables which will be set by spec_file
our $batch_options;
our @run_inis;
our @run_names;
our $post_assembly;

# Assign variables from spec_file
unless (my $return = do $spec_file) {
	die "couldn't parse $spec_file: $@" if $@;
	die "couldn't do $spec_file: $!"    unless defined $return;
	die "couldn't run $spec_file"       unless $return;
}

# Perform checks on variables set in the specification file.
$#run_names >=0 or die 'no runs specified, stopped';
$#run_inis == $#run_names or die '@run_inis and @run_names are not the same length, stopped';

####################
# Arguments & derived
####################

# Declare arguments and assign default values

# The name of the model
my $model = '';
# The starting workset
my $workset = 'Default';
# The output directory
my $out_dir = '.';
# The model publishing directory
my $pub_dir = '.';
# The prefix added at the beginning of run names.
my $prefix = '';

# The number of members in each run.
my $members = 1;
# Number of threads in each process.
my $threads = 1;
# Number of MPI processes.
my $processes = 1;
# Flag to append 64 to name of executable.
my $x64 = 0;
# Flag to name executable and launch using mpiexec.
my $mpi = 0;

# Flag indicating to just assemble runs
my $assemble = 0;
# Flag indicating to just post-assemble runs
my $post_assemble = 0;
# Flag indicating to just list runs in batch
my $list = 0;

# Assign any argument values given in the specification file
eval $batch_options;

# Assign any argument values given on the command line.
# Command line arguments override values in batch specification file.
$model         = $opt->model      if $opt->model;
$workset       = $opt->workset    if $opt->workset;
$out_dir       = $opt->out_dir    if $opt->out_dir;
$pub_dir       = $opt->pub_dir    if $opt->pub_dir;
$prefix        = $opt->prefix     if $opt->prefix;

$members       = $opt->members    if $opt->members;
$threads       = $opt->threads    if $opt->threads;
$processes     = $opt->processes  if $opt->processes;
$x64           = 1 if $opt->x64;
$mpi           = 1 if $opt->mpi || $processes > 1;

$assemble      = 1 if $opt->assemble;
$post_assemble = 1 if $opt->post_assemble;
$list          = 1 if $opt->list;

# Checks on final values of arguments

# Input database
my $input_db = "${pub_dir}/${model}.sqlite";
-f $input_db or die "Invalid input database '$input_db'";

# The model executable
my $exe_suffix = '';
$exe_suffix .= '64' if $x64;
$exe_suffix .= '_mpi' if $mpi;
my $model_exe = "${pub_dir}/${model}${exe_suffix}.exe";
$model_exe =~ s/\//\\/g; # replace all forward slash by backwards slash
-x $model_exe or die "invalid model executable '${model_exe}', stopped";

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
else {
	# if runs not specified, default is all runs
	@runs = (0..$#run_names);
}

# Display selected argument and derived values for debugging
if ($opt->verbose) {
	print "batch=${batch}\n";
	print "model_exe=${model_exe}\n";
	print "input_db=${input_db}\n";
	print "model=${model}\n";
	print "out_dir=${out_dir}\n";
	print "assemble=${assemble}\n";
	print "post_assemble=${post_assemble}\n";
	print "list=${list}\n";
	print "runs=@{runs}\n";
}

#############
# Processing
#############

if (! -d $out_dir) {
	make_path $out_dir or die "invalid output directory '${out_dir}, stopped'";
}

# OM_ROOT is needed to locate the dbcopy.exe utility
my $om_root = $ENV{'OM_ROOT'};
defined($om_root) or die 'environment variable OM_ROOT not defined, stopped';
my $dbcopy_exe = "${om_root}/bin/dbcopy.exe" or die;
-f $dbcopy_exe or die "dbcopy utility $dbcopy_exe not found, stopped";

#print "runs are numbered from 0 to $#run_names\n";

if ($assemble == 0 && $post_assemble == 0 && $list == 0) {
	# Launch runs
	
	my $retval; # return value from external commands
	my $merged; # output from external commands
	my $run_id;  # run_id of current run
	my $run_name; # run_name of current run

	for my $run (@runs) {
		my $run_name = $prefix.$run_names[$run];
		
		# The input/output database for this run
		my $database_sqlite = "${out_dir}/${run_name}.sqlite";
		if (-f $database_sqlite && ! $opt->force) {
			# output database already exists, and force option not provided, skip
			print "Skip existing run ${run} - ${run_name}\n";
			next;
		}
		
		print "Launch run    ${run} - ${run_name}\n";
		
		# Create ini file with parameter values
		my $ini_file = "${out_dir}/${run_name}.ini";
		open INI_TXT, '>'.$ini_file or die;
		print INI_TXT $run_inis[$run];
		close INI_TXT;

		my $log_file = "${out_dir}/${run_name}.log";
		
		copy $input_db, $database_sqlite or die;
		
		($merged, $retval) = tee_merged {
			my @args;
			
			if ($processes > 1) {
				my $msmpi_bin = $ENV{'MSMPI_BIN'};
				defined($msmpi_bin) or die "Microsoft MPI not installed";
				my $mpiexec_exe = "${msmpi_bin}mpiexec.exe";
				-f $mpiexec_exe or die "mpiexec.exe not found";
				push @args,
					(
					$mpiexec_exe,
					"-n", $processes
					);
			}

			push @args,
				(
				"${model_exe}",
				"-OpenM.Database", "\"Database=${database_sqlite}; Timeout=86400; OpenMode=ReadWrite;\"",
				"-OpenM.LogToFile", "true",
				"-OpenM.LogFilePath", "${log_file}",
				"-OpenM.SubValues", $members,
				"-OpenM.Threads", $threads,
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
elsif ($list == 1) {
	for my $run (0..$#run_names) {
		my $run_name = $prefix.$run_names[$run];
		my $in_database_sqlite = "${out_dir}/${run_name}.sqlite";
		my $status = (-f $in_database_sqlite) ? "  DONE" : "ABSENT";
		print $run, ' ', $status, ' ', $run_name, "\n";
	}
}
elsif ($assemble == 1) {
	# just assemble results

	my $out_database_sqlite = "${out_dir}/${batch}.sqlite";
	# Create db for assembled results
	use File::Copy;
	copy $input_db, $out_database_sqlite or die;
	# TODO - use dbcopy to remove all existing results from copied DB

	my @missing_runs;
	for my $run (0..$#run_names) {
		my $run_name = $prefix.$run_names[$run];
		my $in_database_sqlite = "${out_dir}/${run_name}.sqlite";
		if (-f $in_database_sqlite) {
			print "Merging results from run ${run} - ${run_name}\n";
			
			my $log_file = "${out_dir}/run_${run}_merge.log";
			
			my $retval; # return value from external commands
			my $merged; # output from external commands
			($merged, $retval) = tee_merged {
				my @args = (
					"${dbcopy_exe}",
					"-m", "${model}",
					"-dbcopy.To", "db2db",
					"-dbcopy.Database", "\"Database=${in_database_sqlite};Timeout=86400;OpenMode=ReadWrite;\"",
					"-dbcopy.ToDatabase", "\"Database=${out_database_sqlite};Timeout=86400;OpenMode=ReadWrite;\"",
					"-OpenM.LogToFile",
					"-OpenM.LogFilePath", "${log_file}",
					);
				system(@args);
			};
		}
		else {
			print "Results missing for run ${run} - ${run_name}\n";
			push @missing_runs, $run.' '.$run_name;
		}
	}

	# post_assembly contains optional code supplied in the batch specification script
	eval $post_assembly;
	
	if ($#missing_runs >= 0) {
		print "List of missing runs in batch:\n";
		print join("\n", @missing_runs), "\n";
	}
	else {
		print "All runs assembled.\n";
	}
	
}
elsif ($post_assemble == 1) {
	# just post-assemble results
	my $out_database_sqlite = "${out_dir}/${batch}.sqlite";
	eval $post_assembly;
}

