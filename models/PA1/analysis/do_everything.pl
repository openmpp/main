# Simulate all PA1 scenarios and construct csv for Excel PivotTable exploration

use strict;
use warnings;

use File::Copy; # for copy
use File::Path; # for rmtree

# Locate ompp installation folder
my $om_root = $ENV{'OM_ROOT'};
$om_root ne '' or die 'environment variable OM_ROOT (ompp install directory) not defined, stopped';

# Locate ompp utility dbcopy.exe
my $dbcopy_exe = "${om_root}/bin/dbcopy.exe";
-f $dbcopy_exe or die "ompp utility $dbcopy_exe not found, stopped";

# For PA: The s.d. of RR with condition
my $log_RR_sigma = 0.10;
# For PA: The mean of Alpha values
my $Alpha_mu = 0.20;

my $model = 'PA1';
my $model_exe = "../ompp/bin/${model}.exe";
my $model_sqlite = "../output/${model}.sqlite";

print "copy model exe and DB to current directory (analysis directory)\n";
copy $model_exe, '.' or die "failed to copy ${model_exe}, stopped";
copy $model_sqlite, '.' or die "failed to copy ${model_sqlite}, stopped";

my $SimulationCases = 10000; # number of cases
my $OpenM_SubValues = 100; # number of replicates

# remove any existing csv output
rmtree "${model}";

# Enumerate the runs to simulate
my @runs = (
	#    Cases   Replicates PAonRR PAonAlpha TreatProp TreatEff
	[     10000,        100,     0,        0,     0.00,    1.00 ],
	[    100000,        100,     0,        0,     0.00,    1.00 ],
#	[   1000000,        100,     0,        0,     0.00,    1.00 ],
#	[  10000000,        100,     0,        0,     0.00,    1.00 ],
#	[ 100000000,        100,     0,        0,     0.00,    1.00 ],
);

for my $run (@runs) {
	# get the row as an array
	my @run = @{$run};
	# Turn the row into variables
	my $Cases      = $run[0]; # number of cases
	my $Replicates = $run[1]; # number of replicates
	my $PAonRR     = $run[2]; # do PA on RR_condition
	my $PAonAlpha  = $run[3]; # do PA on Alpha
	my $TreatProp  = $run[4]; # treatment proportion
	my $TreatEff   = $run[5]; # treatment effectiveness

	# Construct run name
	my $run_name = 'N';
	$run_name .= ($Cases >= 1000000) ? ($Cases/1000000).'m' : ($Cases/1000).'k';
	$run_name .= '_R'.$Replicates;
	$run_name .= '_PA'.$PAonRR.$PAonAlpha;
	$run_name .= '_T'.($TreatProp*100);
	$run_name .= '_E'.($TreatEff*100);
	
	#my $run_name = "N10m_R100_PA00_T00_E00";
	print "simulate run ${run_name}\n";

	# construct argument list for model executable
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
		  ? ('-SubValue.Alpha', 'csv')      # get PA values from csv file
		  : ('-Parameter.Alpha', $Alpha_mu) # turn PA off by setting Alpha to the average value
		),
		# Treated proportion
		'-Parameter.Treated_proportion', $TreatProp,
		# Treated efficacy
		'-Parameter.Treated_effectiveness', $TreatEff,
	);
	#print @args, "\n";
	# simulate the scenario
	system(@args);
}

if (0) {
	my $run_name = "B_N10m_R100_PA00";
	print "run scenario ${run_name}\n";
	my @args = (
		$model_exe,
		'-ini',	'Common.ini',
		'-OpenM.RunName', $run_name,
		'-OpenM.LogFilePath', "${run_name}.log", 
		'-Parameter.log_RR_sigma', '0.100000',
		'-OpenM.ParamDir', '.',
		'-SubValue.Alpha', 'csv',
		);
	system(@args);
}

if (0) {
	my $run_name = 'Intervention_PA';
	print "running scenario ${run_name}\n";
	my @args = (
		$model_exe,
		'-ini',	'Common.ini',
		'-OpenM.RunName', $run_name,
		'-OpenM.LogFilePath', "${run_name}.log", 
		'-Parameter.log_RR_sigma', '0.100000',
		'-OpenM.ParamDir', '.',
		'-SubValue.Alpha', 'csv',
		'-Parameter.Treated_proportion', '0.750000',
		);
	system(@args);
}

print "export DB to csv\n";
system (
	$dbcopy_exe,
	#'-OpenM.LogToConsole', '0',
	'-m', $model,
	'-dbcopy.To', 'csv',
);
