# Copyright (c) 2013-2014 OpenM++
# This code is licensed under MIT license (see LICENSE.txt for details)

# Script to create a modify Modgen output files for cross-compatible models

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

#
# ACTORS.CPP
#
# Patch generated code to fix self_scheduling_int bug
#

{
	my $file_name = 'ACTORS.CPP';
	my $actors_cpp = "${src_dir}/${file_name}";
	if (!-f $actors_cpp) {
		logmsg error, $script_name, "File ${actors_cpp} not found\n";
		exit 1;
	}
	(my $fh, my $filename) = tempfile();
	# Comment the line #define MAIN_MODULE to allow use of pre-compiled headers
	open ACTORS_CPP, "<".$actors_cpp;
	
	while (<ACTORS_CPP>) {
		chomp;
		my $line = $_;

		# Modifications go here.

		print $fh $line."\n";
	}
	close $fh;
	close ACTORS_CPP;
	copy $filename, $actors_cpp;
	unlink $filename;
}
#
# ACTORS.H
#
# Also, build list of entity names in the model for subsequent patches
#
my @entities;

{
	my $file_name = 'ACTORS.H';
	my $actors_h = "${src_dir}/${file_name}";
	if (!-f $actors_h) {
		logmsg error, $script_name, "File ${actors_h} not found\n";
		exit 1;
	}
	(my $fh, my $filename) = tempfile();
	open ACTORS_H, "<".$actors_h;
	
	my $in_classifications = 0;
	my $classification_name = '';
	my $in_ranges = 0;
	my $in_partitions = 0;
	while (<ACTORS_H>) {
		chomp;
		my $line = $_;
		
		if ($line =~ /^class (\w+);$/ ) {
			# build list of entity names
			push @entities, $1;
		}

		# detect classification, range, partition block for subsequent patching
		if ($line eq "// classifications") {
			$in_classifications = 1;
		}
		if ($line eq "// ranges") {
			$in_classifications = 0;
			$in_ranges = 1;
		}
		if ($line eq "// partitions") {
			$in_partitions = 1;
			$in_ranges = 0;
		}
		if ($line eq "class Parameters {") {
			$in_partitions = 0;
		}
		
		if ($in_ranges || $in_partitions) {
			if ($line =~ /^typedef\s+(\w+)\s+(\w+);$/) {
				my $typ = $1;
				my $name = $2;

				# construct _t typedef
				my $line2 = "typedef ${name} ${name}_t; // Inserted by patch_modgenXX_outputs after Modgen compilation.";

				# Append line with _t typedef
				$line = $line."\n".$line2;
				print "patched ${file_name}: $line2\n";
			}
		}
		
		# Print the line (perhaps modified)
		print $fh $line."\n";
	}
	close $fh;
	close ACTORS_H;
	copy $filename, $actors_h;
	unlink $filename;
}

#
# MODEL.H
#

{
	my $file_name = 'MODEL.H';
	my $model_h = "${src_dir}/${file_name}";
	if (!-f $model_h) {
		logmsg error, $script_name, "File ${model_h} not found\n";
		exit 1;
	}
	(my $fh, my $filename) = tempfile();
	open MODEL_H, "<".$model_h;
	while (<MODEL_H>) {
		chomp;
		my $line = $_;
		if ($line eq '#include "custom_early.h"' ) {
			# insert forward declarations for entities before #include "custom_early.h"
			print $fh "// The following block was inserted by patch_modgenXX_outputs after Modgen compilation:\n";
			print $fh "namespace mm {\n";
			for my $entity (@entities) {
				print $fh "\tclass ${entity};\n";
				print $fh "\ttypedef ${entity} * ${entity}_ptr;\n";
			}
			print $fh "};\n";
			print "patched ${file_name}: inserted forward declarations for entities in MODEL.H\n";
		}
		print $fh $line."\n";
	}
	close $fh;
	close MODEL_H;
	copy $filename, $model_h;
	unlink $filename;
}

exit 0;
