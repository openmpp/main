# Copyright (c) 2013 OpenM++
# This code is licensed under MIT license (see LICENSE.txt for details)

# Create openM++ database for a subsequent run of a model
# Run this script from the /openm/test_models folder
# Argument is the name of a model, located in a sister directory of test_models, e.g. Alpha1

# Input files:
#   Alpha1_inputs.pl - input parameter values for Alpha1
# 
# Creates a new openM++ database, which  has a fixed model_id of 11.


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