# Copyright (c) 2013-2014 OpenM++
# This code is licensed under MIT license (see LICENSE.txt for details)

# Script to export the output tables of a SQLite scenario to Excel in pivot table format

use strict;

use File::Compare;

my $create_scex_exe = '../../bin/ompp_create_scex.exe';

my $correct_scex = 'Correct.scex';
my $in_odat = 'Base(Framework).odat';
my $in_ompp = 'ompp_framework.ompp';

for my $file ($correct_scex, $in_odat, $in_ompp) {
	if (! -e $file) {
		print "Missing file ${file} - can't perform test\n";
		exit 1;
	}
}

my $out_scex = 'Base.scex';
unlink $out_scex;

my @args = (
	$create_scex_exe,
	$out_scex,
	$in_odat,
	$in_ompp,
	'*.dat'
	);
system(@args);

if (! -e $out_scex) {
	print "Output file ${out_scex} is missing.\n";
	exit 1;
}
	
if (compare($correct_scex, $out_scex) != 0) {
	print "Output file ${out_scex} differs from correct file ${correct_scex}\n";
	exit 1;
}
else {
	print "Passed\n";
	exit 0;
}
	
