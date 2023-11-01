/**
* @file    globals1.h
* Global functions available to all models (late).
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include <limits>
#include "libopenm/omModel.h" // for openm::IRunBase

// Undefined value for a table cell
const double UNDEF_VALUE = std::numeric_limits<double>::quiet_NaN();

// The following global functions are defined in framework modules,
// as indicated. They are mostly documented in the Modgen Developer's Guide.

// defined in use/common.ompp
extern void ModelExit(const char * msg);
extern int get_simulation_members();
extern int get_simulation_member();
extern long long get_combined_seed();
extern int get_next_entity_id();
extern void SetMaxTime(double max_value);
extern void StartEventTrace();
extern void StopEventTrace();
extern int GetThreadNumber();
extern int GetThreads();
extern void SetPopulation(long lPopulation);
extern long GetPopulation();
double PieceLinearLookup(double x, const double *ax, const double *ay, int n);
double PieceLinearLookup(double x, const double *axy, int n);
bool IsUndefined(double x);
void report_simulation_progress(int member, int percent);
void report_simulation_progress_beat(int percent, double value = 0.0);
void ProgressMessage(const char *msg);
void TimeReport(double dTime);
void initialize_model_streams();
void handle_streams_exceeded(int strm, int max_model_stream);
void process_trace_options(openm::IRunBase* const i_runBase);

// if defined by model developer then it is called before each model run started
void ProcessDevelopmentOptions(const openm::IRunOptions * const i_options);

// defined in use/random/random_*.ompp
typedef std::vector<std::string> random_state; // type used to store state of all streams
extern void new_streams();
extern void delete_streams();
extern void initialize_stream(int model_stream, int member, long seed);
random_state serialize_random_state();
void deserialize_random_state(const random_state & rs);
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

// defined in use/calendar/leapless_modgen.ompp
int LeaplessCalendarToDay(int year, int month_in_year, int day_in_month);
void LeaplessDayToCalendar(int day_number, int *p_year, int *p_month_in_year, int *p_day_in_month, int *p_day_in_week);
bool LeaplessIsLeapYear(int year);
int LeaplessTimeToDay(double exact_time);
int LeaplessTimeToDay(double exact_time, int *p_day_boundary);
double LeaplessDayToTime(int day_number);
double LeaplessDayLength();
void LeaplessTimeToCalendar(double exact_time, int *p_year, int *p_month_in_year, int *p_day_in_month, int *p_day_in_week);
double LeaplessCalendarToTime(int year, int month_in_year, int day_in_month);

// defined in use/case_based/case_based.ompp
//         or use/time_based/time_based.ompp
extern void RunSimulation(int mem_id, int mem_count, openm::IModel * const i_model);
extern void before_presimulation(int mem_id, int mem_count);
extern void after_presimulation();

// defined in use/case_based/case_based.ompp
extern long long GetCaseID();
extern double GetCaseSeed();
extern int GetCaseSample();
extern int GetUserTableSubSample();
extern long long GetAllCases();
extern int GetSubSamples();
extern void SetCaseWeight(double weight);
extern void SetCaseWeight(double weight, double weight2);

// defined in use/time_based/time_based.ompp
extern int GetReplicate();
extern int GetUserTableReplicate();
extern int GetReplicates();
extern double SIMULATION_END();

// defined in use/case_based/case_based_common.ompp
//         or use/time_based/time_based_common.ompp
void Set_actor_weight(double weight);
void Set_actor_subsample_weight(double weight);

// defined in use/case_based/scaling_*.ompp
//         or use/time_based/scaling_*.ompp
extern double population_scaling_factor();

// defined in generated code om_definitions.cpp
extern const char * ModelString(const char * string_name);
namespace omr {
    extern const int event_name_to_id(const std::string event_name);
    extern const char* event_id_to_name(int event_id);
    extern const int member_name_to_id(const std::string member_name);
    extern const int table_name_to_id(const std::string name);
}