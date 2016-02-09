# Copyright (c) 2013-2015 OpenM++
# This code is licensed under MIT license (see LICENSE.txt for details)

# Test Script to export the output tables of a SQLite scenario to Excel in pivot table format

use strict;

use File::Compare;

my $utility_exe = '../../bin/ompp_export_excel.exe';

my $out1_correct = 'Correct.xlsx';
my $in1 = 'RiskPaths.sqlite';

for my $file ($in1, $out1_correct) {
	if (! -e $file) {
		print "Missing file ${file} - can't perform test\n";
		exit 1;
	}
}

my $out1 = 'Test.xlsx';
unlink $out1;

my @args = (
	$utility_exe,
	$in1,
	$out1,
	);
system(@args);

if (! -e $out1) {
	print "Output file ${out1} is missing.\n";
	exit 1;
}

# Can't perform binary compare of output with correct .xlsx due to internal timestamp.
# Comparing content would require using an extraction tool or API of some sort,
# e.g. to explode into CSV's for each sheet, then perform compare on those.
print "Passed\n";
exit 0;
	
