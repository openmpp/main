# Copyright (c) 2013-2015 OpenM++
# This code is licensed under MIT license (see LICENSE.txt for details)

# Script to prepare files for binary deployment

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

for my $subdir ('bin', 'include', 'lib', 'props', 'R', 'sql', 'use') {
	-d "${om_root}/${subdir}" or die;
}

my $deploy_dir = "deploy_windows";
remove_tree $deploy_dir;
mkdir $deploy_dir || die;

my $subdir;
my @files;

# bin
$subdir = 'bin';
@files = (
	'create_scex.exe',
	'excel_export.exe',
	'omc.exe',
	'sqlite3.exe',
	);
mkdir "${deploy_dir}/${subdir}" or die;
for my $file (@files) {
	copy "${om_root}/${subdir}/${file}", "${deploy_dir}/${subdir}/${file}" or die "Failed to copy ${subdir}/${file}";
}

# include
$subdir = 'include';
dircopy $subdir, "${deploy_dir}/${subdir}" || die;

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

# props
$subdir = 'props';
dircopy $subdir, "${deploy_dir}/${subdir}" || die;

# R
$subdir = 'R';
dircopy $subdir, "${deploy_dir}/${subdir}" || die;

# sql
$subdir = 'sql';
dircopy $subdir, "${deploy_dir}/${subdir}" || die;

# use
$subdir = 'use';
dircopy $subdir, "${deploy_dir}/${subdir}" || die;


