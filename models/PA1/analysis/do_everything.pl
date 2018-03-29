use File::Copy;

# Locate ompp
my $om_root = $ENV{'OM_ROOT'};
$om_root ne '' or die 'environment variable OM_ROOT not defined, stopped';

# Locate dbcopy.exe utility
my $dbcopy_exe = "${om_root}/bin/dbcopy.exe";
-f $dbcopy_exe or die "utility $dbcopy_exe not found, stopped";

my $model = 'PA1';
my $model_exe = "../ompp/bin/${model}.exe";
my $model_sqlite = '../output/PA1.sqlite';

print "copying exe and sqlite\n";
copy $model_exe, '.' or die;
copy $model_sqlite, '.' or die;

my $SimulationCases = 10000;

{
	my $run_name = 'Base';
	print "running scenario ${run_name}\n";
	my @args = (
		$model_exe,
		'-ini',	'Common.ini',
		'-OpenM.RunName', $run_name,
		'-OpenM.LogFilePath', "${run_name}.log", 
		'-Parameter.log_RR_sigma', '0.000000',
		'-Parameter.Alpha', '0.20',
		);
	system(@args);
}

{
	my $run_name = 'Base_PA';
	print "running scenario ${run_name}\n";
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

{
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

print "running dbcopy\n";
system (
	$dbcopy_exe,
	"-m", $model,
	"-dbcopy.To", "csv",
	#"-dbcopy.IdCsv",
	"-dbcopy.DatabaseDriver", "SQLite",
	"-dbcopy.Database", $conn,
	#"-dbcopy.OutputDir", $out_export_dir,
);


