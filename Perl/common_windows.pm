# Copyright (c) 2013-2014 OpenM++
# This code is licensed under MIT license (see LICENSE.txt for details)

# Common shared components for ompp Perl utilities, e.g. test_models.pl and friends 
# Contains functions which are specific to the Windows platform,
# or which require different implementations due to different Perl behaviour
# on Windows and Linux.


# Determine the version of Modgen from Modgen.exe
# arg0 - the file Modgen.exe
# returns - version string
sub modgen_version {
	my $modgen_exe = shift(@_);
	use Win32::Exe;
	return Win32::Exe->new($modgen_exe)->version_info->get('FileVersion');
}


# Run a SQL statement on a Jet (.mdb) database
# arg0 - the Jet database
# arg1 - the SQL statement
# arg2 - the return value from 
# returns - multi-line string
sub run_jet_statement {
	my $db = shift(@_);
	my $sql = shift(@_);
	my $result;

	use Win32::OLE;
	use Win32::OLE::Const 'Microsoft ActiveX Data Objects';

	my $sConnect = "Provider = Microsoft.Jet.OLEDB.4.0; Data source = ${db}";
	#my $sConnect = "Provider = Microsoft.ACE.OLEDB.12.0; Data source = ${db}";
	my $ADO_Conn = Win32::OLE->new('ADODB.Connection');	# creates a connection object
	my $ADO_RS = Win32::OLE->new('ADODB.Recordset');		# creates a recordset object
	$ADO_Conn->Open($sConnect);
	if (Win32::OLE->LastError()) {
		print "Fatal Error: ", Win32::OLE->LastError(), "\n";
		return 1;
	}

	$ADO_RS = $ADO_Conn->Execute($sql);
	if (Win32::OLE->LastError()) {
		print "Fatal Error: ", Win32::OLE->LastError(), "\n";
		return 1;
	}

	my $fields = $ADO_RS->Fields->count;
	while ( !$ADO_RS->EOF ) {
		for (my $field_ordinal = 0; $field_ordinal < $fields; $field_ordinal++) {
			my $value = $ADO_RS->Fields($field_ordinal)->value;
			$result .= $value;
			if ($field_ordinal < $fields - 1) {
				$result .= ","
			}
		}
		$result .= "\n";
		#my $value = $ADO_RS->Fields(0)->value;
		#$result .= "${value}\n";
		$ADO_RS->MoveNext;
	}
	
	return $result;
}

