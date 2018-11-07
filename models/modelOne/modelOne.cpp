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
thread_local int om_value_startSeed = { 0 };

static vector<unique_ptr<double[]>> om_param_ageSex; 
thread_local double * om_value_ageSex = nullptr;

static vector<unique_ptr<int[]>> om_param_salaryAge; 
thread_local int * om_value_salaryAge = nullptr;

static vector<unique_ptr<int[]>> om_param_salaryFull; 
thread_local int * om_value_salaryFull = nullptr;

static vector<int> om_param_baseSalary;
thread_local int om_value_baseSalary = { jobKind::partTime };

static vector<string> om_param_filePath;
thread_local string om_value_filePath;

static vector<unique_ptr<bool[]>> om_param_isOldAge;
thread_local bool * om_value_isOldAge = nullptr;

// model output tables: salary by sex
const char * SalarySex::NAME = "salarySex";
static thread_local unique_ptr<SalarySex> theSalarySex; // salary by sex

// Model event loop: user code
void RunModel(IModel * const i_model)
{
    theLog->logMsg("Running Simulation");
    i_model->updateProgress(0);             // update sub-value progress: 0% completed

    // calculte salary by sex, accumulator 0: sum
    // "sex" dimension has total enabled
    size_t nCell = 0;

    for (size_t nSalary = 0; nSalary < N_SALARY; nSalary++) {
        for (size_t nSex = 0; nSex < N_SEX; nSex++) {
            for (size_t nAge = 0; nAge < N_AGE; nAge++) {

                // make some test value
                theSalarySex->acc[SalarySex::ACC_SUM_ID][nCell] +=
                    ((double)salaryAge[nSalary][nAge]) * 
                    ageSex[nAge][nSex] * 
                    (double)(i_model->subValueId() + isOldAge[nAge] ? 20 : 1);
            }
            nCell++;
        }
        // "sex" dimension has total enabled: make test value for "total"
        theSalarySex->acc[SalarySex::ACC_SUM_ID][nCell++] =
            ((double)(nSalary + 800 * i_model->subValueId() + 1));
    }

    // update sub-value progress: 50% completed
    // second parameter of updateProgress() is "value" of type double, usually it number of cases or Time for time-based models
    i_model->updateProgress(50, (double)nCell);
    theTrace->logFormatted("Sub-value: %d progress: %d %g", i_model->subValueId(), 50, (double)nCell);  // trace output: disabled by default, use command-line or model.ini to enable it

    // calculte salary by sex, accumulator 1: count
    // "sex" dimension has total enabled
    nCell = 0;

    for (size_t nSalary = 0; nSalary < N_SALARY; nSalary++) {

        int nFullBonus = (salaryFull[nSalary] == jobKind::fullTime) ? 1 : 0;

        for (size_t nSex = 0; nSex < N_SEX; nSex++) {

            // make some test value
            theSalarySex->acc[SalarySex::ACC_COUNT_ID][nCell++] = 
                (double)(nSalary + nSex + i_model->subValueId() + 1 + nFullBonus);
        }
        // "sex" dimension has total enabled: make test value for "total"
        theSalarySex->acc[SalarySex::ACC_COUNT_ID][nCell++] =
            (double)(nSalary + 800 + i_model->subValueId() + 1 + nFullBonus);
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
    theLog->logMsg("Reading Parameters");
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
    theTrace->logMsg("Start model sub-value");      // trace output: disabled by default, use command-line or model.ini to enable it

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

    // allocate and initialize new output table(s)
    theSalarySex->initialize_accumulators();
}

// Model shutdown method: write output tables
void ModelShutdown(IModel * const i_model)
{
    // write output result tables: salarySex sub-value accumulators
    theLog->logMsg("Writing Output Tables");

    i_model->writeOutputTable(SalarySex::NAME, SalarySex::N_CELL, theSalarySex->acc_storage);
}
