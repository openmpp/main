# Copyright (c) 2013-2014 OpenM++
# This code is licensed under MIT license (see LICENSE.txt for details)

# Script to copy invariant files used in build to model folders 

use strict;

use File::Copy::Recursive qw(dircopy);
use Cwd qw(getcwd);

# use the common.pm module of shared functions	
use common qw(
				warning change error info
				logmsg
				logerrors
		    );
# Check for compile errors, and bail if any.
if ($@) {
	print $@;
	exit;
}


chdir "../models" || die "Invoke propagate_invariant from Perl folder";
my $models_root = getcwd;
my @model_dirs;

if ($#ARGV >= 0) {
	# model folders listed explicitly on command line
	@model_dirs = @ARGV;
}
else {
	# no models listed on command line
	# Create model list by identifying all model subdirectories
	my @paths = glob "*/code";
	for my $path (@paths) {
		$path =~ s/\/.*//;
		push @model_dirs, $path;
	}
}

# Remove target file (is this necessary?)
local $File::Copy::Recursive::RMTrgFil = 1;

MODEL:
for my $model_dir (@model_dirs) {

	my $model_path = "${models_root}/${model_dir}";
	if (! -d $model_path) {
		logmsg error, "${model_path}: Missing model";
		next MODEL;
	}

	logmsg info, "propagating invariant files to ${model_dir}";
	dircopy "invariant", "${model_dir}";
}