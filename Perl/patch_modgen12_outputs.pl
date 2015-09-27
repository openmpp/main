# Copyright (c) 2013-2014 OpenM++
# This code is licensed under MIT license (see LICENSE.txt for details)

# Script to create a modify Modgen output files for cross-compatible models

use strict;
use File::Temp qw(tempfile);
use File::Copy;


my $version = 0.1;

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
		if ($line =~ /poDerivedStates->Set_ssint_age\( int\(value\) \);/) {
			my $original_line = "//".$line." // Modified by patch_modgen12_outputs after Modgen compilation.\n";
			print $fh $original_line;
			$line =~ s/ int/ std::floor/;
			print "patched ${file_name}: ${line}";
		}
		print $fh $line."\n";
	}
	close $fh;
	close ACTORS_CPP;
	copy $filename, $actors_cpp;
	unlink $filename;
}

exit 0;
