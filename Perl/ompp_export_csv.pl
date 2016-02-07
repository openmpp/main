# Copyright (c) 2016 OpenM++ contributors
# This code is licensed under the MIT license (see LICENSE.txt for details)

# Script to export the output tables of a SQLite scenario to CSV format

use strict;
use warnings;

my $script_name = 'ompp_export_csv';
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

my $db = shift @ARGV;
my $out_dir = shift @ARGV;

use common qw(
				warning change error info
				logmsg
				ompp_tables_to_csv
		    );

if (! -s $db) {
	logmsg error, $script_name, "SQLite database '${db}' not found\n";
	exit -1;
}

if (-d $out_dir || -f $out_dir) {
	logmsg error, $script_name, "Output directory for csv files '${out_dir}' must not already exist\n";
	exit -1;
}

my $failure = ompp_tables_to_csv($db, $out_dir);
if ($failure) {
	logmsg error, $script_name, "Failed to extract tables\n";
	exit -1
};



