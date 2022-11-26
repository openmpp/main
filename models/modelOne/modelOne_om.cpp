// ModelOne: special model to test openM++ runtime without openM++ compiler
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "modelOne_om.h"
using namespace openm;

// model name
const char * OM_MODEL_NAME = "modelOne";

// model metadata digest: unique model key
// model metadata digest expected to be 32 digits hex
// modelOne is using special test value instead of actual digest
const char * OM_MODEL_DIGEST = "_201208171604590148_";

// model parameters
//
static vector<int> om_param_startSeed;
thread_local int om_value_startSeed = { 0 };

static vector<int> om_param_baseSalary;
thread_local int om_value_baseSalary = { jobKind::partTime };

static vector<string> om_param_filePath;
thread_local string om_value_filePath;

static vector<unique_ptr<double[]>> om_param_ageSex;
thread_local double * om_value_ageSex = nullptr;

static vector<unique_ptr<int[]>> om_param_salaryAge;
thread_local int * om_value_salaryAge = nullptr;

static vector<unique_ptr<int[]>> om_param_salaryFull;
thread_local int * om_value_salaryFull = nullptr;

static vector<unique_ptr<bool[]>> om_param_isOldAge;
thread_local bool * om_value_isOldAge = nullptr;

// model output tables
//
thread_local unique_ptr<SalarySex> theSalarySex;            // salary by sex
thread_local unique_ptr<FullAgeSalary> theFullAgeSalary;    // full time by age by salary bracket
thread_local unique_ptr<AgeSexIncome> theAgeSexIncome;      // age by sex income
thread_local unique_ptr<SeedOldAge> theSeedOldAge;          // seed old age output values

// special empty entity instances
thread_local Person Person::om_null_entity;
thread_local Other Other::om_null_entity;

// large model parameters
//
#ifdef MODEL_ONE_LARGE

static vector<unique_ptr<double[]>> om_param_salaryByYears;
thread_local double * om_value_salaryByYears = nullptr;

static vector<unique_ptr<double[]>> om_param_salaryByPeriod;
thread_local double * om_value_salaryByPeriod = nullptr;

static vector<unique_ptr<double[]>> om_param_salaryByLow;
thread_local double * om_value_salaryByLow = nullptr;

static vector<unique_ptr<double[]>> om_param_salaryByMiddle;
thread_local double * om_value_salaryByMiddle = nullptr;

// large model output tables
//
thread_local unique_ptr<IncomeByYear> theIncomeByYear;      // income by age, sex, salary, year
thread_local unique_ptr<IncomeByLow> theIncomeByLow;        // income by age, sex, salary, year, period
thread_local unique_ptr<IncomeByMiddle> theIncomeByMiddle;  // income by age, sex, salary, year, low
thread_local unique_ptr<IncomeByPeriod> theIncomeByPeriod;  // income by age, sex, salary, year, middle

// large modelOne development options
static bool isIncomeByYear = false;     // if true then use output table IncomeByYear
static bool isIncomeByLow = false;      // if true then use output table IncomeByLow
static bool isIncomeByMiddle = false;   // if true then use output table IncomeByMiddle
static bool isIncomeByPeriod = false;   // if true then use output table IncomeByPeriod

#endif  // MODEL_ONE_LARGE

// Model one-time initialization
void RunOnce(IRunBase * const i_runBase)
{
    theTrace->logMsg("One-time initialization");    // trace output: disabled by default, use command-line or model.ini to enable it
}

