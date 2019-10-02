# Copyright (c) 2013-2015 OpenM++
# This code is licensed under MIT license (see LICENSE.txt for details)

# Script to clean all temporary and output files in ompp, ompp-linux, ompp-mac, and modgen

# usage: perl clean_models.pl models
# If models is not specified, all models will be processed.

chdir "../models" || die "Invoke test_models from Perl folder";

use File::Path qw(make_path remove_tree);

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

for my $model_dir (@model_dirs) {
	print "Cleaning ${model_dir}\n";
	for my $subdir ("src", "build", "bin") {
		my @temp_dirs = glob("${model_dir}/*/${subdir}");
		
		for my $temp_dir (@temp_dirs) {
			print "  removing ${temp_dir}\n";
			remove_tree $temp_dir;
		}
	}
}