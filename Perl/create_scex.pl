# Copyright (c) 2013-2014 OpenM++
# This code is licensed under MIT license (see LICENSE.txt for details)

# Script to create a Modgen .scex file given ompp framework files and a list of .dat files.

use strict;

my $version = 0.1;

if ( $#ARGV < 3 ) {
	# must have at least 4 arguments
	print "create_scex version $version\n";
	print "usage: create_scex out_scex in_framework_odat in_framework_ompp param1.dat...\n";
	print "  example: create_scex Base.scex Base(Framework).odat ompp_framework.ompp *.dat\n";
	exit 1;
}

# Control verbosity of log output (0, 1, 2)
my $verbosity = 0;

my $script_name = "create_scex";

my $out_scex = shift @ARGV;
my $in_odat = shift @ARGV;
my $in_ompp = shift @ARGV;
my @in_dat = @ARGV;

use common qw(
				warning change error info
				modgen_create_scex
		    );
# Check for compile errors, and bail if any.
if ($@) {
	print $@;
	exit 1;
}

if (!-s $in_odat) {
	logmsg error, $script_name, "Framework parameter file ${in_odat} not found\n";
	exit 1;
}
if (!-s $in_ompp) {
	logmsg error, $script_name, "Framework parameter module ${in_ompp} not found\n";
	exit 1;
}

my @in_dat;
for my $arg (@ARGV) {
	push @in_dat, glob $arg;
}

my $result = modgen_create_scex $out_scex, $in_odat, $in_ompp, @in_dat;

return $result;
