# Creates a parameter set with output from an upstream model for use in a downstream model

# Details:
# 

#use strict;
use warnings;

my $script_name = 'create_import_set';
my $script_version = '1.0';

use Getopt::Long::Descriptive;
use Cwd;
use File::Basename;

use Archive::Zip qw( :ERROR_CODES :CONSTANTS );
use Archive::Zip::MemberRead;
Archive::Zip::MemberRead->setLineEnd("\x0a"); # change default from \r\n (Windows), because dbcopy uses \n

# add the directory of this script to search path to use Perl modules
use lib dirname (__FILE__);

my ($opt, $usage) = describe_options(
	$script_name.' %o',
	[ 'help|h'      => 'print usage message and exit' ],
	[ 'version|v'   => 'print version and exit' ],

	[ 'imports|i=s' => 'path of model imports csv file' ],
	[ 'upstream|u=s'   => 'name of upstream model' ],
	[ 'downstream|d=s' => 'name of downstream model' ],
	[ 'run|r=s'     => 'path of input zip of upstream model run' ],
	[ 'set|s=s'     => 'path of output zip of downstream model parameter set' ],
	[ 'verbose'     => 'verbose log output' ],
);

if ($opt->version) {
	print $script_name.' version '.$script_version."\n";
	exit 0;
}

if ($opt->help) {
	print $usage->text;
	exit 0;
}

# Obtain arguments

$opt->imports or die "name of model imports csv file must be specified using -i";
my $imports = $opt->imports;
-e $imports or die "invalid path for model imports file ${imports}";

$opt->upstream or die "name of upstream model must be specified using -u";
my $upstream = $opt->upstream;
length($upstream) > 0 or die "name of upstream model must be specified using -u";

$opt->downstream or die "name of downstream model must be specified using -d";
my $downstream = $opt->downstream;
length($downstream) > 0 or die "name of downstream model must be specified using -d";

$opt->run or die "path of upstream model run zip must be specified using -r";
my $run = $opt->run;
-e $run or die "invalid path for run zip ${run}";

$opt->set or die "path of output zip for downstream model set must be specified using -s";
my $set = $opt->set;
# TODO attempt to create file, and bail if not successful

my $verbose = 0;
$verbose = 1 if $opt->verbose;


# Prepare the input zip for the upstream model run
my $zip_in = Archive::Zip->new();
unless ( $zip_in->read( ${run} ) == AZ_OK ) {
    die "problem reading ${run}";
}

{
    # code in this block is isolated and used just to probe the input zip
    my @in_member_names = $zip_in->memberNames();
    my $count = @in_member_names;
    print "\n${count} members in run zip\n" if $verbose;
    #print join("\n",@in_member_names)."\n" if $verbose;
}

# obtain json file from run zip
my $json_in = '';
my $upstream_model_name = '';
my $upstream_run_name = '';
my $upstream_lang_code = '';
{
    my @arr = $zip_in->membersMatching(".*\.json");
    my $count = @arr;
    $count == 1 || die "${count} matches for json metadata file in run zip";
    my $zip_member = $arr[0];
    $json_in = $zip_in->contents($zip_member);
    print "json_in length=".length($json_in)."\n" if $verbose;
    # obtain upstream model name from json
    # search for property ModelName
    $json_in =~ /"ModelName":"(\w+)"/;
    $upstream_model_name = $1;
    print "json_in upstream_model_name=${upstream_model_name}\n" if $verbose;
    
    # obtain upstream run name from json
    # search for property Name
    $json_in =~ /"Name":"(\w+)"/;
    $upstream_run_name = $1;
    print "json_in upstream_run_name=${upstream_run_name}\n" if $verbose;
    
    # obtain upstream default language name
    # search for property Name
    $json_in =~ /"LangCode":"(\w+)"/;
    $upstream_lang_code = $1;
    print "json_in upstream_lang_code=${upstream_lang_code}\n" if $verbose;
}

# Name output set to be the same as the input run name
my $downstream_set_name = $upstream_run_name;

# Prepare the output set zip
my $zip_out = Archive::Zip->new();
# Add top-level directory
my $zip_out_topdir = "${downstream}.set.101";
my $zip_out_csvdir = "${zip_out_topdir}/set.101.${downstream_set_name}/";
my $zip_out_json = "${zip_out_topdir}/${downstream}.set.101.${downstream_set_name}.json";

