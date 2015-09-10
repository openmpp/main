/**
* @file    globals1.h
* Global functions available to all models (late).
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include <limits>

// Undefined value for a table cell
const double UNDEF_VALUE = numeric_limits<double>::quiet_NaN();

// The following global functions are defined in framework modules,
// as indicated. They are mostly documented in the Modgen Developer's Guide.

// defined in use/common.ompp
extern void SetMaxTime(double max_value);
extern void StartEventTrace();
extern void StopEventTrace();
extern int GetThreadNumber();
extern int GetThreads();
extern void SetPopulation(long lPopulation);
extern void signal_exit_simulation_all();
double PieceLinearLookup(double x, const double *ax, const double *ay, int n);
bool IsUndefined(double x);
void WarningMsg(const char *fmt, ...);
void ProgressMsg(const char *msg);
void TimeReport(double dTime);

// defined in use/random/random_modgen.ompp
extern void initialize_stream(int model_stream, long seed);
extern double RandUniform(int strm);
extern double RandNormal(int strm);
extern double RandLogistic(int strm);

// defined and documented in use/calendar/leapless.ompp
int date_to_day(int year, int month_in_year, int day_in_month);
void day_to_date(int day_number, int& year, int& month_in_year, int& day_in_month, int& day_in_week);
bool is_leap_year(int year);
int time_to_day(double exact_time);
int time_to_day(double exact_time, bool& day_boundary);
double day_to_time(int day_number);
double day_length();
void time_to_date(double exact_time, int& year, int& month_in_year, int& day_in_month, int& day_in_week);
double date_to_time(int year, int month_in_year, int day_in_month);

// defined in use/calendar/modgen_leapless.ompp
int LeaplessCalendarToDay(int year, int month_in_year, int day_in_month);
void LeaplessDayToCalendar(int day_number, int *p_year, int *p_month_in_year, int *p_day_in_month, int *p_day_in_week);
bool LeaplessIsLeapYear(int year);
int LeaplessTimeToDay(double exact_time);
int LeaplessTimeToDay(double exact_time, int *p_day_boundary);
double LeaplessDayToTime(int day_number);
double LeaplessDayLength();
void LeaplessTimeToCalendar(double exact_time, int *p_year, int *p_month_in_year, int *p_day_in_month, int *p_day_in_week);
double LeaplessCalendarToTime(int year, int month_in_year, int day_in_month);

// defined in use/case_based/case_based_modgen.ompp
//         or use/time_based/time_based_modgen.ompp
extern void RunSimulation(int mem_id, int mem_count);

// defined in use/case_based/case_based_modgen.ompp
extern long long GetCaseID();
extern double GetCaseSeed();
extern int GetCaseSample();
extern int GetUserTableSubSample();
extern long long GetAllCases();
extern int GetSubSamples();

// defined in use/time_based/time_based_modgen.ompp
extern int GetReplicate();
extern int GetUserTableReplicate();
extern int GetReplicates();
extern double SIMULATION_END();

// defined in use/case_based/scaling_*.ompp
//         or use/time_based/scaling_*.ompp
extern double population_scaling_factor();
