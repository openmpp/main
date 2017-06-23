# Script to run and assemble a batch of scenarios

# Details:
#
# --executable
#
# The stem of the model executable is the model name.
# If the model name would contain a trailing _mpi, it is removed.
#
# --output_dir
#
# The output and intermediate files of the batch
# are written to a batch-specific directory.
#
# --spec_file
# 
# Provides the name of the Perl batch specification file,
# which assigns values to the following Perl variables:
# $default_values - A Perl string containing batch-specific Perl code
#   to be executed before processing command-line options.
# @run_names - A Perl array containing the names to be used for each scenario in the batch.
# @run_inis - A Perl array containing multi-line strings, each in the format of an ompp ini file.
#   The contents of @run_inis are used to override default values of model parameters.
# $post_assembly - A Perl string containing batch-specific Perl code
#   to be executed after run assembly.
# The specification file returns the value 1 to indicate success.
#
# --processes
#
# If processes is greater than 1, mpiexec is used to launch the executable.
# Otherwise, the executable is launched directly.
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
	[ 'help|h'    => 'print usage message and exit' ],
	[ 'version|v' => 'print version and exit' ],
	[ 'spec_file|s=s' => 'the batch specification script' ],
	[ 'executable|e=s' => 'model executable' ],
	[ 'input_db|i=s' => 'input DB containing Default scenario' ],
	[ 'prefix|p=s' => 'run prefix' ],
	[ 'members|m=i' => 'members' ],
	[ 'threads|t=i' => 'threads' ],
	[ 'processes=i' => 'MPI processes' ],
	[ 'output_dir|o=s' => 'output directory (default is ./<spec_file_stem>' ],
	[ 'runs|r=s' => 'which runs to launch, e.g. 0,1,2,4,8-10' ],
	[ 'list|l' => 'just list runs in batch' ],
	[ 'assemble|a' => 'just assemble existing runs' ],
	[ 'post_assemble|p' => 'just post-assemble existing runs' ],
	[ 'clean|c' => 'just remove output directory and its contents' ],
	[ 'verbose' => 'additional diagnostic messages' ],
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

#####################
# Specification file
#####################

if (! $opt->spec_file) {
	print "spec_file is required";
	exit -1;
}
my $spec_file = $opt->spec_file;
-f $spec_file or die "spec_file=${spec_file} not found, stopped";
#print "spec_file=${spec_file}\n";

# Declare variables set by spec_file
our $default_options;
our @run_inis;
our @run_names;
our $post_assembly;

# Assign variables using spec_file
unless (my $return = do $spec_file) {
	die "couldn't parse $spec_file: $@" if $@;
	die "couldn't do $spec_file: $!"    unless defined $return;
	die "couldn't run $spec_file"       unless $return;
}

# The specification name is the stem of the spec_file
my $spec_name = $spec_file;
# Remove leading path portion
$spec_name =~ s/.*[\/\\]//;
# remove trailing .pl if present
$spec_name =~ s/[.]pl$//;

# Perform checks on variables set in the specification file.
$#run_names >=0 or die 'no runs specified, stopped';
$#run_inis == $#run_names or die '@run_inis and @run_names are not the same length, stopped';


####################
# Arguments & derived
####################

# Declare arguments and assign default values

# The model executable - default assumes current dir is subdir like model/batch
my $executable = glob('../ompp/bin/*.exe');
# The number of members in each run.
my $members = 1;
# The prefix added at the beginning of run names.
my $prefix = '';
# Number of threads in each process.
my $threads = 1;
# Number of MPI processes.
my $processes = 1;
# Input database
my $input_db = glob('../output/*.sqlite');
# Output directory - default is a subdirectory of the current directory, named spec_name.
my $output_dir = "./${spec_name}";
# Flag indicating to just assemble runs
my $assemble = 0;
# Flag indicating to just post-assemble runs
my $post_assemble = 0;
# Flag indicating to just list runs in batch
my $list = 0;
# Flag indicating to just clean result_dir
my $clean = 0;


# Assign any argument values given in the specification file
eval $default_options;

