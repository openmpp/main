# Creates a parameter set with output from an upstream model for use in a downstream model

# Details:
# 

#use strict;
use warnings;

my $script_name = 'create_import_set';
my $script_version = '1.0';

use Getopt::Long::Descriptive;
use Cwd;
use File::Path qw(make_path remove_tree);
use File::Copy;
use File::Temp qw(tempdir);
use Capture::Tiny qw/capture tee capture_merged tee_merged/;
use File::Basename;

# add the directory of this script to search path to use Perl modules
use lib dirname (__FILE__);

my ($opt, $usage) = describe_options(
	$script_name.' %o',
	[ 'help|h'      => 'print usage message and exit' ],
	[ 'version|v'   => 'print version and exit' ],

	[ 'imports|i=s' => 'path of imports csv file of downstream model' ],
	[ 'model|m=s'   => 'name of upstream model' ],
	[ 'run|r=s'     => 'path of input zip of upstream model run' ],
	[ 'set|s=s'     => 'path of output zip of downstream parameter set' ],
	[ 'verbose'    => 'verbose log output' ],
);

if ($opt->version) {
	print $script_name.' version '.$script_version."\n";
	exit 0;
}

if ($opt->help) {
	print $usage->text;
	exit 0;
}

# Obtain arguments

my $imports = '';
$imports = $opt->imports if $opt->imports;
-e $imports or die "invalid path for model imports file ${imports}";

my $model = '';
$model = $opt->model if $opt->model;
length($model) > 0 or die "name of upstream model must be specified using -m";

my $run = '';
$run = $opt->run if $opt->run;
-e $run or die "invalid path for run file ${run}";

my $set = '';
$set = $opt->set if $opt->set;
# TODO attempt to create file, and bail if not successful

my $verbose = 0;
$verbose = 1 if $opt->verbose;

