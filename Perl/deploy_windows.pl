# Copyright (c) 2013-2016 OpenM++
# This code is licensed under MIT license (see LICENSE.txt for details)

# Script to prepare files for binary deployment

# Should be re-factored to use subroutines rather than all that repetitive copying...

use strict;
use warnings;

my $script_name = "deploy_windows";
my $script_version = '1.1';

use Getopt::Long::Descriptive;

my ($opt, $usage) = describe_options(
	$script_name.' %o model...',
	[ 'help|h'    => 'print usage message and exit' ],
	[ 'version|v' => 'print version and exit' ],
	[ 'no64' => 'do not include 64-bit components' ],
	[ 'mpi' => 'deploy MPI build' ],
);

if ($opt->version) {
	print $script_name.' version '.$script_version."\n";
	exit 0;
}

if ($opt->help) {
	print $usage->text;
	exit 0;
}

# Check for and process -no64 flag (makes a smaller package)
my $no64 = 0;
if ($opt->no64) {
	$no64 = 1;
}

# Check for --mpi flag: build models and libs with MPI
my $doMpi = "no";
my $mpiSuffix = "";
if ($opt->mpi) {
	$doMpi = "yes";
    $mpiSuffix = "_mpi";
}

chdir "..";
use Cwd qw(getcwd);
my $om_root = getcwd;

print "Build tools and utilities\n";
chdir 'Perl';
do 'build_tools.pl' || die;
chdir $om_root;

# Deployed models
my @models = (
	'IDMM',
	'OzProj',
	'OzProjGen',
	'RiskPaths',
	'NewCaseBased',
	'NewCaseBased_bilingual',
	'NewTimeBased',
	'NewTimeBased_bilingual',
	);

print "Build deployed models\n";
chdir 'Perl';
system 'perl', 'test_models.pl', '--nomodgen', '--noclean', "--ompp_deploy_mpi=${doMpi}", @models;
chdir $om_root;

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

# Preliminary check of source folder structure
for my $subdir ('licenses', 'bin', 'include', 'openm', 'lib', 'props', 'R', 'Excel', 'sql', 'use', 'models', 'ompp-ui') {
	-d "${om_root}/${subdir}" or die "Missing directory ${om_root}/${subdir}";
}

my $deploy_dir = "deploy_windows";

print "Removing previous deployment directory ${deploy_dir}\n";
remove_tree $deploy_dir;
mkdir $deploy_dir or die "Unable to remove deployment directory ${deploy_dir}";

my $subdir;
my @files;

print "Copying files and folders to ${deploy_dir}\n";

# ompp root
$subdir = '.';
@files = (
	'README_win.txt',
	'README.txt',
	'LICENSE.txt',
	'AUTHORS.txt',
	);
for my $file (@files) {
	copy "${om_root}/${subdir}/${file}", "${deploy_dir}/${subdir}/${file}" or die "Failed to copy ${subdir}/${file}";
}

# licenses
$subdir = 'licenses';
dircopy $subdir, "${deploy_dir}/${subdir}" || die;

# bin
$subdir = 'bin';
@files = (
	'omc.exe',
	'omc.message.ini',
	'ompp_create_scex.exe',
	'ompp_export_csv.exe',
	'ompp_export_excel.exe',
	'README_win.txt',
	'modgen_export_csv.exe',
	'dbcopy.exe',
    'oms.exe',
	'patch_modgen11_outputs.exe',
	'patch_modgen12_outputs.exe',
	'patch_modgen12.1_outputs.exe',
	'sqlite3.exe',
    'ompp_ui.bat',
	);
mkdir "${deploy_dir}/${subdir}" or die;
for my $file (@files) {
	if ($no64 && $file =~ /64/) {
		next;
	}
	copy "${om_root}/${subdir}/${file}", "${deploy_dir}/${subdir}/${file}" or die "Failed to copy ${subdir}/${file}";
}

# dbcopy and oms: skip if GOPATH not defined
#
if ("$ENV{GOPATH}" eq "") {
	print "Skip dbcopy and oms: GOPATH is empty\n";
}
else {
    # copy dbcopy.exe
	my $dbcopy_exe = "$ENV{GOPATH}/bin/dbcopy.exe";
	if (-e "${dbcopy_exe}") {
		copy "${dbcopy_exe}", "${deploy_dir}/${subdir}/dbcopy.exe" or die "Failed to copy ${dbcopy_exe}";
	}
	else {
		print "Skip dbcopy: it does not exist\n";
	}
	
    # copy oms.exe
	my $oms_exe = "$ENV{GOPATH}/bin/oms.exe";
	if (-e "${oms_exe}") {
		copy "${oms_exe}", "${deploy_dir}/${subdir}/oms.exe" or die "Failed to copy ${oms_exe}";
	}
	else {
		print "Skip oms: it does not exist\n";
	}
	
	# dbcopy and oms source code
	my $go_src_dir = "$ENV{GOPATH}/src/go.openmpp.org";
	dircopy "$go_src_dir/dbcopy", "${deploy_dir}/go.openmpp.org/dbcopy" || die;
	dircopy "$go_src_dir/ompp", "${deploy_dir}/go.openmpp.org/ompp" || die;
	dircopy "$go_src_dir/licenses", "${deploy_dir}/licenses" || die;
	dircopy "$go_src_dir/oms", "${deploy_dir}/go.openmpp.org/oms" || die;
}

