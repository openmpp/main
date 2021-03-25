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
static thread_local int om_value_startSeed = { 0 };
thread_local const int & startSeed = om_value_startSeed;

static vector<int> om_param_baseSalary;
static thread_local int om_value_baseSalary = { jobKind::partTime };
thread_local const int & baseSalary = om_value_baseSalary;

static vector<string> om_param_filePath;
static thread_local string om_value_filePath;
thread_local const string & filePath = om_value_filePath;

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
thread_local unique_ptr<SalarySex> theSalarySex;         // salary by sex
thread_local unique_ptr<FullAgeSalary> theFullAgeSalary; // full time by age by salary bracket
thread_local unique_ptr<AgeSexIncome> theAgeSexIncome;   // age by sex income
thread_local unique_ptr<SeedOldAge> theSeedOldAge;       // age by sex income

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
    //
    // parameters ready now and can be used by the model

    // clear existing output table(s) - release memory if allocated by previous run
    if (!i_model->isSuppressed(SalarySex::NAME)) theSalarySex.reset(new SalarySex());
    if (!i_model->isSuppressed(FullAgeSalary::NAME)) theFullAgeSalary.reset(new FullAgeSalary());
    if (!i_model->isSuppressed(AgeSexIncome::NAME)) theAgeSexIncome.reset(new AgeSexIncome());
    if (!i_model->isSuppressed(SeedOldAge::NAME)) theSeedOldAge.reset(new SeedOldAge());

    // allocate and initialize new output table(s)
    if (theSalarySex) theSalarySex->initialize_accumulators();
    if (theFullAgeSalary) theFullAgeSalary->initialize_accumulators();
    if (theAgeSexIncome) theAgeSexIncome->initialize_accumulators();
    if (theSeedOldAge) theSeedOldAge->initialize_accumulators();
}

// Model shutdown method: write output tables
void ModelShutdown(IModel * const i_model)
{
    // write output result tables: salarySex and fullAgeSalary sub-value accumulators
    theLog->logMsg("Writing output tables");

    if (theSalarySex) i_model->writeOutputTable(SalarySex::NAME, SalarySex::N_CELL, theSalarySex->acc_storage);
    if (theFullAgeSalary) i_model->writeOutputTable(FullAgeSalary::NAME, FullAgeSalary::N_CELL, theFullAgeSalary->acc_storage);
    if (theAgeSexIncome) i_model->writeOutputTable(AgeSexIncome::NAME, AgeSexIncome::N_CELL, theAgeSexIncome->acc_storage);
    if (theSeedOldAge) i_model->writeOutputTable(SeedOldAge::NAME, SeedOldAge::N_CELL, theSeedOldAge->acc_storage);

    if (theSalarySex) theSalarySex.release();
    if (theFullAgeSalary) theFullAgeSalary.release();
    if (theAgeSexIncome) theAgeSexIncome.release();
    if (theSeedOldAge) theSeedOldAge.release();
}

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
}
