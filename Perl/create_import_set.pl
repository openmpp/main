# Creates a parameter set with output from an upstream model for use in a downstream model

# Details:
# 

use strict;
use warnings;

my $script_name = 'create_import_set';
my $script_version = '1.0';

use Getopt::Long::Descriptive;
use Cwd;
use File::Basename;
use File::Temp qw/ tempfile tempdir /;
use File::Path qw(make_path remove_tree);

use Archive::Zip qw( :ERROR_CODES :CONSTANTS );
use Archive::Zip::MemberRead;
Archive::Zip::MemberRead->setLineEnd("\x0a"); # change default from \r\n (Windows), because dbcopy uses \n

# add the directory of this script to search path to use Perl modules
use lib dirname (__FILE__);

my ($opt, $usage) = describe_options(
	$script_name.' %o',
	[ 'help|h'         => 'print usage message and exit' ],
	[ 'version|v'      => 'print version and exit' ],

	[ 'imports|i=s'    => 'path of model imports csv file' ],
	[ 'upstream|u=s'   => 'name of upstream model' ],
	[ 'run|r=s'        => 'name of upstream model run' ],
	[ 'downstream|d=s' => 'name of downstream model' ],
	[ 'workdir|w=s'    => 'path of working directory for zips (default is current directory)',
        { default => '.' } ],
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
my $imports_in = $opt->imports;
-e $imports_in or die "model imports file ${imports_in} not found";

$opt->upstream or die "name of upstream model must be specified using -u";
my $upstream_model_name = $opt->upstream;
length($upstream_model_name) > 0 or die "name of upstream model must be specified using -u";

$opt->run or die "name of upstream model run must be specified using -r";
my $run_name = $opt->run;

$opt->downstream or die "name of downstream model must be specified using -d";
my $downstream_model_name = $opt->downstream;
length($downstream_model_name) > 0 or die "name of downstream model must be specified using -d";

my $workdir = $opt->workdir;
-d $workdir or die "working directory ${workdir} not found";

my $verbose = 0;
$verbose = 1 if $opt->verbose;

my $run_in_zip = "${workdir}/${upstream_model_name}.run.${run_name}.zip";
-f $run_in_zip or die "upstream run file ${run_in_zip} not found";

# Read the input zip of the upstream model run
my $zip_in = Archive::Zip->new();
unless ( $zip_in->read( ${run_in_zip} ) == AZ_OK ) {
    die "problem reading ${run_in_zip}";
}

if (0) {
    # code in this block is isolated and used just to probe the input zip
    my @in_member_names = $zip_in->memberNames();
    my $count = @in_member_names;
    print "\n${count} members in run zip\n" if $verbose;
    #print join("\n",@in_member_names)."\n" if $verbose;
}

# process json file from run zip
my $upstream_lang_code = '';
{
    my $run_zip_json = '';
    my $run_zip_model_name = '';
    my $run_zip_run_name = '';
    
    my @arr = $zip_in->membersMatching(".*\.json");
    my $count = @arr;
    $count == 1 || die "${count} matches for json metadata file in run zip";
    my $zip_member = $arr[0];
    $run_zip_json = $zip_in->contents($zip_member);
    print "run_zip_json length=".length($run_zip_json)."\n" if $verbose;
    
    # obtain upstream model name from json
    # search for property ModelName
    $run_zip_json =~ /"ModelName":"(\w+)"/;
    $run_zip_model_name = $1;
    print "run_zip_model_name = ${run_zip_model_name}\n" if $verbose;
    $upstream_model_name eq $run_zip_model_name or die "incoherence between upstream model name ${upstream_model_name} and model name inside run zip ${run_zip_model_name}";
    
    # obtain upstream run name from json
    # search for property Name
    $run_zip_json =~ /"Name":"(\w+)"/;
    $run_zip_run_name = $1;
    print "run_zip_run_name=${run_zip_run_name}\n" if $verbose;
    $run_name eq $run_zip_run_name or die "incoherence between upstream run name ${run_name} and run name inside run zip ${run_zip_run_name}";
    
    # obtain upstream default language name
    # search for property LangCode
    $run_zip_json =~ /"LangCode":"(\w+)"/;
    $upstream_lang_code = $1;
    print "upstream_lang_code=${upstream_lang_code}\n" if $verbose;
    length($upstream_lang_code) > 0 or die "unable to get default language code from run zip file";
}

# Output set has same name as input run
my $set_name = $run_name;

# Build set contents
my $zip_out_topdir = "${downstream_model_name}.set.${set_name}";
my $zip_out_csvdir = "${zip_out_topdir}/set.${set_name}/";
my $zip_out_json = "${zip_out_topdir}/${downstream_model_name}.set.${set_name}.json";

chdir $workdir or die "unable to change directory to ${workdir}";
remove_tree $zip_out_topdir || die "unable to remove ${zip_out_topdir}";
make_path $zip_out_topdir || die "unable to create ${zip_out_topdir}";
make_path $zip_out_csvdir || die "unable to create ${zip_out_csvdir}";

# Prepare the output set zip
my $zip_out = Archive::Zip->new();
my $result;
$result = $zip_out->addDirectory($zip_out_topdir);
$result = $zip_out->addDirectory($zip_out_csvdir);

# The json metadata file for the output set
my $set_json = '';

# Build top slice of sandwich of json metadata for output set
$set_json .= "{\n";
$set_json .= "  \"ModelName\":\"${downstream_model_name}\",\n";
$set_json .= "  \"Name\":\"${set_name}\",\n";
$set_json .= "  \"Param\":[\n";

# Read the imports csv file
# and process each import table / parameter.

my $line = 0;
my $parameters_processed = 0;
open IN_FILE, '<'.$imports_in || die "failed to open ${imports_in}";
my $header = <IN_FILE>; # skip header
chomp $header;
$header eq 'parameter_name,parameter_rank,from_name,from_model_name,is_sample_dim'
   || die "invalid header line in ${imports_in}";
while (<IN_FILE>) {
    $line++;
    my $record = $_;
    chomp $record;
    my @fields = split(/,/, $record);
    my $field_count = @fields;
    $field_count == 5 || die "invalid number of fields in record ${line} of ${imports_in}";
    my $parameter_name = $fields[0];
    my $parameter_rank = $fields[1];
    my $from_name = $fields[2];
    my $from_model_name = $fields[3];
    my $is_sample_dim = $fields[4];
    if ($from_model_name eq $upstream_model_name) {
        # Found downstream model import for specified upstream model
        $parameters_processed++;
        print "\ncreating ${parameter_name} from ${from_name}\n" if $verbose;
        
        my $subcount = 1; # is always 1 for hacked modgen-style import
        
        # Add information on this parameter to json file for the set
        # The parameter value description is set equal to the upstream model name followed by the run name 
        $set_json .= "    {\"Name\":\"${parameter_name}\",\n";
        $set_json .= "     \"Subcount\":${subcount},\n";
        $set_json .= "     \"Txt\":[\n";
        $set_json .= "        {\"LangCode\":\"${upstream_lang_code}\",\"Descr\":\"${upstream_model_name}: ${run_name}\"}\n";
        $set_json .= "     ],\n";
        $set_json .= "    },\n";

        # TODO use hack flag column to decide what to do.
        my $out_header = "sub_id";
        for (my $i = 0; $i < $parameter_rank; $i++) {
            $out_header .= ",Dim".$i;
        }
        $out_header .= ",param_value";
        print "out_header = ${out_header}\n" if $verbose;
        
        # regexp in line below finds table like TABLE.acc-all.csv, ignores path part
        my @arr = $zip_in->membersMatching(".*\\/${from_name}\\.acc-all\\.csv");
        my $count = @arr;
        $count == 1 || die "${count} matches for table ${from_name} in run zip";
        my $zip_member = $arr[0];
        
        my $zip_out_csv = "${zip_out_csvdir}/${parameter_name}.csv";
        print "zip_out_csv = ${zip_out_csv}\n" if $verbose;
        
        if (0) {
            # This code block for testing only, pending line-by-line transformation of table cell to parameter cell
            my $contents_in = $zip_in->contents($zip_member);
            print "length=".length($contents_in)."\n" if $verbose;;
            
            # TODO placeholder code for testing output zip
            my $contents_out = $contents_in;
            my $csv_out_member = $zip_out->addString( $contents_out, $zip_out_csv );
            $csv_out_member->desiredCompressionMethod( COMPRESSION_DEFLATED );
        }

        if (1) {
            # This code block reads the current input run csv line by line and transforms it to the output csv, line by line.
            # get file handle for the input table csv
            my $fh_in = Archive::Zip::MemberRead->new($zip_member);
            
            # get file handle for the output parameter csv
            # Archive::Zip has no method to write to zip through a file handle,
            # so use a temporary file.
            
            open my $fh_out, ">", $zip_out_csv or die "can't open ${zip_out_csv}";
            #my ($fh_out, $temp_name) = tempfile('create_import_set_tmpXXXXXX');
            #print "temp_name = ${temp_name}\n" if $verbose;
            $fh_out->write("${out_header}\n");
            
            # read the input table line by line
            # and write the output parameter line by line
            my $line_in;
            my $first_line = 1;
            while (defined($line_in = $fh_in->getline())) {
                #chomp $line_in; # probably not necessary per documentation of Archive::Zip::MemberRead
                if ($first_line) {
                    $first_line = 0;
                    # Note that file may start with BOM, in which case those bytes will be at the beginning of $line
                    print "in_header = ${line_in}\n" if $verbose;
                }
                my $line_out;
                # transform input line to output line
                $line_out = $line_in; # just for testing
                $fh_out->write("${line_out}\n");
            }
            $fh_in->close();
            $fh_out->close();
            #$zip_out->addFile($temp_name, $zip_out_csv);
            #my $temp_member = Archive::Zip::Member->newFromFile($temp_name, $zip_out_csv);
            #$zip_out->addMember($temp_member);
            #unlink $temp_name;
            #sleep 1;
        }
    }
}
close IN_FILE;

# Build bottom slice of sandwich of json metadata for output set
$set_json .= "  ]\n";
$set_json .= "}\n";

# write json metadata to output file
{
    my $filename = "test/test.csv";
    open JSON_FILE, ">${zip_out_json}" || die "unable to open ${zip_out_json}";;
    print JSON_FILE $set_json;
    close JSON_FILE;
}

$zip_out->addTree($zip_out_topdir, $zip_out_topdir);

#my $json_member = $zip_out->addString( $set_json, $zip_out_json, COMPRESSION_DEFLATED);
#$json_member->desiredCompressionMethod( COMPRESSION_DEFLATED );

# Save the zip file
unless ( $zip_out->writeToFileNamed("${downstream_model_name}.set.${set_name}.zip") == AZ_OK ) {
    die 'write error';
}

#print "\njson metadata for output set:\n".$json if $verbose;

print "${line} lines read\n" if $verbose;
print "${parameters_processed} parameters processed\n";

