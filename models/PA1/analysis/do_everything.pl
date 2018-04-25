# Simulate all PA1 scenarios and construct csv for Excel PivotTable exploration

my $log_RR_sigma      = 0.10; # For PA: The s.d. of RR with condition (from equation estimation)
my $Alpha_mu          = 0.190000;  # For PA: The mean of the sample of Alpha values (copied from [Alpha.xlsx]Distribution!B3)
my $chosen_cases      = 10000000; # The chosen number of cases for the analysis
my $chosen_replicates = 100; # The chosen number of replicates for the analysis
my $chosen_intrv_prop = 0.80; # The chosen target proportion of the population in the Intervention
my $chosen_treat_eff  = 0.75; # The chosen value for treatment effectiveness

use strict;
use warnings;

use File::Copy; # for copy
use File::Path; # for rmtree
use Text::CSV_XS; # for csv functionality

# Locate ompp installation folder
my $om_root = $ENV{'OM_ROOT'};
$om_root ne '' or die 'environment variable OM_ROOT (ompp install directory) not defined, stopped';

# Locate ompp utility dbcopy.exe
my $dbcopy_exe = "${om_root}/bin/dbcopy.exe";
-f $dbcopy_exe or die "ompp utility $dbcopy_exe not found, stopped";

my $model = 'PA1';
my $model_exe = "../ompp/bin/${model}.exe";
my $model_sqlite = "../output/${model}.sqlite";


####################
# Initialise       #
####################

if (1) {
	print "copy model exe and DB to current directory (analysis directory)\n";
	copy $model_exe, '.' or die "failed to copy ${model_exe}, stopped";
	copy $model_sqlite, '.' or die "failed to copy ${model_sqlite}, stopped";
}

# remove any existing csv output
rmtree "${model}";


####################
# Specify Runs     #
####################

# Runs to simulate (array of hashrefs)
my @runs;

if (1) {
	# Vary number of replicates, with number of cases constant
	my $run = { Cases => $chosen_cases,
				TreatEff => $chosen_treat_eff };
	# Iterate over values for Base (0.00) and the chosen value for the Intervention
	for my $IntrvProp (0.00, $chosen_intrv_prop) {
		$run->{IntrvProp} = $IntrvProp;
		for my $Replicates (4, 8, 10, 16, 20, 32, 64, 100, 200, 400, 800, 1000) {
		#for my $Replicates (10, 16) { # test
			$run->{Replicates} = $Replicates;
			# Activate sources of PA, sequentially
			for my $PAonRR (0, 1) {
				$run->{PAonRR} = $PAonRR;
				for my $PAonAlpha (0, 1) {
					$run->{PAonAlpha} = $PAonAlpha;
					push @runs, { %$run }; # create shallow copy of hashref and push into array
				}
			}
		}
	}
}

if (1) {
	# Vary number of cases, with number of replicates constant
	my $run = { Replicates => $chosen_replicates,
				TreatEff => $chosen_treat_eff};
	# Iterate over values for Base (0.00) and the chosen value for the Intervention
	for my $IntrvProp (0.00, $chosen_intrv_prop) {
		$run->{IntrvProp} = $IntrvProp;
		for my $Cases (10000, 100000, 1000000, 10000000, 100000000) {
		#for my $Cases (10000, 100000) {
			$run->{Cases} = $Cases;
			# Activate sources of PA, sequentially
			for my $PAonRR (0, 1) {
				$run->{PAonRR} = $PAonRR;
				for my $PAonAlpha (0, 1) {
					$run->{PAonAlpha} = $PAonAlpha;
					push @runs, { %$run }; # create shallow copy of hashref and push into array
				}
			}
		}
	}
}


####################
# Simulate Runs    #
####################

my %run_names;

