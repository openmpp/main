# Copyright (c) 2013-2014 OpenM++
# This code is licensed under MIT license (see LICENSE.txt for details)

# Script to copy invariant files used in build to model folders 

use strict;

use File::Copy;
use File::Copy::Recursive qw(dircopy);
use File::Compare;
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

	# Invariant files to align if present in model
	# First element is the file, second is the model containing the definitive version to propagate.
	my @invariant_list = (
		"makefile",                     "WizardCaseBased",
		"modgen/Model.vcxproj",         "WizardCaseBased",
		"modgen/Model.vcxproj.filters", "WizardCaseBased",
		"ompp/Model.vcxproj",           "WizardCaseBased",
		"ompp/Model.vcxproj.filters",   "WizardCaseBased",
		"code/modgen_case_based.mpp",   "WizardCaseBased",
		"code/case_based.h",            "WizardCaseBased",
		"code/modgen_time_based.mpp",   "WizardTimeBased",
		"code/microdata_csv.h",         "OzProj",
	);
	
	my $any_propagated = 0;
	for( my $j = 0; $j <= $#invariant_list; $j += 2) {
		my $invariant_file = @invariant_list[$j];
		my $invariant_dir = @invariant_list[$j + 1];
		my $src = "${invariant_dir}/${invariant_file}";
		my $dst = "${model_dir}/${invariant_file}";
		-e "${src}" || die "Not found: ${src}";
		if (-f "${dst}" && ${model_dir} ne ${invariant_dir}) {
			if (compare ${src}, ${dst}) {
				logmsg info, ${model_dir}, "Updating ${invariant_file} from ${invariant_dir}";
				copy ${src}, ${dst};
				$any_propagated = 1;
			}
		}
	}
	if ($any_propagated) {
		# put spacing line in to make more easily visible that a file was updated.
		logmsg info, " ";
	}

}