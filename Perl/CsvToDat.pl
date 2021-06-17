# Create missing Modgen dat parameter input files from OpenM++ database

# Details:
# 
# For each param in scenario folder which was given as <param>*.csv or <param>*.tsv,
# creates a file modgen_<param>.dat in Modgen format, using metadata in a pbulsihed ompp database.

#use strict;
use warnings;

my $script_name = 'CsvToDat';
my $script_version = '1.0';

use Getopt::Long::Descriptive;
use Cwd;
use File::Path qw(make_path remove_tree);
use File::Copy;
use File::Temp qw(tempdir);
use Capture::Tiny qw/capture tee capture_merged tee_merged/;
use File::Basename;

# add the directory of this script to search path to use Perl modules
use lib dirname (__FILE__);

my ($opt, $usage) = describe_options(
	$script_name.' %o',
	[ 'help|h'      => 'print usage message and exit' ],
	[ 'version|v'   => 'print version and exit' ],

	[ 'model=s'   => 'name of model' ],
	[ 'ompp_database=s'   => 'path of ompp database containing scenario' ],
	[ 'scenario_folder=s' => 'path of scenario folder for generated dat files' ],
	[ 'scenario=s' => 'name of scenario (default is Default)' ],
	[ 'keep_tmp|k' => 'keep temporary files in folder ./tmp_CsvToDat' ],
	[ 'verbose'    => 'verbose log output' ],
);

if ($opt->version) {
	print $script_name.' version '.$script_version."\n";
	exit 0;
}

if ($opt->help) {
	print $usage->text;
	exit 0;
}

my $scenario_folder = '';
$scenario_folder = $opt->scenario_folder if $opt->scenario_folder;
-d $scenario_folder or die "invalid scenario folder ${scenario_folder}";

my $ompp_database = '';
$ompp_database = $opt->ompp_database if $opt->ompp_database;
-e $ompp_database or die "invalid ompp database ${ompp_database}";

my $model = '';
$model = $opt->model if $opt->model;

my $scenario = 'Default';
$scenario = $opt->scenario if $opt->scenario;

# adapted from test_models.pl
my $om_root = $ENV{'OM_ROOT'};
if ( ! $om_root || $om_root eq '') {
	# Try parent directory, assuming this script was invoked in the OM_ROOT/Perl directory
	my $save_dir = getcwd();
	chdir '..';
	$om_root = getcwd();
	chdir $save_dir;
}
else {
	-d $om_root || die "directory not found OM_ROOT='${om_root}'\n";
}

# adapted from ompp_batch.pl
my $dbcopy_exe = "${om_root}/bin/dbcopy.exe" or die;
-f $dbcopy_exe or die "dbcopy utility $dbcopy_exe not found, stopped";

# adapted from result_bridge.pl
my $temp_dir;
if ($opt->keep_tmp) {
	# For debugging, place dbcopy results in known fixed location.
	$temp_dir = "./tmp_CsvToDat";
	# Remove existing folder and recreate to avoid anomalies
	remove_tree $temp_dir || die;
	make_path $temp_dir || die;
}
else {
	# For production, place dbcopy results in temporary directory, which is deleted at script end.
	$temp_dir = tempdir( CLEANUP => 1 );
}
print "temp_dir=$temp_dir\n" if $opt->verbose;

# connection string to ompp DB
my $in_db_msdos = $ompp_database;
$in_db_msdos =~ s@/@\\@g;
my $conn = "\"Database=${in_db_msdos}; Timeout=86400; OpenMode=ReadWrite;\"";

# Dump ompp DB to csv
my ($merged, $retval) = capture_merged {
	my @args = (
		"${dbcopy_exe}",
		"-m" , $model,
		"-dbcopy.To", "csv",
		"-dbcopy.IdCsv",
		"-dbcopy.DatabaseDriver", "SQLite",
		"-dbcopy.Database", $conn,
		"-dbcopy.OutputDir", $temp_dir,
		);
	system(@args);
};

if ($retval) {
	print $merged;
	die;
}

