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
		print $fh $line."\n";
	}
	
	# append definitions of mind, etc.
	my $code = <<'END_CODE';
	
//
// The following code was inserted by patch_modgenXX_outputs after Modgen compilation:
//

// The following seems required in some versions of VS 2019 when requesting C++17
#include <set>

// minimum of two doubles
inline double mind(double a, double b)
{
    return (b < a ? b : a);
}

// maximum of two doubles
inline double maxd(double a, double b)
{
    return (a < b ? b : a);
}

// clamped value of a double
inline double clampd(double v, double lo, double hi)
{
    return (v < lo ? lo : v > hi ? hi : v);
}

// minimum of two ints
inline int mini(int a, int b)
{
    return (b < a ? b : a);
}

// maximum of two ints
inline int maxi(int a, int b)
{
    return (a < b ? b : a);
}

// clamped value of an int
inline int clampi(int v, int lo, int hi)
{
    return (v < lo ? lo : v > hi ? hi : v);
}
END_CODE

	print $fh $code;
	
	close $fh;
	close MODEL_H;
	copy $filename, $model_h;
	unlink $filename;
}

exit 0;
