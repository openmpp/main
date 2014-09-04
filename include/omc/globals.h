/**
* @file    globals.h
* Global functions available to all models.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

// The following global functions are defined in framework modules,
// as indicated. They are documented in the Modgen Developer's Guide.

// defined in use/common.ompp
extern void SetMaxTime(double max_value);
extern void StartEventTrace();
extern void StopEventTrace();
extern int GetThreadNumber();
extern int GetThreads();

// defined in use/random/random_modgen.ompp
extern double RandUniform(int strm);
extern double RandNormal(int strm);
extern double RandLogistic(int strm);

// defined in use/case_based/case_based_modgen.ompp
extern double GetCaseSeed();
extern int GetCaseSample();
extern long long GetAllCases();
extern int GetSubSamples();

// defined in use/time_based/time_based_modgen.ompp
extern int GetReplicate();
extern int GetReplicates();