// Initialize model run: read parameters
void RunInit(IRunBase * const i_runBase)
{
    // load model parameters
    theLog->logMsg("Reading parameters");
    theModelRunState->updateProgress(0);            // update modeling process-wide progress: 0% completed

    om_param_startSeed = std::move(read_om_parameter<int>(i_runBase, "StartingSeed"));
    om_param_ageSex = std::move(read_om_parameter<double>(i_runBase, "ageSex", N_AGE * N_SEX));
    om_param_salaryAge = std::move(read_om_parameter<int>(i_runBase, "salaryAge", N_SALARY * N_AGE));
    om_param_salaryFull = std::move(read_om_parameter<int>(i_runBase, "salaryFull", N_SALARY));
    om_param_baseSalary = std::move(read_om_parameter<int>(i_runBase, "baseSalary"));
    om_param_filePath = std::move(read_om_parameter<string>(i_runBase, "filePath"));
    om_param_isOldAge = std::move(read_om_parameter<bool>(i_runBase, "isOldAge", N_AGE));

#ifdef MODEL_ONE_LARGE

    om_param_salaryByYears = std::move(read_om_parameter<double>(i_runBase, "salaryByYears", N_AGE * N_SEX * N_SALARY * N_YEARS));
    om_param_salaryByPeriod = std::move(read_om_parameter<double>(i_runBase, "salaryByPeriod", N_AGE * N_SEX * N_SALARY * N_YEARS * N_PERIOD));
    om_param_salaryByLow = std::move(read_om_parameter<double>(i_runBase, "salaryByLow", N_AGE * N_SEX * N_SALARY * N_YEARS * N_LOW));
    om_param_salaryByMiddle = std::move(read_om_parameter<double>(i_runBase, "salaryByMiddle", N_AGE * N_SEX * N_SALARY * N_YEARS * N_MIDDLE));

#endif  // MODEL_ONE_LARGE

    /*
    // read model development run options from model.ini
    //
    // [EventTrace]
    // ReportStyle = readable          ; string option
    // ShowScheduling = yes            ; true if: "yes", "1", "true" or empty value, false if missing
    // MinimumTime = 1234.56           ; double value, default: -inf
    // MaximumTime =                   ; double value, default: +inf
    // LineCount = 4321                ; integer
    // EntityId = 1234567890123456789  ; long long integer
    // SelectedEvents = e1,e2,e3       ; comma separated list of event names

    // read model development run options from model.ini
    string rptStyle = i_runBase->strOption("EventTrace.ReportStyle");
    bool isShowSch = i_runBase->boolOption("EventTrace.ShowScheduling");
    double minTime = i_runBase->doubleOption("EventTrace.MinimumTime", -numeric_limits<double>::infinity());
    double maxTime = i_runBase->doubleOption("EventTrace.MaximumTime", numeric_limits<double>::infinity());
    int lineCount = i_runBase->intOption("EventTrace.LineCount", 0);
    long long entityId = i_runBase->longOption("EventTrace.EntityId", 0);
    list<string> evtList = splitCsv(i_runBase->strOption("EventTrace.SelectedEvents"));

    if (!i_runBase->isOptionExist("EventTrace.ReportStyle")) {
        // if report style option is not specified at all
    }

    // get a copy of all options
    vector<pair<string, string>> kvLst = i_runBase->allOptions();
    */

#ifdef MODEL_ONE_LARGE

    // check modelOne development options
    isIncomeByYear = i_runBase->boolOption("LargeOutput.incomeByYear");
    isIncomeByLow = i_runBase->boolOption("LargeOutput.incomeByLow");
    isIncomeByMiddle = i_runBase->boolOption("LargeOutput.incomeByMiddle");
    isIncomeByPeriod = i_runBase->boolOption("LargeOutput.incomeByPeriod");

#endif  // MODEL_ONE_LARGE
}