# lib
$subdir = 'lib';
mkdir "${deploy_dir}/${subdir}" or die;

my @lib_names = ("libopenm", "libsqlite");
my @lib_suffixes = ("", "D", "64", "64D");

for my $ln (@lib_names) {
	for my $sf (@lib_suffixes) {
		if ($no64 && $sf =~ /64/) {
			next;
		}
		my $file = "${ln}${sf}.lib";
		if (${ln} eq "libopenm") {
			$file = "${ln}${sf}${mpiSuffix}.lib";   # if MPI then: libopenm_mpi.lib
		}
		copy "${om_root}/${subdir}/${file}", "${deploy_dir}/${subdir}/${file}" or die "Failed to copy ${subdir}/${file}";
	}
}

# include
$subdir = 'include';
dircopy $subdir, "${deploy_dir}/${subdir}" || die;

# openm (omc can be excluded, the rest is required for debug)
$subdir = 'openm';
dircopy "${subdir}/libsqlite", "${deploy_dir}/${subdir}/libsqlite" || die;
dircopy "${subdir}/libopenm", "${deploy_dir}/${subdir}/libopenm" || die;
dircopy "${subdir}/omc", "${deploy_dir}/${subdir}/omc" || die;
copy "${om_root}/${subdir}/openm.build.props", "${deploy_dir}/${subdir}" || die;
copy "${om_root}/${subdir}/openm.sln", "${deploy_dir}/${subdir}" || die;

# props
$subdir = 'props';
dircopy $subdir, "${deploy_dir}/${subdir}" || die;

# R
$subdir = 'R';
dircopy $subdir, "${deploy_dir}/${subdir}" || die;

# Excel
$subdir = 'Excel';
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

# models bin: pre-built models exe, message.ini database
my $models_bin = "${deploy_dir}/models/bin";
mkdir "${models_bin}" or die;

# models sql: models sql scripts used to create database
my $models_sql = "${deploy_dir}/models/sql";
mkdir "${models_sql}" or die;

$subdir = 'models/microdata';
dircopy $subdir, "${deploy_dir}/${subdir}" || die;

$subdir = 'models';
@files = (
	'.gitignore',
	);
for my $file (@files) {
	copy "${om_root}/${subdir}/${file}", "${deploy_dir}/${subdir}/${file}" or die "Failed to copy ${subdir}/${file}";
}

for my $model (@models) {
	my $model_dir = "models/${model}";
	-d $model_dir or print "folder for model ${model} not found.\n";
	
	mkdir "${deploy_dir}/${model_dir}" or die;
	
	# code
	$subdir = "${model_dir}/code";
	dircopy $subdir, "${deploy_dir}/${subdir}" || die;
	
	# parameters
	$subdir = "${model_dir}/parameters";
	dircopy $subdir, "${deploy_dir}/${subdir}" || die;
	
	# solution files
	$subdir = "${model_dir}";
	@files = glob("${subdir}/*.sln");
	for my $file (@files) {
		copy "${om_root}/${file}", "${deploy_dir}/${subdir}" or die "Failed to copy ${subdir}/${file}";
	}
	
	# modgen project files
	$subdir = "${model_dir}/modgen";
	mkdir "${deploy_dir}/${subdir}" or die;
	@files = glob("${subdir}/*.vcxproj ${subdir}/*.vcxproj.filters ${subdir}/*.props");
	for my $file (@files) {
		copy "${om_root}/${file}", "${deploy_dir}/${subdir}" or die "Failed to copy ${subdir}/${file}";
	}
	
	# ompp project files
	$subdir = "${model_dir}/ompp";
	mkdir "${deploy_dir}/${subdir}" or die;
	@files = glob("${subdir}/*.vcxproj ${subdir}/*.vcxproj.filters ${subdir}/*.props");
	for my $file (@files) {
		copy "${om_root}/${file}", "${deploy_dir}/${subdir}" or die "Failed to copy ${subdir}/${file}";
	}
	
	# copy model.exe or model_mpi.exe
	my $model_exe = "${model_dir}/ompp/bin/${model}${mpiSuffix}.exe";
	copy "${model_exe}", "${models_bin}" 
		or die "Failed to copy ${model_exe}";
		
	# model db.sqlite
	copy "${model_dir}/output/${model}.sqlite", "${models_bin}/${model}.sqlite" 
		or die "Failed to copy ${model_dir}/output/${model}.sqlite";
	
	# model message.ini
	if (-e "${model_dir}/ompp/bin/${model}.message.ini") {
		copy "${model_dir}/ompp/bin/${model}.message.ini", "${models_bin}" 
			or die "Failed to copy ${model_dir}/ompp/bin/${model}.message.ini";
	}

	# model sql files
	$subdir = "${model_dir}/ompp/src";
	@files = glob("${subdir}/*.sql");
	for my $file (@files) {
		copy "${om_root}/${file}", "${models_sql}" or die "Failed to copy ${subdir}/${file}";
	}
}