# Simulate all runs
for my $run (@runs) {
	# Turn the row into variables
	my $Cases      = $run->{Cases}; # number of cases
	my $Replicates = $run->{Replicates}; # number of replicates
	my $PAonRR     = $run->{PAonRR}; # do PA on RR_condition
	my $PAonAlpha  = $run->{PAonAlpha}; # do PA on Alpha
	my $IntrvProp  = $run->{IntrvProp}; # treatment proportion
	my $TreatEff   = $run->{TreatEff}; # treatment effectiveness

	# Determine scenario based on parameter inputs
	my $Scenario = 'Unknown';
	$Scenario = 'Base' if $IntrvProp == 0.0; # Base scenario
	$Scenario = 'Intervention' if $IntrvProp == $chosen_intrv_prop; # Intervention scenario
	
	# Construct run name
	my $run_name = $Scenario;
	$run_name .= '_N'.(($Cases >= 1000000) ? ($Cases/1000000).'m' : ($Cases/1000).'k');
	$run_name .= '_R'.$Replicates;
	$run_name .= '_PA'.$PAonRR.$PAonAlpha;
	$run_name .= '_P'.($IntrvProp*100);
	$run_name .= '_E'.($TreatEff*100);
	
	# skip run if already done
	next if exists $run_names{$run_name};
	$run_names{$run_name} = 1;
	
	#my $run_name = "N10m_R100_PA00_P00_E00";
	print "simulate run ${run_name}\n";

	# construct command line argument list for model executable
	my @args = (
		$model_exe,
		# settings and parameters common to all scenarios
		'-ini',	'Common.ini',
		# directory containing csv files with parameter values for replicates
		'-OpenM.ParamDir', '.',
		# scenario name
		'-OpenM.RunName', $run_name,
		'-OpenM.LogFilePath', "${run_name}.log",
		
		# number of cases
		'-Parameter.SimulationCases', "${Cases}",
		# number of replicates
		'-OpenM.SubValues', "${Replicates}", 
		# do PA for RR (use s.d. or set to s.d. to zero to disable PA for RR)
		'-Parameter.log_RR_sigma', ($PAonRR ? $log_RR_sigma : 0.000000),
		# do PA for Alpha (use values from csv, or supply mean to disable PA for Alpha
		($PAonAlpha
		  ? ('-SubValue.Alpha', 'csv')      # get PA values for Alpha from csv file
		  : ('-Parameter.Alpha', $Alpha_mu) # turn PA off by setting Alpha to the average value
		),
		# Intervention proportion
		'-Parameter.InterventionProportion', $IntrvProp,
		# Treated efficacy
		'-Parameter.TreatedEffectiveness', $TreatEff,
	);
	#print @args, "\n";
	# simulate the scenario
	system(@args);
}


####################
# Assemble results #
####################

print "export DB to csv\n";
system (
	$dbcopy_exe,
	#'-OpenM.LogToConsole', '0',
	'-m', $model,
	'-dbcopy.To', 'csv',
);

# Get run_lst.csv as a refarray of refhash's, (array of records with named fields)
# Note: The helper function get_table_rh is defined below.
my $run_lst_ra = get_table_rh("${model}/run_lst.csv");

# Get run_option.csv and convert into hash %run_options with key run_id|option_key
my $run_option_ra = get_table_rh("${model}/run_option.csv");
my %run_option;
for my $rec (@{$run_option_ra}) {
	my $run_id = $rec->{run_id};
	my $option_key = $rec->{option_key};
	my $option_value = $rec->{option_value};
	my $key = "${run_id}|${option_key}";
	$run_option{$key} = $option_value;
}

# Prepare output csv file for PivotTable construction

# Output file name 
my $out_name_csv = "Results.csv";

# Output csv column names, in output order
my @out_column_names = (
	# columns from inputs
	  'run_name'
	, 'Scenario' # derived from inputs
	, 'Cases'
	, 'Replicates'
	, 'PAonRR'
	, 'PAonAlpha'
	, 'IntrvProp'
	, 'TreatEff'
	
	# columns from results
	, 'member' # replicate identifier {0..whatever}
	, 'AvgYearsLived' # Average Years lived
	, 'GiniYearsLived' # Gini coefficient of years lived
	, 'AvgIntrvCost' # Average intervention cost
);

# Initialize output csv file
open my $out, '>:utf8', $out_name_csv or die "unable to create output file out_name_csv=${out_name_csv}, stopped";
# Write BOM to CSV so that Excel will read UTF-8 correctly
#print $out_B "\xEF\xBB\xBF";
print $out chr(65279);
# Write header record
print $out join(',', @out_column_names), "\n";
# Create Text::CSV object for output
my $out_csv = Text::CSV_XS->new({ binary => 1, auto_diag => 1 });
$out_csv->column_names(\@out_column_names);