// Model startup method: initialize sub-value
void ModelStartup(IModel * const i_model)
{
    // trace output: disabled by default, use command-line or model.ini to enable it
    // for example: -OpenM.TraceToConsole true
    theTrace->logFormatted("Start model sub-value %d", i_model->subValueId());

    // bind parameters reference thread local values
    // at this point parameter values undefined and cannot be used by the model
    //
    om_value_startSeed = om_param_startSeed[i_model->parameterSubValueIndex("StartingSeed")];
    om_value_ageSex = om_param_ageSex[i_model->parameterSubValueIndex("ageSex")].get();
    om_value_salaryAge = om_param_salaryAge[i_model->parameterSubValueIndex("salaryAge")].get();
    om_value_salaryFull = om_param_salaryFull[i_model->parameterSubValueIndex("salaryFull")].get();
    om_value_baseSalary = om_param_baseSalary[i_model->parameterSubValueIndex("baseSalary")];
    om_value_filePath = om_param_filePath[i_model->parameterSubValueIndex("filePath")];
    om_value_isOldAge = om_param_isOldAge[i_model->parameterSubValueIndex("isOldAge")].get();

#ifdef MODEL_ONE_LARGE

    om_value_salaryByYears = om_param_salaryByYears[i_model->parameterSubValueIndex("salaryByYears")].get();
    om_value_salaryByPeriod = om_param_salaryByPeriod[i_model->parameterSubValueIndex("salaryByPeriod")].get();
    om_value_salaryByLow = om_param_salaryByLow[i_model->parameterSubValueIndex("salaryByLow")].get();
    om_value_salaryByMiddle = om_param_salaryByMiddle[i_model->parameterSubValueIndex("salaryByMiddle")].get();

#endif  // MODEL_ONE_LARGE
    //
    // parameters ready now and can be used by the model

    // clear existing output table(s) - release memory if allocated by previous run
    if (!i_model->isSuppressed("salarySex")) theSalarySex.reset(new SalarySex());
    if (!i_model->isSuppressed("fullAgeSalary")) theFullAgeSalary.reset(new FullAgeSalary());
    if (!i_model->isSuppressed("ageSexIncome")) theAgeSexIncome.reset(new AgeSexIncome());
    if (!i_model->isSuppressed("seedOldAge")) theSeedOldAge.reset(new SeedOldAge());

#ifdef MODEL_ONE_LARGE

    if (isIncomeByYear && !i_model->isSuppressed("incomeByYear")) theIncomeByYear.reset(new IncomeByYear());
    if (isIncomeByLow && !i_model->isSuppressed("incomeByLow")) theIncomeByLow.reset(new IncomeByLow());
    if (isIncomeByMiddle && !i_model->isSuppressed("incomeByMiddle")) theIncomeByMiddle.reset(new IncomeByMiddle());
    if (isIncomeByPeriod && !i_model->isSuppressed("incomeByPeriod")) theIncomeByPeriod.reset(new IncomeByPeriod());

#endif  // MODEL_ONE_LARGE

    // allocate and initialize new output table(s)
    if (theSalarySex) theSalarySex->initialize_accumulators();
    if (theFullAgeSalary) theFullAgeSalary->initialize_accumulators();
    if (theAgeSexIncome) theAgeSexIncome->initialize_accumulators();
    if (theSeedOldAge) theSeedOldAge->initialize_accumulators();

#ifdef MODEL_ONE_LARGE

    if (theIncomeByYear) theIncomeByYear->initialize_accumulators();
    if (theIncomeByLow) theIncomeByLow->initialize_accumulators();
    if (theIncomeByMiddle) theIncomeByMiddle->initialize_accumulators();
    if (theIncomeByPeriod) theIncomeByPeriod->initialize_accumulators();

#endif  // MODEL_ONE_LARGE
}

// Model shutdown method: write output tables
void ModelShutdown(IModel * const i_model)
{
    // write output result tables: salarySex and fullAgeSalary sub-value accumulators
    theLog->logMsg("Writing output tables");

    if (theSalarySex) i_model->writeOutputTable("salarySex", SalarySex::N_CELL, theSalarySex->acc_storage);
    if (theFullAgeSalary) i_model->writeOutputTable("fullAgeSalary", FullAgeSalary::N_CELL, theFullAgeSalary->acc_storage);
    if (theAgeSexIncome) i_model->writeOutputTable("ageSexIncome", AgeSexIncome::N_CELL, theAgeSexIncome->acc_storage);
    if (theSeedOldAge) i_model->writeOutputTable("seedOldAge", SeedOldAge::N_CELL, theSeedOldAge->acc_storage);

#ifdef MODEL_ONE_LARGE

    if (theIncomeByYear) i_model->writeOutputTable("incomeByYear", IncomeByYear::N_CELL, theIncomeByYear->acc_storage);
    if (theIncomeByLow) i_model->writeOutputTable("incomeByLow", IncomeByLow::N_CELL, theIncomeByLow->acc_storage);
    if (theIncomeByMiddle) i_model->writeOutputTable("incomeByMiddle", IncomeByMiddle::N_CELL, theIncomeByMiddle->acc_storage);
    if (theIncomeByPeriod) i_model->writeOutputTable("incomeByPeriod", IncomeByPeriod::N_CELL, theIncomeByPeriod->acc_storage);

#endif  // MODEL_ONE_LARGE

    // release output tables memory
    if (theSalarySex) theSalarySex.reset(nullptr);
    if (theFullAgeSalary) theFullAgeSalary.reset(nullptr);
    if (theAgeSexIncome) theAgeSexIncome.reset(nullptr);
    if (theSeedOldAge) theSeedOldAge.reset(nullptr);

#ifdef MODEL_ONE_LARGE

    if (theIncomeByYear) theIncomeByYear.reset(nullptr);
    if (theIncomeByLow) theIncomeByLow.reset(nullptr);
    if (theIncomeByMiddle) theIncomeByMiddle.reset(nullptr);
    if (theIncomeByPeriod) theIncomeByPeriod.reset(nullptr);

#endif  // MODEL_ONE_LARGE
}

