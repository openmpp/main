// ModelOne: special model to test openM++ runtime without openM++ compiler
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "modelOne_om.h"
using namespace openm;

// model type "full": full or part time job
enum jobKind : int
{
    fullTime = 22,
    partTime = 33
};

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
const char * SalarySex::NAME = "salarySex";
static thread_local unique_ptr<SalarySex> theSalarySex;         // salary by sex

const char * FullAgeSalary::NAME = "fullAgeSalary";
static thread_local unique_ptr<FullAgeSalary> theFullAgeSalary; // full time by age by salary bracket

const char * AgeSexIncome::NAME = "ageSexIncome";
static thread_local unique_ptr<AgeSexIncome> theAgeSexIncome;   // age by sex income

const char * SeedOldAge::NAME = "seedOldAge";
static thread_local unique_ptr<SeedOldAge> theSeedOldAge;       // age by sex income

// Model event loop: user code
void RunModel(IModel * const i_model)
{
    theLog->logMsg("Running simulation");
    i_model->updateProgress(0);             // update sub-value progress: 0% completed

    // calculate salary by sex, accumulator 0: sum
    // "sex" dimension has total enabled
    size_t nCell = 0;

    for (size_t nSalary = 0; nSalary < N_SALARY; nSalary++) {
        for (size_t nSex = 0; nSex < N_SEX; nSex++) {
            for (size_t nAge = 0; nAge < N_AGE; nAge++) {

                theSalarySex->acc[SalarySex::ACC_SUM_ID][nCell] +=
                    ((double)salaryAge[nSalary][nAge]) * 
                    ageSex[nAge][nSex];
            }
            nCell++;
        }
        // "sex" dimension has total enabled: make test value for "total"
        theSalarySex->acc[SalarySex::ACC_SUM_ID][nCell++] =
            ((double)(nSalary + 800 * i_model->subValueId() + 1));
    }

    // calculate salary by sex, accumulator 1: count
    // "sex" dimension has total enabled
    nCell = 0;

    for (size_t nSalary = 0; nSalary < N_SALARY; nSalary++) {
        for (size_t nSex = 0; nSex < N_SEX; nSex++) {

            theSalarySex->acc[SalarySex::ACC_COUNT_ID][nCell++] = (double)(nSalary + nSex + i_model->subValueId());
        }
        // "sex" dimension has total enabled: make test value for "total"
        theSalarySex->acc[SalarySex::ACC_COUNT_ID][nCell++] = (double)(nSalary + 800 + i_model->subValueId());
    }

    // update sub-value progress: 25% completed
    // second parameter of updateProgress() is "value" of type double, usually it number of cases or Time for time-based models
    i_model->updateProgress(25, (double)nCell);
    theTrace->logFormatted("Sub-value: %d progress: %d %g", i_model->subValueId(), 25, (double)nCell);  // trace output: disabled by default, use command-line or model.ini to enable it

    // calculate full or part time salary by age, accumulator 0
    // "age" dimension has total enabled
    nCell = 0;

    for (size_t nFull = 0; nFull < N_FULL; nFull++) {
        for (size_t nAge = 0; nAge < N_AGE + 1; nAge++) {
            for (size_t nSalary = 0; nSalary < N_SALARY; nSalary++) {

                // "age" dimension has total enabled
                if (nAge < N_AGE) {
                    theFullAgeSalary->acc[FullAgeSalary::ACC_ID][nCell++] =
                        ((salaryFull[nSalary] == jobKind::fullTime) ? 2.0 : 1.0) *
                        ((double)salaryAge[nSalary][nAge]);
                }
                else {
                    // make test value for age "total"
                    theFullAgeSalary->acc[FullAgeSalary::ACC_ID][nCell++] = (double)((nFull + nAge + nSalary) * (1 + i_model->subValueId()));
                }
            }
        }
    }

    // update sub-value progress: 50% completed
    i_model->updateProgress(50, (double)nCell);
    theTrace->logFormatted("Sub-value: %d progress: %d %g", i_model->subValueId(), 50, (double)nCell);  // trace output: disabled by default, use command-line or model.ini to enable it

    // calculate age by sex, accumulator 0: income
    nCell = 0;
    for (size_t nAge = 0; nAge < N_AGE; nAge++) {
        for (size_t nSex = 0; nSex < N_SEX; nSex++) {

            theAgeSexIncome->acc[AgeSexIncome::ACC_INCOME_ID][nCell++] = ageSex[nAge][nSex] + (double)i_model->subValueId();
        }
    }

    // calculate age by sex, accumulator 1: age adjusted income
    nCell = 0;
    for (size_t nAge = 0; nAge < N_AGE; nAge++) {
        for (size_t nSex = 0; nSex < N_SEX; nSex++) {

            theAgeSexIncome->acc[AgeSexIncome::ACC_OLD_AGE_ID][nCell++] = 
                (ageSex[nAge][nSex] + i_model->subValueId()) *
                (double)(i_model->subValueId() + (isOldAge[nAge] ? 10 : 1));
        }
    }

    // update sub-value progress: 50% completed
    i_model->updateProgress(75, (double)nCell);
    theTrace->logFormatted("Sub-value: %d progress: %d %g", i_model->subValueId(), 75, (double)nCell);  // trace output: disabled by default, use command-line or model.ini to enable it

    // calculate old age seed, accumulator 0, scalar output table, only one cell
    for (size_t nAge = 0; nAge < N_AGE + 1; nAge++) {
        theSeedOldAge->acc[SeedOldAge::ACC_ID][0] += (double)(i_model->subValueId() + (isOldAge[nAge] ? 10 : 1));
    }

    i_model->updateProgress(100);               // update sub-value progress: 100% completed
    theTrace->logMsg("Event loop completed");   // trace output: disabled by default, use command-line or model.ini to enable it
}

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
    theSalarySex.reset(new SalarySex());
    theFullAgeSalary.reset(new FullAgeSalary());
    theAgeSexIncome.reset(new AgeSexIncome());
    theSeedOldAge.reset(new SeedOldAge());

    // allocate and initialize new output table(s)
    theSalarySex->initialize_accumulators();
    theFullAgeSalary->initialize_accumulators();
    theSeedOldAge->initialize_accumulators();
}

// Model shutdown method: write output tables
void ModelShutdown(IModel * const i_model)
{
    // write output result tables: salarySex and fullAgeSalary sub-value accumulators
    theLog->logMsg("Writing output tables");

    i_model->writeOutputTable(SalarySex::NAME, SalarySex::N_CELL, theSalarySex->acc_storage);
    i_model->writeOutputTable(FullAgeSalary::NAME, FullAgeSalary::N_CELL, theFullAgeSalary->acc_storage);
    i_model->writeOutputTable(AgeSexIncome::NAME, AgeSexIncome::N_CELL, theAgeSexIncome->acc_storage);
    i_model->writeOutputTable(SeedOldAge::NAME, SeedOldAge::N_CELL, theSeedOldAge->acc_storage);
}
