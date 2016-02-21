# Copyright (c) 2016 OpenM++ contributors
# This code is licensed under the MIT license (see LICENSE.txt for details)

# Script to transform a csv file to dat format

use strict;
use warnings;

my $script_name = 'ompp_csv_dat';
my $script_version = '1.0';

use Getopt::Long::Descriptive;

my ($opt, $usage) = describe_options(
	$script_name.' %o <source-sqlite> <destination-folder>',
	[ 'help|h'    => "print usage message and exit" ],
	[ 'version|v' => "print version and exit" ],
);

if ($opt->version) {
	print $script_name.' version '.$script_version."\n";
	exit 0;
}

if ($opt->help) {
	print $usage->text;
	exit 0;
}

if ($#ARGV != 1) {
	# must have exactly two arguments
	print $usage->text;
	exit -1;
}

my $in_csv = shift @ARGV;
my $out_dat = shift @ARGV;

use common qw(
				warning change error info
				logmsg
		    );

if (! -s $in_csv) {
	logmsg error, $script_name, "Input csv file '${in_csv}' not found\n";
	exit -1;
}

logmsg error, $script_name, "Sorry, this utility is not yet implemented!\n";
exit -1;
