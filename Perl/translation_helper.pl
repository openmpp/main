# Copyright (c) 2013-2016 OpenM++ Contributors
# This code is licensed under the MIT license (see LICENSE.txt for details)

# Script to find translatable strings in source code, e.g. omc

use strict;

my @files = glob(join(' ', @ARGV));

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

print "[FR]\n";
foreach my $string ( sort keys %strings ) {
   #print %strings{$string}.": ".$string."\n";
   print '"'.$string.'" = '."\n";
   print '"'.$string.'"'."\n\n";
}