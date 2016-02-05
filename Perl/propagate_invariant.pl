# Copyright (c) 2013-2014 OpenM++
# This code is licensed under MIT license (see LICENSE.txt for details)

# Script to copy invariant files used in build to model folders 

use strict;

use Cwd qw(getcwd);

my $om_root = %ENV{'OM_ROOT'};
if ( $om_root eq '') {
	# Try parent directory, assuming this script was invoked in the OM_ROOT/Perl directory
	my $save_dir = getcwd();
	chdir '..';
	$om_root = getcwd();
	chdir $save_dir;
}
else {
	-d $om_root || die "directory not found OM_ROOT='${om_root}'\n";
}


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

# Default location assumes script is invoked from OM_ROOT/Perl.
my $models_root = "../models";

# Check for and process -m <model-folder> option
if ($#ARGV >= 0) {
	if ( $ARGV[0] eq "-m") {
		# discard -m argument
		shift @ARGV;
		# get immediately following value
		$models_root = @ARGV[0];
		shift @ARGV;
	}
}

chdir $models_root || die "Folder ${models_root} not found";

my @model_dirs; # list of models (without path)

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

	# Invariant files to align if present in each model
	# First element is the file, second is the model containing the definitive version to propagate.
	# As a special case, if MODEL is in first element, it is replaced by the source and destination model
	my @invariant_list = (
		"makefile",                     "WizardCaseBased",
		"MODEL-modgen.sln",             "WizardCaseBased",
		"MODEL-ompp.sln",               "WizardCaseBased",
		"modgen/Model.vcxproj",         "WizardCaseBased",
		"modgen/Model.props",           "WizardCaseBased",
		"modgen/Model.vcxproj.filters", "WizardCaseBased",
		"ompp/Model.vcxproj",           "WizardCaseBased",
		"ompp/Model.props",             "WizardCaseBased",
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
		my $src = "${om_root}/models/${invariant_dir}/${invariant_file}";
		my $dst = "${model_dir}/${invariant_file}";
		# handle special case where string 'MODEL' is in the src file name
		$src =~ s/MODEL/${invariant_dir}/;
		$dst =~ s/MODEL/${model_dir}/;
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