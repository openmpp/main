# Copyright (c) 2013-2014 OpenM++
# This code is licensed under MIT license (see LICENSE.txt for details)

# Script to copy invariant files used in build to model folders 

use strict;

use Cwd qw(getcwd);

my $om_root = $ENV{'OM_ROOT'};
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

my $verbose = 0;

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

if (! -d $models_root) {
	logmsg error, "Folder ${models_root} not found";
	exit 1;
}

my @models; # list of models (without path)

if ($#ARGV >= 0) {
	# model folders listed explicitly on command line
	@models = @ARGV;
}
else {
	# no models listed on command line
	# Create model list by identifying all model subdirectories
	my $save_dir = getcwd();
	chdir $models_root;
	my @paths = glob "*/code";
	chdir $save_dir;
	for my $path (@paths) {
		$path =~ s/\/.*//;
		push @models, $path;
	}
	logmsg info, "models: ".join(',',@models) if $verbose > 1;
}

# Activate option to remove target file (perhaps not necessary - doc'n unclear)
local $File::Copy::Recursive::RMTrgFil = 1;

MODEL:
for my $model (@models) {

	logmsg info, "checking model=$model" if $verbose > 0;

	if (! -d "${models_root}/${model}/code") {
		logmsg error, "${model}: Not a model folder";
		next MODEL;
	}

	# Invariant files to align if present in each model
	# First element is the file, second is the model containing the definitive version to propagate.
	# As a special case, if MODEL is in first element, it is replaced by the source and destination model
	my @invariant_list = (
		"makefile",                     "NewCaseBased",

		"MODEL-modgen.sln",             "NewCaseBased",
		"modgen/Model.vcxproj",         "NewCaseBased",
		"modgen/Model.vcxproj.filters", "NewCaseBased",

		"MODEL-ompp.sln",               "NewCaseBased",
		"ompp/Model.vcxproj",           "NewCaseBased",
		"ompp/Model.vcxproj.filters",   "NewCaseBased",

		"code/modgen_case_based.mpp",   "NewCaseBased",
		"code/case_based.h",            "NewCaseBased",

		"code/modgen_time_based.mpp",   "NewTimeBased",
	);
	
	my $any_propagated = 0;
	for( my $j = 0; $j <= $#invariant_list; $j += 2) {
		my $invariant_file = @invariant_list[$j];
		my $invariant_model = @invariant_list[$j + 1];
		my $src = "${om_root}/models/${invariant_model}/${invariant_file}";
		my $dst = "${models_root}/${model}/${invariant_file}";
		# handle special case where string 'MODEL' is in the src file name
		$src =~ s/MODEL/${invariant_model}/;
		$dst =~ s/MODEL/${model}/;
		-e "${src}" || die "Not found: ${src}";
		logmsg info, "checking src=$src" if $verbose > 1;
		logmsg info, "checking dst=$dst" if $verbose > 1;
		if (-f "${dst}" && ${model} ne ${invariant_model}) {
			logmsg info, "checking src=$src and dst=$dst" if $verbose > 0;
			if (compare ${src}, ${dst}) {
				logmsg info, ${model}, "Updating ${invariant_file} from ${invariant_model}";
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