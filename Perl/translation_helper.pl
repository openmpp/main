# Copyright (c) 2013-2016 OpenM++ Contributors
# This code is licensed under the MIT license (see LICENSE.txt for details)

# Script to find translatable strings in source code, e.g. omc

# This script is a WIP.
# Run from ompp/Perl
# hard-coded to omc
# TODO:
# - generalize to other directories and patterns
# - compare to existing translatable_baseline.txt

# Use:
#
# The output file translatable_current.txt is used to maintain translation moving forward.
# It contains the strings in the source code in the source code language, e.g. English
# After translating all strings, rename this file to translatable_baseline.txt and retain it.
# When updating translation, the up-to-date translatable_current.txt produced by this script
# can be compared to translatable_baseline.txt to identify the changes
# - modifications, deletions, additions -
# required to bring the translation file omc.message.ini up to date.
# After the translation file omc.message.ini is up-to-date, replace the old translatable_baseline.txt
# with the up-to-date content in translatable_current.txt, as a new baseline for the next translation.
#
# The file translatable_starting.ini provides a convenient starting point for translating
# all of the content into ini file format.  It can also be used for maintenance, by copying
# selected content to omc.message.ini

use strict;

my $dir = "../openm/omc";
#my @files = glob(join(' ', @ARGV));
my @files = glob("${dir}/*.cpp ${dir}/*.[ly]");


# Find all translatable strings in source code in $dir
my %strings;
for my $file (@files) {
	#print "FILE=${file}\n";
	open FILE, "<".$file;
	while (<FILE>) {
		chomp;
		my $line = $_;
		
		while ($line =~ /(\w+)\(\"([^\"]*)\"\)/g) {
			my $func = $1;
			my $match = $2;
			if ($func =~ /^LT|logMsg|logFormatted$/) {
				$strings{$match} = $func;
			}
		}
	}
	close FILE;
}

# Create the file of 'current' translatable strings
my $out_current = $dir."/translatable_current.txt";
open OUT_CURRENT, ">".$out_current;

# Create a starting file for translation, in ini format
my $out_starting = $dir."/translatable_starting.ini";
open OUT_STARTING, ">".$out_starting;

#Create a starting file for translation based on current translatable strings
print OUT_STARTING "[FR]\n\n";
foreach my $string ( sort keys %strings ) {
   #print %strings{$string}.": ".$string."\n";
   
   print OUT_CURRENT '"'.$string.'"'."\n";
   
   print OUT_STARTING '"'.$string.'" = "'.$string.'"'."\n\n";
}


close OUT_CURRENT;
close OUT_STARTING