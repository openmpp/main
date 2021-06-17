# Copyright (c) 2013-2021 OpenM++ Contributors
# This code is licensed under the MIT license (see LICENSE.txt for details)

# Script to build utilities, etc.

# Get highest installed R version by enumeration (with implicit sorting)
my @R_exes = glob('"C:/Program Files/R/*/bin/R.exe"');
#print join("\n", @R_exes)."\n";
my $R_exe = pop(@R_exes);
#print "R_exe=".$R_exe."\n";

# Verbosity of report:
#  0 - errors only
#  1 - timing and success
#  2 - chatty
my $verbosity = 3;

use Capture::Tiny qw/capture tee capture_merged tee_merged/;
use File::Copy;
use File::Copy::Recursive qw(dircopy);
use File::Path qw(make_path remove_tree);
use File::Which;

# Get ompp root directory
chdir "..";
use Cwd qw(getcwd);
my $om_root = getcwd;
chdir 'Perl' or die;

my $merged; # output from build command
my $retval; # return value from build command
for my $utility ('ompp_export_excel', 'ompp_export_csv', 'modgen_export_csv', 'ompp_create_scex', 'patch_modgen12.1_outputs', 'test_models', 'CsvToDat') {
	my $input = "${utility}.pl";
	my $output = "${om_root}/bin/${utility}.exe";
	print "Building utility ${utility}\n";
	($merged, $retval) = capture_merged {
		my @args = (
			'pp',
			'-o',
			$output,
			$input,
			);
		system(@args);
	};
}

exit; #SFG temporary exit from following broken steps

# build R package
if (! -e "${R_exe}") {
	print "Skip R package build, not found: ${R_exe}\n";
}
else {
    print "Building R package using ${R_exe}\n";
    chdir "${om_root}/R";
    ($merged, $retval) = capture_merged {
        my @args = (
            $R_exe,
            'CMD',
            'build',
            'openMpp',
            );
        system(@args);
    };
}

# if GOROOT and GOPATH (and others for MinGW) defined then build dbcopy and oms
#
if ("$ENV{GOROOT}" eq "" || "$ENV{GOPATH}" eq "") {
	print "Skip go-based utilities (dbcopy, oms) : GOROOT or GOPATH is empty\n";
}
else {
	# For successful go build, arrange PATH so that gcc is 64-bit MinGW version
	# e.g. First path entry is C:\MinGW\mingw64\bin
	
	my $gopath = "$ENV{GOPATH}";
	
	for my $utility ('dbcopy', 'oms') {
		print "Building utility ${utility}\n";
		($merged, $retval) = capture_merged {
			my @args = (
				'go',
				'install',
				'-tags',
				'odbc',
				'github.com/openmpp/go/'.$utility,
				);
			system(@args);
		};
		if ($retval != 0) {
			print "Build ${utility} failed (output follows:\n$merged\n"
		}
		else {
			copy "${gopath}/bin/${utility}.exe", "${om_root}/bin";
		}
	}
}

# build ompp-ui
my $npm_exe = which 'npm';

sub is_folder_empty {
    my $dirname = shift;
    opendir(my $dh, $dirname) or die "Not a directory";
    return scalar(grep { $_ ne "." && $_ ne ".." } readdir($dh)) == 0;
}

if (! -e "${npm_exe}") {
	print "Skip ompp-ui build, npm not found. Install node and run nodevars.bat\n";
}
else {
    print "Building ompp-ui\n";
    chdir "${om_root}/../ompp-ui" || die;
    
    # install node modules if not yet installed
    if (! -d 'node_modules' || is_folder_empty('node_modules') ) {
        print "Doing 'npm install', please wait...\n";
        ($merged, $retval) = capture_merged {
            my @args = (
                'npm',
                'install',
                );
            system(@args);
        };
        if ($retval != 0) {
            print "Build ompp-ui failed (output follows:\n$merged\n";
			die;
        }
        print "Now we can do 'npm run build'\n";
    }
    
    # do actual build
	($merged, $retval) = capture_merged {
		my @args = (
			'npm',
			'run',
			'build',
			);
		system(@args);
	};
	if ($retval != 0) {
		print "Build ompp-ui failed (output follows:\n$merged\n"
	}
	
	# deploy to standard location
	remove_tree "${om_root}/html";
	dircopy 'dist', "${om_root}/html";
	
    chdir "${om_root}/html";
	
}

#done
chdir "${om_root}/Perl";