# metadata to hashes
my %param_name_TO_param_id;
my %param_id_TO_param_name;
my %param_id_TO_rank;
my %param_id_TO_type_id;
{
    my $in_file = "${temp_dir}/${model}/parameter_dic.csv";
    open IN_FILE, '<'.$in_file || die "failed to open ${in_file}";
    my $header = <IN_FILE>;
    while (<IN_FILE>) {
        my $record = $_;
        chomp $record;
		my @fields = split(/,/, $record);
        my $param_id = $fields[1];
        my $param_name = $fields[3];
        my $param_rank = $fields[7];
        my $type_id = $fields[8];
        $param_name_TO_param_id{$param_name} = $param_id;
        $param_id_TO_param_name{$param_id} = $param_name;
        $param_id_TO_rank{$param_id} = $param_rank;
        $param_id_TO_type_id{$param_id} = $type_id;
    }
    close IN_FILE;
}

# metadata to hash
my %param_id_dim_TO_type;
{
    my $in_file = "${temp_dir}/${model}/parameter_dims.csv";
    open IN_FILE, '<'.$in_file || die "failed to open ${in_file}";
    my $header = <IN_FILE>;
    while (<IN_FILE>) {
        my $record = $_;
        chomp $record;
		my @fields = split(/,/, $record);
        my $param_id = $fields[1];
        my $dim_id = $fields[2];
        my $type_id = $fields[4];
        # combined key of param_id and dim_id
        $param_id_dim_TO_type{$param_id.'_'.$dim_id} = $type_id;
    }
    close IN_FILE;
}

# metadata to hash
my %type_id_TO_type_name;
{
    my $in_file = "${temp_dir}/${model}/type_dic.csv";
    open IN_FILE, '<'.$in_file || die "failed to open ${in_file}";
    my $header = <IN_FILE>;
    while (<IN_FILE>) {
        my $record = $_;
        chomp $record;
		my @fields = split(/,/, $record);
        my $type_id = $fields[1];
        my $type_name = $fields[3];
        # map bool to logical or else modgen croaks
        $type_name = 'logical' if $type_name eq 'bool';
        $type_id_TO_type_name{$type_id} = $type_name;
    }
    close IN_FILE;
}


# create array of the parameters with a csv/tsv which require a dat version
my @parameters;
for my $file (glob("${scenario_folder}/*.[ct]sv")) {
    $file =~ s@^.*[/\\]@@; # remove directory part
    $file =~ s/[.].*$//; # remove extension+ part
    push @parameters, $file;
}
#print join "\n", @parameters;

@parameters = sort @parameters;
for my $param_name (@parameters) {
    #print "param_name=${param_name}\n";
    my $out_file = "${scenario_folder}/modgen_${param_name}.dat";
    my $in_file = "${temp_dir}/${model}/set.${scenario}/${param_name}.id.csv";
    -e $in_file || die "parameter ${param_name} in scenario ${scenario} not found in dbcopy results: in_file=${in_file}";
    # the id of this param
    my $param_id = $param_name_TO_param_id{$param_name};
    #print "param_id=${param_id}\n";
    my $param_type_id = $param_id_TO_type_id{$param_id};
    my $param_type_name = $type_id_TO_type_name{$param_type_id};
    my $param_type_logical = $param_type_name eq 'logical';
    my $param_rank = $param_id_TO_rank{$param_id};
    my $dim_part = '';
    for (my $dim_id = 0; $dim_id < $param_rank; $dim_id++) {
        my $dim_type_id = $param_id_dim_TO_type{$param_id.'_'.$dim_id};
        my $dim_type_name = $type_id_TO_type_name{$dim_type_id};
        $dim_part .="[${dim_type_name}]";
    }
    
    open OUT_FILE, '>'.$out_file || die "failed to open ${out_file}";
    
    open IN_FILE, '<'.$in_file || die "failed to open ${in_file}";
    my $header = <IN_FILE>; # skip header
    
    print OUT_FILE "parameters {\n";
    print OUT_FILE "    ${param_type_name} ${param_name}${dim_part} = ";
    if ($param_rank == 0) {
        # scalar
        my $record = <IN_FILE>;
        chomp $record;
        $record =~ /([^,]+)$/;
        $value = $1;
        $value = uc($value) if $param_type_logical; # for Modgen TRUE and FALSE
        print OUT_FILE "${value};\n";
        close IN_FILE;
    }
    else {
        # non-scalar
        print OUT_FILE "{\n";
        while (<IN_FILE>) {
            my $record = $_;
            chomp $record;
            $record =~ /([^,]+)$/;
            $value = $1;
            print OUT_FILE "        ${value},\n";
        }
        close IN_FILE;
        print OUT_FILE "    };\n";
    }
    print OUT_FILE "};\n";
    close OUT_FILE;
}
