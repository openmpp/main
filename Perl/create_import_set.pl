#!/usr/bin/perl
# Copyright (c) 2022 OpenM++ Contributors
# This code is licensed under the MIT license (see LICENSE.txt for details)

# Script to create a parameter set with output from an upstream model for use in a downstream model

use strict;
use warnings;

my $script_name = 'create_import_set';
my $script_version = '1.1';

use Getopt::Long::Descriptive;
use Cwd;
use File::Path qw(make_path remove_tree);

use Text::CSV qw( csv );

# The following module is included explicitly here
# because otherwise pp does not include it when building executable version of utility
# because pp does not detect that Text::CSV_PP is required
# because the module is loaded dynamically at runtime by Txt::CSV, not statically
use Text::CSV_PP; 

use Archive::Zip qw( :ERROR_CODES :CONSTANTS );
use Archive::Zip::MemberRead;
Archive::Zip::MemberRead->setLineEnd("\x0a"); # change default from \r\n (Windows), because dbcopy uses \n

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
	[ 'keep'           => 'keep and propagate all subs' ],
	[ 'verbose'        => 'verbose log output' ],
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

$opt->imports or die "Error: name of model imports csv file must be specified using -i";
my $imports_in = $opt->imports;
-e $imports_in or die "Error: model imports file ${imports_in} not found";

$opt->upstream or die "Error: name of upstream model must be specified using -u";
my $upstream_model_name = $opt->upstream;
length($upstream_model_name) > 0 or die "Error: name of upstream model must be specified using -u";

$opt->run or die "Error: name of upstream model run must be specified using -r";
my $run_name = $opt->run;

$opt->downstream or die "Error: name of downstream model must be specified using -d";
my $downstream_model_name = $opt->downstream;
length($downstream_model_name) > 0 or die "Error: name of downstream model must be specified using -d";

my $workdir = $opt->workdir;
-d $workdir or die "Error: working directory ${workdir} not found";

my $keep_all_subs = 0;
$keep_all_subs = 1 if $opt->keep;

my $verbose = 0;
$verbose = 1 if $opt->verbose;

my $run_in_zip = "${workdir}/${upstream_model_name}.run.${run_name}.zip";
-f $run_in_zip or die "Error: upstream run file ${run_in_zip} not found";

# Read the input zip of the upstream model run
my $zip_in = Archive::Zip->new();
unless ( $zip_in->read( ${run_in_zip} ) == AZ_OK ) {
    die "Error: problem reading ${run_in_zip}";
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
my $upstream_subcount;
{
    my $run_zip_json = '';
    my $run_zip_model_name = '';
    my $run_zip_run_name = '';
    
    my @arr = $zip_in->membersMatching(".*\.json");
    my $count = @arr;
    $count == 1 || die "Error: ${count} matches for json metadata file in run zip";
    my $zip_member = $arr[0];
    $run_zip_json = $zip_in->contents($zip_member);
    print "run_zip_json length=".length($run_zip_json)."\n" if $verbose;
    
    # obtain upstream model name from json
    # search for property ModelName
    $run_zip_json =~ /"ModelName":"([^"]*)"/;
    $run_zip_model_name = $1;
    print "run_zip_model_name = ${run_zip_model_name}\n" if $verbose;
    $upstream_model_name eq $run_zip_model_name or die "Error: incoherence between upstream model name ${upstream_model_name} and model name inside run zip ${run_zip_model_name}";
    
    # obtain upstream run name from json
    # search for property Name
    $run_zip_json =~ /"Name":"([^"]*)"/;
    $run_zip_run_name = $1;
    print "run_zip_run_name=${run_zip_run_name}\n" if $verbose;
    $run_name eq $run_zip_run_name or die "Error: incoherence between upstream run name ${run_name} and run name inside run zip ${run_zip_run_name}";
    
    # obtain upstream subcount from json
    # search for property SubCount
    $run_zip_json =~ /"SubCount":(\d+)/;
    $upstream_subcount = $1;
    print "upstream_subcount=${upstream_subcount}\n" if $verbose;
    
    # obtain upstream default language name
    # search for property LangCode
    $run_zip_json =~ /"LangCode":"([^"]*)"/;
    $upstream_lang_code = $1;
    print "upstream_lang_code=${upstream_lang_code}\n" if $verbose;
    length($upstream_lang_code) > 0 or die "Error: unable to get default language code from run zip file";
}