# modelOne
$subdir = "models/modelOne";
mkdir "${deploy_dir}/${subdir}" or die;
@files = glob("${subdir}/*.*");
for my $file (@files) {
	copy "${om_root}/${file}", "${deploy_dir}/${subdir}" or die "Failed to copy ${subdir}/${file}";
}
# get rid of VS-generated files
unlink "${deploy_dir}/${subdir}/modelOne.sdf";
unlink "${deploy_dir}/${subdir}/modelOne.VC.db";

$subdir = "models/modelOne/ompp";
mkdir "${deploy_dir}/${subdir}" or die;
@files = glob("${subdir}/*.vcxproj ${subdir}/*.vcxproj.filters");
for my $file (@files) {
	copy "${om_root}/${file}", "${deploy_dir}/${subdir}" or die "Failed to copy ${subdir}/${file}";
}

# modelOne executable, sql scripts and if exists ini-file and db.sqlite
my $model_dir = "${om_root}/models/modelOne";

copy "${model_dir}/ompp/bin/modelOne${mpiSuffix}.exe", "${models_bin}" 
	or die "Failed to copy ${model_dir}/ompp/bin/modelOne${mpiSuffix}.exe";

@files = glob("${model_dir}/*.sql");
for my $file (@files) {
	copy "${file}", "${models_sql}" or die "Failed to copy ${file}";
}
	
my $model_ini = "${model_dir}/ompp/bin/modelOne.ini";
if (-e $model_ini) {
	copy "${model_ini}", "${models_bin}" or die "Failed to copy ${model_ini}";
}

my $msg_ini = "${model_dir}/ompp/bin/modelOne.message.ini";
if (-e $msg_ini) {
	copy "${msg_ini}", "${models_bin}" or die "Failed to copy ${msg_ini}";
}

my $model_sqlite = "${model_dir}/ompp/modelOne.sqlite";
if (-e $model_sqlite) {
	copy "${model_sqlite}", "${models_bin}" or die "Failed to copy ${model_sqlite}";
}

# ompp-ui
dircopy "ompp-ui/dist", "${deploy_dir}/html" || die "Failed to copy ompp-ui/dist";
mkdir "${deploy_dir}/log" or die;
mkdir "${deploy_dir}/models/log" or die;

# ompp-ui sources
mkdir "${deploy_dir}/ompp-ui" or die;

@files = glob("ompp-ui/* ompp-ui/.*");
for my $file (@files) {
    if (! -d ${file}) {
        copy "${om_root}/${file}", "${deploy_dir}/${file}" or die "Failed to copy ${om_root}/${file}";
    }
    else {
        if ($file ne "ompp-ui/." && $file ne "ompp-ui/.." && $file ne "ompp-ui/dist" && $file ne "ompp-ui/node_modules") {
            dircopy "${om_root}/${file}", "${deploy_dir}/${file}" || die "Failed to copy ${om_root}/${file}";
        }
    }
}

# Create zip archive
my $seven_zip = "C:\\Program Files\\7-Zip\\7z.exe";
if (! -e $seven_zip) {
	$seven_zip = "C:\\Program Files (x86)\\7-Zip\\7z.exe";
}

chdir "${om_root}/deploy_windows" or die;
if (-e $seven_zip) {
	my $time_stamp = strftime "%Y%m%d", localtime;
	my $archive = "openmpp_win_${time_stamp}.zip";
	print "Constructing archive $archive\n";
	unlink $archive;
	(my $stdout, my $stderr) = capture {
		my @args = ("$seven_zip", "a", "-tzip", '"'.$archive.'"',
			'".\\*"',
			);
		system(@args);
	};
	(my $last_line) = $stdout =~ /\n(.*)$/;
	print "  7zip: |$last_line|\n" if ( $last_line ne "Everything is Ok" );
}
else {
	print "7-zip not found - .zip archive not created\n";
}
chdir $om_root;
