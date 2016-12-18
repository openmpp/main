# Copyright (c) 2013-2016 OpenM++ Contributors
# This code is licensed under the MIT license (see LICENSE.txt for details)

# Script to modify Modgen output files for cross-compatible models

use strict;
use File::Temp qw(tempfile);
use File::Copy;


my $version = 1.1;

if ( $#ARGV != 0 ) {
	# must have 1 argument
	print "patch_modgen12_outputs version $version\n";
	print "usage: patch_modgen12_outputs dir\n";
	print "  example: patch_modgen12_outputs Alpha1/modgen/src\n";
	exit 1;
}

# Control verbosity of log output (0, 1, 2)
my $verbosity = 0;

my $script_name = "patch_modgen_outputs";

# Directory containing Modgen output files
my $src_dir = shift @ARGV;

use common qw(
				warning change error info
		    );
# Check for compile errors, and bail if any.
if ($@) {
	print $@;
	exit 1;
}

if (!-d $src_dir) {
	logmsg error, $script_name, "Directory ${src_dir} does not exist\n";
	exit 1;
}

exit 0;
