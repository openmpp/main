# Copyright (c) 2016 OpenM++ contributors
# This code is licensed under the MIT license (see LICENSE.txt for details)

# Script to export a SQLite scenario to scex/dat format for Modgen model

use strict;
use warnings;

my $script_name = 'ompp_export_dat';
my $script_version = '1.0';

use Getopt::Long::Descriptive;

my ($opt, $usage) = describe_options(
	$script_name.' %o <input-file> <output-file>',
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

my $db = shift @ARGV;
my $out_dir = shift @ARGV;

use common qw(
				warning change error info
				logmsg
		    );

if (! -s $db) {
	logmsg error, $script_name, "SQLite database '${db}' not found\n";
	exit -1;
}

if (! -d $out_dir) {
	logmsg error, $script_name, "Output directory  '${out_dir}' does not exist\n";
	exit -1;
}

logmsg error, $script_name, "Sorry, this utility is not yet implemented!\n";
exit -1;
