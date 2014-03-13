# Copyright (c) 2013 OpenM++
# This code is licensed under MIT license (see LICENSE.txt for details)

# Runs a model using a previously created database.


my $version = 1.0;

if ( $#ARGV+1 != 1 ) {
	print "inputs version $version\n";
	print "inputs modelname\n";
	exit -1;
}

# name of model (from first argument)
my $model_name = $ARGV[0];

# for diagnostics
my $debug = 0;

system "../Debug/Win32/${model_name}.exe -s ${model_name}.ini"