// entity events, id must be and index in events array
const int BIRTH_EVENT_ID = 0;
const int RETIRE_EVENT_ID = 1;
const int DEATH_EVENT_ID = 2;
const int OM_SS_EVENT_ID = 3;   // entity special event: internal system events

namespace openm
{
    // model API entries holder
    struct ModelEntryHolder
    {
        ModelEntryHolder(
            OM_RUN_ONCE_HANDLER i_RunOnceHandler,
            OM_RUN_INIT_HANDLER i_RunInitHandler,
            OM_STARTUP_HANDLER i_ModelStartupHandler,
            OM_EVENT_LOOP_HANDLER i_RunModelHandler,
            OM_SHUTDOWN_HANDLER i_ModelShutdownHandler
        )
        {
            RunOnceHandler = i_RunOnceHandler;              // link run once function
            RunInitHandler = i_RunInitHandler;              // link run init function
            ModelStartupHandler = i_ModelStartupHandler;    // link model startup function
            RunModelHandler = i_RunModelHandler;            // link user portion of model event loop
            ModelShutdownHandler = i_ModelShutdownHandler;  // link model shutdown function
        }
    };

    // set entry points to the code generated by openM++ compiler
    static ModelEntryHolder theModelEntry(RunOnce, RunInit, ModelStartup, RunModel, ModelShutdown);

#ifndef MODEL_ONE_LARGE

    // size of parameters list: number of model input parameters
    const size_t PARAMETER_NAME_ARR_LEN = 7;

    // list of model input parameters name, type and size
    const ParameterNameSizeItem parameterNameSizeArr[PARAMETER_NAME_ARR_LEN] = {
        {"StartingSeed", typeid(int), 1},
        {"ageSex", typeid(double), N_AGE * N_SEX},
        {"salaryAge", typeid(int), N_SALARY * N_AGE},
        {"salaryFull", typeid(int), N_SALARY},
        {"baseSalary", typeid(int), 1},
        {"filePath", typeid(string), 1},
        {"isOldAge", typeid(bool), N_AGE}
    };

#else   // MODEL_ONE_LARGE

    // size of parameters list: number of model input parameters
    const size_t PARAMETER_NAME_ARR_LEN = 11;

    // list of model input parameters name, type and size
    const ParameterNameSizeItem parameterNameSizeArr[PARAMETER_NAME_ARR_LEN] = {
        {"StartingSeed", typeid(int), 1},
        {"ageSex", typeid(double), N_AGE * N_SEX},
        {"salaryAge", typeid(int), N_SALARY * N_AGE},
        {"salaryFull", typeid(int), N_SALARY},
        {"baseSalary", typeid(int), 1},
        {"filePath", typeid(string), 1},
        {"isOldAge", typeid(bool), N_AGE},
        {"salaryByYears", typeid(double), N_AGE * N_SEX * N_SALARY * N_YEARS},
        {"salaryByPeriod", typeid(double), N_AGE * N_SEX * N_SALARY * N_YEARS * N_PERIOD},
        {"salaryByLow", typeid(double), N_AGE * N_SEX * N_SALARY * N_YEARS * N_LOW},
        {"salaryByMiddle", typeid(double), N_AGE * N_SEX * N_SALARY * N_YEARS * N_MIDDLE}
    };

#endif  // MODEL_ONE_LARGE

    // Entity instances to calculate attributes member offset
    #define om_PersonEntityNull (Person::om_null_entity)
    static const uint8_t * om_PersonEntityNullThis = reinterpret_cast<const uint8_t *>(&om_PersonEntityNull);

    #define om_OtherEntityNull (Other::om_null_entity)
    static const uint8_t * om_OtherEntityNullThis = reinterpret_cast<const uint8_t *>(&om_OtherEntityNull);

    // size of entity attributes list: all attributes of all entities
    const size_t ENTITY_NAME_SIZE_ARR_LEN = 14;

