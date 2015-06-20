# Copyright (c) 2013-2014 OpenM++
# This code is licensed under MIT license (see LICENSE.txt for details)

# Script to create a modify Modgen output files for cross-compatible models

use strict;
use File::Temp qw(tempfile);
use File::Copy;


my $version = 0.1;

if ( $#ARGV != 0 ) {
	# must have 1 argument
	print "patch_modgen_outputs version $version\n";
	print "usage: patch_modgen_outputs dir\n";
	print "  example: patch_modgen_outputs Alpha1/modgen/src\n";
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

my $model_h = "${src_dir}/MODEL.H";
if (!-f $model_h) {
	logmsg error, $script_name, "File ${model_h} not found\n";
	exit 1;
}

(my $fh, my $filename) = tempfile();

# Insert a line in model.h to include custom_early.h before actors.h
open MODEL_H, "<".$model_h;
while (<MODEL_H>) {
	chomp;
	my $line = $_;
	if ($line =~ /actors/) {
		print $fh "#include \"custom_early.h\" // Inserted by patch_modgen_outputs after Modgen compilation.\n";
	}
	print $fh $line."\n";
}
close $fh;
close MODEL_H;

copy $filename, $model_h;
unlink $filename;

exit 0;