# Iterate all runs
for my $run (@{$run_lst_ra}) {
	my $run_id = $run->{run_id};
	my $run_name = $run->{run_name};

	# $data_dir is the folder containing the csv's for parameters and tables for the run
	my $data_dir = "${model}/run.${run_id}.${run_name}";
	-d $data_dir or die "invalid data directory data_dir=${data_dir}, stopped";
	
	# Get classifier variables for the run
	# Could also get scalar parameters using helper function get_scalar_parameter:
	#my $Cases      = get_scalar_parameter("${data_dir}/SimulationCases.csv");
	#my $TreatProp  = get_scalar_parameter("${data_dir}/Treated_proportion.csv");
	#my $TreatEff   = get_scalar_parameter("${data_dir}/Treated_effectiveness.csv");
	my $Cases      = $run_option{"${run_id}|Parameter.SimulationCases"};
	my $Replicates = $run->{sub_count};
	my $log_RR_sigma = $run_option{"${run_id}|Parameter.log_RR_sigma"};
	my $PAonRR = ($log_RR_sigma != 0.0) ? 1 : 0;
	my $PAonAlpha = (exists $run_option{"${run_id}|SubValue.Alpha"}) ? 1 : 0;
	my $IntrvProp  = $run_option{"${run_id}|Parameter.InterventionProportion"};
	my $TreatEff   = $run_option{"${run_id}|Parameter.TreatedEffectiveness"};

	# Determine scenario from prefix of run_name
	my $Scenario = $run_name;
	$Scenario =~ s/_.*//; # delete trailing part of name, _ and everything following
	
	# Diagnostic output
	print "run_name=${run_name}:"
		." Scenario=${Scenario}"
		." Cases=${Cases}"
		." Replicates=${Replicates}"
		." PAonRR=${PAonRR}"
		." PAonAlpha=${PAonAlpha}"
		." IntrvProp=${IntrvProp}"
		." TreatEff=${TreatEff}"
		."\n" if 0;

	# Get Results table, with all accumulators (trailing accumulators are computed measures)
	my $Results_acc_all = get_table_rh("${data_dir}/Results.acc-all.csv");
	
	# Initialize output record hash values for scenario properties, using values set from run properties (above).
	# These values will be reused for each CSV record written in loop below.
	my $out_rec = {
		run_name => $run_name,
		Scenario => $Scenario,
		Cases => $Cases,
		Replicates => $Replicates,
		PAonRR => $PAonRR,
		PAonAlpha => $PAonAlpha,
		IntrvProp => $IntrvProp,
		TreatEff => $TreatEff,
	};

	# Iterate over all table cells
	for my $in_rec (@{$Results_acc_all}) {
		my $member = $in_rec->{sub_id};
		# Get measures
		my $Persons =  $in_rec->{acc4};
		my $AvgYearsLived = $in_rec->{acc5};
		my $GiniYearsLived = $in_rec->{acc6};
		my $AvgIntrvCost = $in_rec->{acc7};
		$out_rec->{member} = $member;
		$out_rec->{AvgYearsLived} = $AvgYearsLived;
		$out_rec->{GiniYearsLived} = $GiniYearsLived;
		$out_rec->{AvgIntrvCost} = $AvgIntrvCost;
		$out_csv->print_hr($out, $out_rec);
		print $out "\n";
	}

} # runs

close $out;


####################
# Helper functions #
####################

# Retrieve all values of a table from a csv exported by dbcopy, as an array of hashref's
# arg0 - the input csv file for the table (exported by dbcopy)
# returns - the array of hashref's
sub get_table_rh {
	my $table_csv = shift(@_);
	open my $fh, '<:encoding(UTF-8)', $table_csv or die "unable to open table_csv=${table_csv}, stopped";
	
	my $csv = Text::CSV_XS->new({ binary => 1, auto_diag => 1 });
	$csv->column_names($csv->getline($fh)); # use header to assign field names for hash
	my $result = $csv->getline_hr_all($fh);
	close $fh;
	return $result;
}

# Retrieve the value of a scalar parameter from a csv exported by dbcopy
# arg0 - the input csv file for the parameter (exported by dbcopy)
# returns - the scalar parameter value
sub get_scalar_parameter {
	my $param_csv = shift(@_);
	my $param_value;
	open INPUT, '<'.$param_csv or die "unable to open param_csv=${param_csv}, stopped";
	# Skip header record
	my $header = <INPUT>;
	# Next record is the scalar parameter value
	my $record = <INPUT>;
	defined($record) or die "missing record in param_csv=${param_csv}, stopped";
	chomp $record;
	# Fields are sub_id,param_value
	$param_value = (split(/,/, $record))[1];
	close INPUT;
	return $param_value;
}