    // list of entity attributes name, type, size and member offset
    const EntityNameSizeItem EntityNameSizeArr[ENTITY_NAME_SIZE_ARR_LEN] = {
        { 0, "Person", 0, "Age",         typeid(int),      sizeof(int),      reinterpret_cast<const uint8_t *>(&(om_PersonEntityNull.age)) - om_PersonEntityNullThis },
        { 0, "Person", 1, "AgeGroup",    typeid(int),      sizeof(int),      reinterpret_cast<const uint8_t *>(&(om_PersonEntityNull.age_group)) - om_PersonEntityNullThis },
        { 0, "Person", 2, "Sex",         typeid(int),      sizeof(int),      reinterpret_cast<const uint8_t *>(&(om_PersonEntityNull.sex)) - om_PersonEntityNullThis },
        { 0, "Person", 3, "Income",      typeid(double),   sizeof(double),   reinterpret_cast<const uint8_t *>(&(om_PersonEntityNull.income)) - om_PersonEntityNullThis },
        { 0, "Person", 4, "Salary",      typeid(double),   sizeof(double),   reinterpret_cast<const uint8_t *>(&(om_PersonEntityNull.salary)) - om_PersonEntityNullThis },
        { 0, "Person", 5, "SalaryGroup", typeid(int),      sizeof(int),      reinterpret_cast<const uint8_t *>(&(om_PersonEntityNull.salary_group)) - om_PersonEntityNullThis },
        { 0, "Person", 6, "FullTime",    typeid(int),      sizeof(int),      reinterpret_cast<const uint8_t *>(&(om_PersonEntityNull.full_time)) - om_PersonEntityNullThis },
        { 0, "Person", 7, "IsOldAge",    typeid(bool),     sizeof(bool),     reinterpret_cast<const uint8_t *>(&(om_PersonEntityNull.is_old_age)) - om_PersonEntityNullThis },
        { 0, "Person", 8, "Pension",     typeid(double),   sizeof(double),   reinterpret_cast<const uint8_t *>(&(om_PersonEntityNull.pension)) - om_PersonEntityNullThis },
        { 0, "Person", 9, "crc",         typeid(uint64_t), sizeof(uint64_t), reinterpret_cast<const uint8_t *>(&(om_PersonEntityNull.crc)) - om_PersonEntityNullThis },
        { 1, "Other", 20, "Age",         typeid(int),      sizeof(int),      reinterpret_cast<const uint8_t *>(&(om_OtherEntityNull.age)) - om_OtherEntityNullThis },
        { 1, "Other", 21, "AgeGroup",    typeid(int),      sizeof(int),      reinterpret_cast<const uint8_t *>(&(om_OtherEntityNull.age_group)) - om_OtherEntityNullThis },
        { 1, "Other", 22, "Income",      typeid(double),   sizeof(double),   reinterpret_cast<const uint8_t *>(&(om_OtherEntityNull.income)) - om_OtherEntityNullThis },
        { 1, "Other", 23, "crc",         typeid(uint64_t), sizeof(uint64_t), reinterpret_cast<const uint8_t *>(&(om_OtherEntityNull.crc)) - om_OtherEntityNullThis }
    };

    // size of event list: all events in all entities
    const size_t EVENT_ID_NAME_ARR_LEN = 5;

    // list of events id, name
    // id must be and index in events array
    const EventIdNameItem EventIdNameArr[EVENT_ID_NAME_ARR_LEN] =
    {
        {-1, "(no event)"},
        {BIRTH_EVENT_ID,  "Birth"},
        {RETIRE_EVENT_ID,  "Retire"},
        {DEATH_EVENT_ID,  "Death"},
        {OM_SS_EVENT_ID,  "om_ss_event"}
    };

    // if microdata disabled by compiler:
    /*
    // size of entity attributes list: all attributes of all entities
    const size_t ENTITY_NAME_SIZE_ARR_LEN = 1;

    // list of entity attributes name, type, size and member offset
    const EntityNameSizeItem EntityNameSizeArr[ENTITY_NAME_SIZE_ARR_LEN] = { { 0, "", 0, "", typeid(int), sizeof(int), 0 } };

    // size of event list: all events in all entities
    const size_t EVENT_ID_NAME_ARR_LEN = 1;

    // list of events id, name
    const EventIdNameItem EventIdNameArr[EVENT_ID_NAME_ARR_LEN] =
    {
        {-1, "(no event)"}
    };
    */
}
