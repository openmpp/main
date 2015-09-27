# Copyright (c) 2013-2015 OpenM++
# This code is licensed under MIT license (see LICENSE.txt for details)

# Script to prepare files for binary deployment

# SHould be re-factored to use subroutines rather than all that repetitve copying...

# Check for and process -no64 flag
my $no64 = 0;
if ($#ARGV == 0 && $ARGV[0] eq '-no64') {
	$no64 = 1;
	shift @ARGV;
}


use File::Copy;
use File::Copy::Recursive qw(dircopy);
use File::Path qw(make_path remove_tree);

#####################
# Common settings
#####################

# Verbosity of report:
#  0 - errors only
#  1 - timing and success
#  2 - chatty
my $verbosity = 3;

chdir "..";
use Cwd qw(getcwd);
my $om_root = getcwd;

# Preliminary check of source folder structure
for my $subdir ('bin', 'include', 'lib', 'props', 'R', 'sql', 'use', 'models') {
	-d "${om_root}/${subdir}" or die "Missing directory ${om_root}/${subdir}";
}

my $deploy_dir = "deploy_windows";
remove_tree $deploy_dir;
mkdir $deploy_dir or die "Unable to remove deployment directory ${deploy_dir}";

my $subdir;
my @files;

# bin
$subdir = 'bin';
@files = (
	'omc.exe',
	'sqlite3.exe',
	'create_scex.exe',
	'excel_export.exe',
	'patch_modgen11_outputs.exe',
	'patch_modgen12_outputs.exe',
	);
mkdir "${deploy_dir}/${subdir}" or die;
for my $file (@files) {
	copy "${om_root}/${subdir}/${file}", "${deploy_dir}/${subdir}/${file}" or die "Failed to copy ${subdir}/${file}";
}

# lib
$subdir = 'lib';
@files = (
	'libopenm.lib',
	'libopenmD.lib',
	'libopenm64.lib',
	'libopenm64D.lib',
	'libsqlite.lib',
	'libsqliteD.lib',
	'libsqlite64.lib',
	'libsqlite64D.lib',
	);
mkdir "${deploy_dir}/${subdir}" or die;
for my $file (@files) {
	if ($no64 && $file =~ /64/) {
		next;
	}
	copy "${om_root}/${subdir}/${file}", "${deploy_dir}/${subdir}/${file}" or die "Failed to copy ${subdir}/${file}";
}

# include
$subdir = 'include';
dircopy $subdir, "${deploy_dir}/${subdir}" || die;

# props
$subdir = 'props';
dircopy $subdir, "${deploy_dir}/${subdir}" || die;

# R
$subdir = 'R';
dircopy $subdir, "${deploy_dir}/${subdir}" || die;

# Perl
$subdir = 'Perl';
dircopy $subdir, "${deploy_dir}/${subdir}" || die;

# sql
$subdir = 'sql';
dircopy $subdir, "${deploy_dir}/${subdir}" || die;

# use
$subdir = 'use';
dircopy $subdir, "${deploy_dir}/${subdir}" || die;

# models
mkdir "${deploy_dir}/models" or die;

$subdir = 'models';
@files = (
	'.gitignore',
	);
for my $file (@files) {
	copy "${om_root}/${subdir}/${file}", "${deploy_dir}/${subdir}/${file}" or die "Failed to copy ${subdir}/${file}";
}

# Selected models
@models = (
	'Alpha1',
	'RiskPaths',
	);
for my $model (@models) {
	my $model_dir = "models/${model}";
	mkdir "${deploy_dir}/${model_dir}" or die;
	
	# code
	$subdir = "${model_dir}/code";
	dircopy $subdir, "${deploy_dir}/${subdir}" || die;
	
	# parameters
	$subdir = "${model_dir}/parameters";
	dircopy $subdir, "${deploy_dir}/${subdir}" || die;
	
	# solution files
	$subdir = "${model_dir}";
	my @files = glob("${subdir}/*.sln");
	for my $file (@files) {
		copy "${om_root}/${file}", "${deploy_dir}/${subdir}" or die "Failed to copy ${subdir}/${file}";
	}
	
	# modgen project files
	$subdir = "${model_dir}/modgen";
	mkdir "${deploy_dir}/${subdir}" or die;
	my @files = glob("${subdir}/*.vcxproj ${subdir}/*.vcxproj.filters ${subdir}/*.props");
	for my $file (@files) {
		copy "${om_root}/${file}", "${deploy_dir}/${subdir}" or die "Failed to copy ${subdir}/${file}";
	}
	
	# ompp project files
	$subdir = "${model_dir}/ompp";
	mkdir "${deploy_dir}/${subdir}" or die;
	my @files = glob("${subdir}/*.vcxproj ${subdir}/*.vcxproj.filters ${subdir}/*.props");
	for my $file (@files) {
		copy "${om_root}/${file}", "${deploy_dir}/${subdir}" or die "Failed to copy ${subdir}/${file}";
	}
}
