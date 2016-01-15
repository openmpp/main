# Copyright (c) 2013-2015 OpenM++
# This code is licensed under MIT license (see LICENSE.txt for details)

# Script to prepare files for binary deployment

# Should be re-factored to use subroutines rather than all that repetitive copying...

# Check for and process -no64 flag (makes a smaller package)
my $no64 = 0;
if ($#ARGV == 0 && $ARGV[0] eq '-no64') {
	$no64 = 1;
	shift @ARGV;
}

do 'build_tools.pl' || die;

use Capture::Tiny qw/capture tee capture_merged tee_merged/;
use File::Copy;
use File::Copy::Recursive qw(dircopy);
use File::Path qw(make_path remove_tree);
use POSIX qw(strftime);

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
for my $subdir ('bin', 'include', 'openm', 'lib', 'props', 'R', 'sql', 'use', 'models') {
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

# openm/libopenm
$subdir = 'openm/libopenm';
dircopy $subdir, "${deploy_dir}/${subdir}" || die;

# props
$subdir = 'props';
dircopy $subdir, "${deploy_dir}/${subdir}" || die;

# R
$subdir = 'R';
dircopy $subdir, "${deploy_dir}/${subdir}" || die;

# Perl
#$subdir = 'Perl';
#dircopy $subdir, "${deploy_dir}/${subdir}" || die;

# sql
$subdir = 'sql';
dircopy $subdir, "${deploy_dir}/${subdir}" || die;

# use
$subdir = 'use';
dircopy $subdir, "${deploy_dir}/${subdir}" || die;

# models
mkdir "${deploy_dir}/models" or die;

$subdir = 'models/microdata';
dircopy $subdir, "${deploy_dir}/${subdir}" || die;

$subdir = 'models';
@files = (
	'.gitignore',
	);
for my $file (@files) {
	copy "${om_root}/${subdir}/${file}", "${deploy_dir}/${subdir}/${file}" or die "Failed to copy ${subdir}/${file}";
}

# Selected models
@models = (
	'IDMM',
	'OzProj',
	'OzProjGen',
	'RiskPaths',
	'WizardCaseBased',
	'WizardTimeBased',
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

my $seven_zip = "C:\\Program Files\\7-Zip\\7z.exe";
if (! -e $seven_zip) {
	$seven_zip = "C:\\Program Files (x86)\\7-Zip\\7z.exe";
}

chdir "${om_root}/deploy_windows" or die;
if (-e $seven_zip) {
	my $time_stamp = strftime "%Y%m%d", localtime;
	my $archive = "openmpp_win_${time_stamp}.zip";
	print "Construct archive $archive\n";
	unlink $archive;
	($stdout, $stderr) = capture {
		my @args = ("$seven_zip", "a", "-tzip", '"'.$archive.'"',
			'".\\*"',
			);
		system(@args);
	};
	($last_line) = $stdout =~ /\n(.*)$/;
	print "  7zip: |$last_line|\n" if ( $last_line ne "Everything is Ok" );
}
else {
	print "7-zip not found - .zip archive not created\n";
}
chdir $om_root;