my $result;
$result = $zip_out->addDirectory($zip_out_topdir);
$result = $zip_out->addDirectory($zip_out_csvdir);

# Json metadata file contents
my $json = '';
$json .= "{\n";
$json .= "  \"ModelName\":\"${downstream}\",\n";
$json .= "  \"Name\":\"${downstream_set_name}\",\n";
$json .= "  \"Param\":[\n";

# read the imports csv file
{
    my $line = 0;
    my $parameters_processed = 0;
    open IN_FILE, '<'.$imports || die "failed to open ${imports}";
    my $header = <IN_FILE>; # skip header
    chomp $header;
    $header eq 'parameter_name,parameter_rank,from_name,from_model_name,is_sample_dim'
       || die "invalid header line in ${imports}";
    while (<IN_FILE>) {
        $line++;
        my $record = $_;
        chomp $record;
        my @fields = split(/,/, $record);
        my $field_count = @fields;
        $field_count == 5 || die "invalid number of fields in record ${line} of ${imports}";
        my $parameter_name = $fields[0];
        my $parameter_rank = $fields[1];
        my $from_name = $fields[2];
        my $from_model_name = $fields[3];
        my $is_sample_dim = $fields[4];
        if ($from_model_name eq $upstream) {
            $parameters_processed++;
            print "\ncreating ${parameter_name} from ${from_name}\n" if $verbose;
            
            my $subcount = 1; # is always 1 for hacked modgen-style import
            
            # Add information on this parameter to json file.
            $json .= "    {\"Name\":\"${parameter_name}\",\n";
            $json .= "     \"Subcount\":${subcount},\n";
            $json .= "     \"Txt\":[\n";
            $json .= "        {\"LangCode\":\"${upstream_lang_code}\",\"Descr\":\"${upstream_run_name}\"}\n";
            $json .= "     ],\n";
            $json .= "    },\n";
            
            my $out_header = "sub_id";
            for (my $i = 0; $i < $parameter_rank; $i++) {
                $out_header .= ",Dim".$i;
            }
            $out_header .= ",param_value";
            print "out header = ${out_header}\n" if $verbose;
            
            # regexp in line below finds table like TABLE.acc-all.csv, ignores path part
            my @arr = $zip_in->membersMatching(".*\\/${from_name}\\.acc-all\\.csv");
            my $count = @arr;
            $count == 1 || die "${count} matches for table ${from_name} in run zip";
            my $zip_member = $arr[0];
            
            my $zip_out_csv = "${zip_out_csvdir}/${parameter_name}.csv";
            
            if (1) {
                # This code block for testing only, pending line-by-line transformation of table cell to parameter cell
                my $contents_in = $zip_in->contents($zip_member);
                print "length=".length($contents_in)."\n" if $verbose;;
                
                # placeholder code for testing output zip
                my $contents_out = $contents_in;
                my $csv_out_member = $zip_out->addString( $contents_out, $zip_out_csv );
                $csv_out_member->desiredCompressionMethod( COMPRESSION_DEFLATED );
            }

            if (1) {
                # This code block reads the input csv line by line and transforms it to the output csv, line by line.
                # get file handle for the input table csv
                my $fh_in = Archive::Zip::MemberRead->new($zip_member);
                
                # get file handle for the output parameter csv
                # TODO
                
                # read the input table line by line
                # and write the output parameter line by line
                my $first_line = 1;
                while (defined($line_in = $fh_in->getline())) {
                    chomp $line_in; # probably not necessary per documentation of Archive::Zip::MemberRead
                    if ($first_line) {
                        $first_line = 0;
                        print "in_header = ${line_in}\n" if $verbose;
                    }
                }
                $fh_in->close();
            }
        }
    }
    close IN_FILE;
    
    $json .= "  ]\n";
    $json .= "}\n";
    
    # write json metadata to output zip
    my $json_member = $zip_out->addString( $json, $zip_out_json );
    $json_member->desiredCompressionMethod( COMPRESSION_DEFLATED );
    
    # Save the zip file
    unless ( $zip_out->writeToFileNamed("${downstream}.set.101.zip") == AZ_OK ) {
        die 'write error';
    }
    
    print "\njson metadata for output set:\n".$json if $verbose;
    
    print "${line} lines read\n" if $verbose;
    print "${parameters_processed} parameters processed\n";
}    

