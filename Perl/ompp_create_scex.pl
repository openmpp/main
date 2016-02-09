# Copyright (c) 2013-2016 OpenM++ contributors
# This code is licensed under the MIT license (see LICENSE.txt for details)

# Script to create a Modgen .scex file given ompp framework files and a list of .dat files.

use strict;
use warnings;

my $script_name = "ompp_create_scex";
my $script_version = 1.0;

use Getopt::Long::Descriptive;

my ($opt, $usage) = describe_options(
	$script_name.' %o <parameter-dat-file>...',
	[ 'help|h'     => "print usage message and exit" ],
	[ 'version|v'  => "print version and exit" ],
	[ 'scex:s'       => "output scex Modgen scenario file",
		{ required => 1 } ],
	[ 'odat:s'       => "input odat OpenM++ scenario framework parameters",
		{ required => 1 } ],
	[ 'ompp:s'       => "input ompp OpenM++ model framework module",
		{ required => 1 } ],
	[ 'props:s'      => "input model OpenM++ Visual Studio user macros file",
		{ required => 1 } ],
);

if ($opt->version) {
	print $script_name.' version '.$script_version."\n";
	exit 0;
}

if ($opt->help) {
	print $usage->text;
	exit 0;
}

if ( $#ARGV < 0 ) {
	# must have at least one parameter input file argument
	print $usage->text;
	exit -1;
}

my $out_scex = $opt->scex;
my $in_odat = $opt->odat;
my $in_ompp = $opt->ompp;
my $model_props = $opt->props;
my @in_dat = @ARGV;

# Control verbosity of log output (0, 1, 2)
my $verbosity = 0;

use common qw(
				warning change error info
				modgen_create_scex
		    );
# Check for compile errors, and bail if any.
if ($@) {
	print $@;
	exit 1;
}

if (! -s $in_odat) {
	logmsg error, $script_name, "Framework parameter file ${in_odat} not found\n";
	exit 1;
}
if (! -s $in_ompp) {
	logmsg error, $script_name, "Framework parameter module ${in_ompp} not found\n";
	exit 1;
}
if (! -s $model_props) {
	logmsg error, $script_name, "Model macros ${model_props} not found\n";
	exit 1;
}

my $result = modgen_create_scex $out_scex, $in_odat, $in_ompp, $model_props, @in_dat;

if ($result) {
	logmsg error, $script_name, "failed to create ${out_scex}\n";
}

return $result;