# Assign any argument values given on the command line
$executable    = $opt->executable if $opt->executable;
$input_db      = $opt->input_db   if $opt->input_db;
$prefix        = $opt->prefix     if $opt->prefix;
$members       = $opt->members    if $opt->members;
$threads       = $opt->threads    if $opt->threads;
$processes     = $opt->processes  if $opt->processes;
$output_dir    = $opt->output_dir if $opt->output_dir;
$assemble      = 1 if $opt->assemble;
$post_assemble = 1 if $opt->post_assemble;
$list          = 1 if $opt->list;
$clean         = 1 if $opt->clean;


# The model name is the 'stem' portion of executable
my $model_name = $executable;
# Remove leading path portion
$model_name =~ s/.*[\/\\]//;
# remove trailing .exe if present
$model_name =~ s/[.]exe$//;
# remove trailing _mpi if present
$model_name =~ s/_mpi$//;

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
	print "spec_name=${spec_name}\n";
	print "executable=${executable}\n";
	print "input_db=${input_db}\n";
	print "model_name=${model_name}\n";
	print "assemble=${assemble}\n";
	print "output_dir=${output_dir}\n";
	print "post_assemble=${post_assemble}\n";
	print "list=${list}\n";
	print "clean=${clean}\n";
	print "runs=@{runs}\n";
}

#############
# Processing
#############

if (! -d $output_dir) {
	make_path $output_dir;
}

# OM_ROOT is needed to locate the dbcopy.exe utility
my $om_root = $ENV{'OM_ROOT'};
$om_root ne '' or die 'environment variable OM_ROOT not defined, stopped';
my $dbcopy_exe = "${om_root}/bin/dbcopy.exe" or die;
-f $dbcopy_exe or die "dbcopy utility $dbcopy_exe not found, stopped";

# Starting database with Default scenario (from omc)
-f $input_db or die;

#print "runs are numbered from 0 to $#run_names\n";

if ($assemble == 0 && $post_assemble == 0 && $clean == 0 && $list == 0) {
	# Launch runs
	-f $executable or die "model executable ${executable} not found, stopped";
	
	my $retval; # return value from external commands
	my $merged; # output from external commands
	my $run_id;  # run_id of current run
	my $run_name; # run_name of current run

	for my $run (@runs) {
		my $run_name = $prefix.$run_names[$run];
		print "Launching run ${run} - ${run_name}\n";
		
		# Create ini file with parameter values
		my $ini_file = "${output_dir}/run_${run}.ini";
		open INI_TXT, '>'.$ini_file or die;
		print INI_TXT $run_inis[$run];
		close INI_TXT;

		my $log_file = "${output_dir}/run_${run}.log";
		
		# The output database for this batch
		my $database_sqlite = "${output_dir}/run_${run}.sqlite";
		copy $input_db, $database_sqlite or die;
		
		($merged, $retval) = tee_merged {
			my @args;
			
			if ($processes > 1) {
				my $msmpi_bin = $ENV{'MSMPI_BIN'};
				$msmpi_bin ne '' or die "Microsoft MPI not installed";
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
				"${executable}",
				"-OpenM.Database", "\"Database=${database_sqlite}; Timeout=86400; OpenMode=ReadWrite;\"",
				"-OpenM.LogToFile", "true",
				"-OpenM.LogFilePath", "${log_file}",
				"-OpenM.SubValues", $members,
				"-SubValue.Member", "iota",
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
		my $in_database_sqlite = "${output_dir}/run_${run}.sqlite";
		my $status = (-f $in_database_sqlite) ? "  DONE" : "ABSENT";
		print $run, ' ', $status, ' ', $run_name, "\n";
	}
}
elsif ($assemble == 1) {
	# just assemble results

	my $out_database_sqlite = "${output_dir}/${model_name}.sqlite";
	# Create db for assembled results
	use File::Copy;
	copy $input_db, $out_database_sqlite or die;
	# TODO - use dbcopy to remove all existing results from copied DB

	my @missing_runs;
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
		print "One or more missing runs in batch:\n";
		print join("\n", @missing_runs), "\n";
	}
	else {
		print "All runs assembled.\n";
	}
	
}
elsif ($post_assemble == 1) {
	# just post-assemble results
	my $out_database_sqlite = "${output_dir}/runs.sqlite";
	eval $post_assembly;
}
elsif ($clean == 1) {
	# just clean output directory
	print "Cleaning batch ${output_dir}";
	if (-d $output_dir) {
		remove_tree $output_dir;
	}
}