# Output set has same name as input run
my $set_name = $run_name;

# Build set contents
my $zip_out_topdir = "${downstream_model_name}.set.${set_name}";
my $zip_out_csvdir = "${zip_out_topdir}/set.${set_name}";
my $zip_out_json = "${zip_out_topdir}/${downstream_model_name}.set.${set_name}.json";

chdir $workdir or die "Error: unable to change directory to ${workdir}";
remove_tree $zip_out_topdir || die "Error: unable to remove ${zip_out_topdir}";
make_path $zip_out_topdir || die "Error: unable to create ${zip_out_topdir}";
make_path $zip_out_csvdir || die "Error: unable to create ${zip_out_csvdir}";

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
$set_json .= "  \"IsReadonly\":true,\n";
$set_json .= "  \"Param\":[\n";

# Read the imports csv file
# and process each import table / parameter.

my $line = 0;
my $is_first_parameter = 1;
my $parameters_processed = 0;
open IN_FILE, '<'.$imports_in || die "Error: failed to open ${imports_in}";
my $header = <IN_FILE>; # skip header
chomp $header;
$header eq 'parameter_name,parameter_rank,from_name,from_model_name,is_sample_dim'
   || die "Error: invalid header line in ${imports_in}";
while (<IN_FILE>) {
    $line++;
    my $record = $_;
    chomp $record;
    my @fields = split(/,/, $record);
    my $field_count = @fields;
    $field_count == 5 || die "Error: invalid number of fields in record ${line} of ${imports_in}";
    my $parameter_name = $fields[0];
    my $parameter_rank = $fields[1];
    my $from_name = $fields[2];
    my $from_model_name = $fields[3];
    my $is_sample_dim = $fields[4] eq 'TRUE';
    # Table rank is one less than parameter rank if modgen hack used
    my $table_rank = $parameter_rank - ($is_sample_dim ? 1 : 0);
    if ($from_model_name eq $upstream_model_name) {
        # Found downstream model import for specified upstream model
        $parameters_processed++;
        print "\n" if $verbose;
        print "creating ${parameter_name} from ${from_name}\n";
        
        # Parameters with modgen hack have one sub
        # others have all subs in the run if --keep flag specified,
        # others just sub 0.
        my $subcount;
        if ($is_sample_dim) {
            # modgen hack
            $subcount = 1;
        }
        elsif ($keep_all_subs) {
            # tables/parameters assumed to be variable by sub
            $subcount = $upstream_subcount;
        }
        else {
            # tables/parameters assumed to be constant over subs
            $subcount = 1;
        }
        
        # Add information on this parameter to json file for the set
        # The parameter value description is set equal to the upstream model name followed by the run name 
        
        if ($is_first_parameter) {
            $is_first_parameter = 0;
        }
        else {
            # insert separator before this array element
            $set_json .= ",\n";
        }
        
        $set_json .= "    {\"Name\":\"${parameter_name}\",\n";
        $set_json .= "     \"Subcount\":${subcount},\n";
        $set_json .= "     \"Txt\":[\n";
        $set_json .= "        {\"LangCode\":\"${upstream_lang_code}\",\"Descr\":\"${upstream_model_name}: ${run_name}\"}\n";
        $set_json .= "     ]\n";
        $set_json .= "    }\n";

        # Build the csv header for output set.
        # It will always match the rank of the downstream model parameter.
        # The first field is sub_id, and the last field is param_value.
        my $out_header = "sub_id";
        for (my $i = 0; $i < $parameter_rank; $i++) {
            $out_header .= ",Dim".$i;
        }
        $out_header .= ",param_value";
        print "out_header = ${out_header}\n" if $verbose;
        
        # regexp in line below finds table like TABLE.acc-all.csv, ignores path part
        my @arr = $zip_in->membersMatching(".*\\/${from_name}\\.acc-all\\.csv");
        my $count = @arr;
        $count == 1 || die "Error: ${count} matches for table ${from_name} in run zip";
        my $zip_member = $arr[0];
        
        my $zip_out_csv = "${zip_out_csvdir}/${parameter_name}.csv";
        print "zip_out_csv = ${zip_out_csv}\n" if $verbose;
        
        {
            # This code block reads the current input csv line by line and transforms it to the output csv.
            # Get file handle for the input table csv using Zip::MemberRead API.
            my $fh_in = Archive::Zip::MemberRead->new($zip_member);

            # open output csv file in staging directory
            open my $fh_out, ">:utf8", $zip_out_csv or die "Error: can't open ${zip_out_csv}";
            $fh_out->write("${out_header}\n");
            
            my $csv = Text::CSV->new({binary => 1});
            
            # read the input table line by line
            # and write the output parameter line by line
            my $first_line = 1;
            my $in_header;
            while (defined(my $line_in = $fh_in->getline())) {
                #chomp $line_in; # probably not necessary per documentation of Archive::Zip::MemberRead
                if ($first_line) {
                    $first_line = 0;
                    # Note that file may start with BOM, in which case those bytes will be at the beginning of $line
                    $in_header = $line_in;
                    print "in_header = ${in_header}\n" if $verbose;
                    next;
                }
                # handle final spurious empty line if present
                next if length($line_in) == 0;
                my $line_out;
                # transform input line to output line
                # Split input line into fields
                #my @fields_in = split(/[,]/, $line_in);
                $csv->parse($line_in) || die "Error: failed to parse ${line_in}";
                my @fields_in = $csv->fields();
                my $sub_id = $fields_in[0]; # The sub_id of this record of the upstream table
                my @fields_out = ();
                if ($is_sample_dim) {
                    # The current import uses hacked modgen approach.
                    # Insert constant sub_id with value 0.
                    # The true sub_id of the upstream table will be pushed to the explicit dimension Dim0 of downstream parameter (the field immediately after sub_id)
                    push @fields_out, '0';
                    push @fields_out, $fields_in[0]; # the sub_id of the table becomes Dim0 of the parameter
                }
                elsif ($keep_all_subs) {
                    # Modgen hack not used and --keep flag was specified,
                    # so propagate actual sub_id from table to parameter
                    push @fields_out, $fields_in[0];
                }
                else {
                    # --keep flag not specified, so upstream table is assumed invariant across subs.
                    # Process only records with sub_id 0 and skip any others.
                    if ($sub_id == 0) {
                        push @fields_out, $fields_in[0];
                    }
                    else {
                        next;
                    }
                }
                # push the dimension indices of the table to the parameter
                for (my $dim = 0; $dim < $table_rank; $dim++) {
                    push @fields_out, $fields_in[$dim + 1];
                }
                # Push the value field.
                # This is the last field in the table record, after any additional accumulator fields
                push @fields_out, $fields_in[-1];
                #$line_out = join ',', @fields_out;
                $csv->combine(@fields_out) || die "Error: failed to combine";
                $line_out = $csv->string();
                $fh_out->write("${line_out}\n");
            }
            $fh_in->close();
            $fh_out->close();
        }
    }
}
close IN_FILE;

# Build bottom slice of sandwich of json metadata for output set
$set_json .= "  ]\n";
$set_json .= "}\n";

# write json metadata to output file in staging directory
{
    open JSON_FILE, ">${zip_out_json}" || die "Error: unable to open ${zip_out_json}";;
    print JSON_FILE $set_json;
    close JSON_FILE;
}
sleep 1;

$zip_out->addTree($zip_out_topdir, $zip_out_topdir);

# Write the zip file
unless ( $zip_out->writeToFileNamed("${downstream_model_name}.set.${set_name}.zip") == AZ_OK ) {
    die 'unable to write output zip';
}

#print "\njson metadata for output set:\n".$json if $verbose;

print "${parameters_processed} downstream ${downstream_model_name} parameters created from upstream ${upstream_model_name} tables\n";

# cleanup - remove staging directory
remove_tree $zip_out_topdir || die "Error: unable to remove ${zip_out_topdir}";
