# Copyright (c) 2013-2015 OpenM++
# This code is licensed under MIT license (see LICENSE.txt for details)

# Script to build utilities, etc.

my $R_exe = 'C:\Program Files\R\R-3.2.1\bin\R.exe';

# Verbosity of report:
#  0 - errors only
#  1 - timing and success
#  2 - chatty
my $verbosity = 3;

use Capture::Tiny qw/capture tee capture_merged tee_merged/;

# Get ompp root directory
chdir "..";
use Cwd qw(getcwd);
my $om_root = getcwd;
chdir 'Perl' or die;

my $merged; # output from build command
my $retval; # return value from build command
for my $utility ('ompp_export_excel', 'ompp_export_csv', 'ompp_create_scex', 'patch_modgen11_outputs', 'patch_modgen12_outputs') {
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


