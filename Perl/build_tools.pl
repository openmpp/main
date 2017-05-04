# Copyright (c) 2013-2016 OpenM++
# This code is licensed under MIT license (see LICENSE.txt for details)

# Script to build utilities, etc.

my $R_exe = 'C:\Program Files\R\R-3.4.0\bin\R.exe';

# Verbosity of report:
#  0 - errors only
#  1 - timing and success
#  2 - chatty
my $verbosity = 3;

use Capture::Tiny qw/capture tee capture_merged tee_merged/;
use File::Copy;

# Get ompp root directory
chdir "..";
use Cwd qw(getcwd);
my $om_root = getcwd;
chdir 'Perl' or die;

my $merged; # output from build command
my $retval; # return value from build command
for my $utility ('ompp_export_excel', 'ompp_export_csv', 'modgen_export_csv', 'ompp_create_scex', 'patch_modgen11_outputs', 'patch_modgen12_outputs', 'patch_modgen12.1_outputs') {
	my $input = "${utility}.pl";
	my $output = "${om_root}/bin/${utility}.exe";
	print "Building utility ${utility}\n";
	($merged, $retval) = capture_merged {
		my @args = (
			'pp',
			'-o',
			$output,
			$input,
			);
		system(@args);
	};
}

print "Building R package\n";
chdir "${om_root}/R";
($merged, $retval) = capture_merged {
	my @args = (
		$R_exe,
		'CMD',
		'build',
		'openMpp',
		);
	system(@args);
};


# if GOROOT and GOPATH (and others for MinGW) defined then build dbcopy
#
if ("$ENV{GOROOT}" eq "" || "$ENV{GOPATH}" eq "" || "$ENV{CPLUS_INCLUDE_PATH}" eq "" || "$ENV{C_INCLUDE_PATH}" eq "") {
	print "Skip dbcopy build: GOROOT or GOPATH or CPLUS_INCLUDE_PATH or C_INCLUDE_PATH is empty\n";
}
else {
	# For successful go build, must set persistent environment variables for MinGW
	# (normally done through cmd line prompt by executing set_distro_paths before invoking go).
	# Example to set persistent values:
	# setx CPLUS_INCLUDE_PATH C:\MinGW\include;C:\MinGW\include\freetype2
	# setx C_INCLUDE_PATH C:\MinGW\include;C:\MinGW\include\freetype2
	
	print "Building utility dbcopy\n";
	chdir "$ENV{GOPATH}";
	($merged, $retval) = capture_merged {
		my @args = (
			'go',
			'install',
			'go.openmpp.org/dbcopy',
			);
		system(@args);
	};
	if ($retval != 0) {
		print "Build dbcopy failed (output follows:\n$merged\n"
	}
}
copy 'bin/dbcopy.exe', "${om_root}/bin";

chdir "${om_root}/Perl";
