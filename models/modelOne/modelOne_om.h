// ModelOne: special model to test openM++ runtime without openM++ compiler
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#ifndef MODEL_ONE_H
#define MODEL_ONE_H

#include <forward_list>
#include <memory>
using namespace std;

#include "libopenm/omModel.h"

// Model run initialization: read input parameters
void RunInit(openm::IRunBase * const i_runBase);

// Model startup method: initialize sub-value
void ModelStartup(openm::IModel * const i_model);

// Model event loop: user code
void RunModel(openm::IModel * const i_model);

// Model shutdown method: write output tables
void ModelShutdown(openm::IModel * const i_model);

// model dimensions size
const size_t N_AGE = 4;
const size_t N_SEX = 2;
const size_t N_SALARY = 3;

// model parameters
extern thread_local reference_wrapper<const int> startSeed;
extern thread_local reference_wrapper<const double[N_AGE][N_SEX]> ageSex;
extern thread_local reference_wrapper<const int[N_SALARY][N_AGE]> salaryAge;
extern thread_local reference_wrapper<const int[N_SALARY]> salaryFull;
extern thread_local reference_wrapper<const int> baseSalary;
extern thread_local reference_wrapper<const string> filePath;

// model output tables: salary by sex
class SalarySex
{
public:
    static const size_t N_CELL = N_SALARY * (N_SEX + 1);    // number of cells, "sex" dimension has total enabled
    static const size_t N_ACC = 2;                          // number of accumulators
    static const size_t ACC_Sum = 0;                        // accumulator 0: sum
    static const size_t ACC_Count = 1;                      // accumulator 1: count
    static const char * NAME;                               // output table name: salarySex

public:
    double * acc[N_ACC];                // acc[N_ACC][N_CELL];
    forward_list<unique_ptr<double> > acc_storage;

    SalarySex(void)
    {
        auto it = acc_storage.before_begin();
        for (size_t k = 0; k < N_ACC; k++) {
            it = acc_storage.insert_after(it, unique_ptr<double>(new double[N_CELL]));
            acc[k] = it->get();
        }
    }

    // initailize accumulators
    void initialize_accumulators(void)
    {
        std::fill(acc[ACC_Sum], &acc[ACC_Sum][N_CELL], 0.0);
        std::fill(acc[ACC_Count], &acc[ACC_Count][N_CELL], 0.0);
    }
};

#endif  // MODEL_ONE_H
