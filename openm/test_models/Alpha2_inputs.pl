# Parameter values included by inputs.pl

%parameter_values = (
    # PersonCore.mpp
	MortalityHazard         =>  0.013  ,

    # Happiness.mpp
	HappinessReversalHazard =>  0.10 ,

    # SyntaxProbes.mpp
	a_long                  =>  42 ,
	a_float                 =>  0.10 ,
	a_double                =>  0.20 ,
	a_switch                =>  1 ,
	a_sex                   =>  0 ,
	a_years                 =>  -1 ,
	a_age_group             =>  0 ,

    # Extensions.ompp
	a_schar                 =>  -1 ,
	a_llong                 => -5000000000,
	a_ullong                => 5000000000,
	a_ldouble               => 3.14159,

  	b_schar                 => -1 ,
  	b_llong                 => 57 ,
  	b_ullong                => 58 ,
  	b_ldouble               => 2.71828 ,

    c_double                => 42.000001 ,

    a_time                  => 2013.1234,
    a_counter               => 5,
    a_integer               => -5,
    a_real                  => 234.987,

    Something               => [1.01, 2.02]

);