# Extract Modgen output tables from a mdb Modgen database to a folder
# arg0 - the Modgen database
# arg1 - the destination folder
# arg2 - the number of significant digits to output (optional)
# returns - 0 for success, otherwise non-zero
sub modgen_tables_to_csv
{
	my $db = shift(@_);
	my $dir = shift(@_);
	my $round_value = 0;
	my $round_prec = 0;
	if ($#_ == 0) {
		$round_prec = shift(@_) - 1;
		if ($round_prec > 0) {
			$round_value = 1;
		}
	}
	my $retval;
	
	my $suppress_margins = 1;

	if (! -d $dir) {
		if (!mkdir $dir) {
			logmsg error, "unable to create directory ${dir}";
			return 1;
		}
	}
	
	use Win32::OLE;
	use Win32::OLE::Const 'Microsoft ActiveX Data Objects';
	my $sConnect = "Provider = Microsoft.Jet.OLEDB.4.0; Data source = ${db}";
	#my $sConnect = "Provider = Microsoft.ACE.OLEDB.12.0; Data source = ${db}";
	my $ADO_Conn = Win32::OLE->new('ADODB.Connection');	# creates a connection object

	$ADO_Conn->Open($sConnect);
	if (Win32::OLE->LastError()) {
		logmsg error, "OLE", Win32::OLE->LastError();
		return 1;
	}

	my $ADO_RS = Win32::OLE->new('ADODB.Recordset');
	
	# Get all of the output table names
	my @tables;
	my @ranks;
	my @expr_positions;
	my @table_ids;
	foreach my $which ('TableDic', 'UserTableDic') {
		my $sql = "Select Name, Rank, AnalysisDimensionPosition, TableID From ${which} Where LanguageID = 0;";
		$ADO_RS = $ADO_Conn->Execute($sql);
		if (Win32::OLE->LastError()) {
			logmsg error, "OLE", Win32::OLE->LastError();
			return 1;
		}
		my $fields = $ADO_RS->Fields->count;
		# Iterate the recordset and create lists of table names and ranks
		while ( !$ADO_RS->EOF ) {
			push @tables, $ADO_RS->Fields(0)->value;
			push @ranks, $ADO_RS->Fields(1)->value;
			push @expr_positions, $ADO_RS->Fields(2)->value;
			push @table_ids, $ADO_RS->Fields(3)->value;
			$ADO_RS->MoveNext;
		}
	}
	#logmsg info, "tables", join("\n", @tables);

	# Iterate the tables
	for my $j (0..$#tables) {
		my $table = @tables[$j];
		my $rank = @ranks[$j];
		my $expr_position = @expr_positions[$j];
		my $table_id = @table_ids[$j];
		
		if (!open CSV, ">${dir}/${table}.csv") {
			logmsg error, "error opening >${dir}/${table}.csv";
		}

		# For each classification dimension of the table, determine if it has a margin
		my $sql = "
		  Select Totals
		  From TableClassDimDic
		  Where LanguageID = 0 And TableID = ${table_id}
		  ;
		";
		$ADO_RS = $ADO_Conn->Execute($sql);
		if (Win32::OLE->LastError()) {
			logmsg error, "OLE", Win32::OLE->LastError();
			return 1;
		}
		my @has_margin;
		while ( !$ADO_RS->EOF ) {
			push @has_margin, $ADO_RS->Fields(0)->value;
			$ADO_RS->MoveNext;
		}
		#logmsg info, ${table}, "has_margin", join(",", @has_margin);

		# construct permuted dimension list which puts analysis dimension last
		# construct max index list at same time
		my $dim_list;
		my $max_dim_list;
		for (my $dim = 0; $dim < $rank; ++$dim) {
			if ($dim > 0) {
				$dim_list .= ", ";
				$max_dim_list .= ", ";
			}
			
			my $permuted_dim;
			if ($dim < $expr_position) {
				# dimensions before the analysis dimension are unchanged for permuted order
				$permuted_dim = $dim;
			}
			elsif ($dim == $rank - 1) {
				# analysis dimension is last dimension for permuted order
				$permuted_dim = $expr_position;
			}
			elsif ($dim >= $expr_position) {
				# skip over the analysis dimension for permuted order
				$permuted_dim = $dim + 1;
			}

			$dim_list .= "Dim${permuted_dim}";
			$max_dim_list .= "Max(Dim${permuted_dim})";
		}
		#logmsg info, ${table}, "dim_list", $dim_list;
		#logmsg info, ${table}, "max_dim_list", $max_dim_list;

		# Determine maximum value of each dimension (to identify margin index)
		my @max_dims;
		if ($rank > 0) {
			my $sql = "Select ${max_dim_list} From ${table};";
			$ADO_RS = $ADO_Conn->Execute($sql);
			if (Win32::OLE->LastError()) {
				logmsg error, "OLE", Win32::OLE->LastError();
				return 1;
			}
			my $fields = $ADO_RS->Fields->count;
			for (my $field_ordinal = 0; $field_ordinal < $fields; $field_ordinal++) {
				my $value = $ADO_RS->Fields($field_ordinal)->value;
				push @max_dims, $value;
			}
		}
		#logmsg info, ${table}, "max_dims", join(",", @max_dims);
		
		my $sql = "Select ";
		if ($rank > 0) {
			$sql .= "${dim_list}, ";
		}
		$sql .= "Value From ${table}";
		if ($rank > 0) {
			$sql .= " Order By ${dim_list}";
		}
		$sql .= ";";
		#logmsg info, "sql", $sql;
		
		$ADO_RS = $ADO_Conn->Execute($sql);
		my $fields = $ADO_RS->Fields->count;
		# First output line contains field names
		# Note that these are not the permuted names.
		# This is to generate csv's which look like those from ompp,
		# where the analysis dimension is always last.
		for (my $dim = 0; $dim < $rank; ++$dim) {
			print CSV "Dim${dim},";
		}
		print CSV "Value\n";

		# data lines
		while ( !$ADO_RS->EOF ) {
			my $out_line;
			my $suppress_line = 0;
			for (my $field_ordinal = 0; $field_ordinal < $fields; $field_ordinal++) {
				my $value = $ADO_RS->Fields($field_ordinal)->value;
				if (length($value) && $round_value && $field_ordinal == $fields - 1) {
					$value = 0 + sprintf("%.${round_prec}e", $value);
				}
				$suppress_line = 1 if $suppress_margins && $has_margin[$field_ordinal] && $value == $max_dims[$field_ordinal];
				$out_line .= "${value}";
				if ($field_ordinal < $fields - 1) {
					$out_line .= ",";
				}
			}
			print CSV "${out_line}\n" if ! $suppress_line;
			$ADO_RS->MoveNext;
		}
		close CSV;
	}

	# Success
	return 0;
}

return 1;