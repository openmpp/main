// ModelOne: special model to test openM++ runtime without openM++ compiler
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#ifndef MODEL_ONE_H
#define MODEL_ONE_H

#include <forward_list>
#include <memory>
using namespace std;

#include "libopenm/omModel.h"

// One-time initialization, if requred by the model
void RunOnce(openm::IRunBase * const i_runBase);

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
const size_t N_FULL = 2;

// model type "full": full or part time job
enum jobKind : int
{
    fullTime = 22,
    partTime = 33
};

// model parameters
extern thread_local const int & startSeed;
extern thread_local const int & baseSalary;
extern thread_local const string & filePath;

extern thread_local double * om_value_ageSex;
extern thread_local int * om_value_salaryAge;
extern thread_local int * om_value_salaryFull;
extern thread_local bool * om_value_isOldAge;

#define ageSex      (*reinterpret_cast<const double(*)[N_AGE][N_SEX]>(om_value_ageSex))
#define salaryAge   (*reinterpret_cast<const int(*)[N_SALARY][N_AGE]>(om_value_salaryAge))
#define salaryFull  (*reinterpret_cast<const int(*)[N_SALARY]>(om_value_salaryFull))
#define isOldAge    (*reinterpret_cast<const bool(*)[N_AGE]>(om_value_isOldAge))

// model output tables
//

// salary by sex
class SalarySex
{
public:
    static const size_t N_CELL = N_SALARY * (N_SEX + 1);    // number of cells, "sex" dimension has total enabled
    static const size_t N_ACC = 2;                          // number of accumulators
    static const size_t ACC_SUM_ID = 0;                     // accumulator 0: sum
    static const size_t ACC_COUNT_ID = 1;                   // accumulator 1: count
    static constexpr const char * NAME = "salarySex";       // output table name: salarySex

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
        std::fill(acc[ACC_SUM_ID], &acc[ACC_SUM_ID][N_CELL], 0.0);
        std::fill(acc[ACC_COUNT_ID], &acc[ACC_COUNT_ID][N_CELL], 0.0);
    }
};

class FullAgeSalary
{
public:
    static const size_t N_CELL = N_FULL * (N_AGE + 1) * N_SALARY;   // number of cells, "age" dimension has total enabled
    static const size_t N_ACC = 1;                                  // number of accumulators
    static const size_t ACC_ID = 0;                                 // accumulator 0: only one accumulator in that table
    static constexpr const char * NAME = "fullAgeSalary";           // output table name: fullAgeSalary

public:
    double * acc[N_ACC];                // acc[N_ACC][N_CELL];
    forward_list<unique_ptr<double> > acc_storage;

    FullAgeSalary(void)
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
        std::fill(acc[ACC_ID], &acc[ACC_ID][N_CELL], 0.0);
    }
};

// age by sex income
class AgeSexIncome
{
public:
    static const size_t N_CELL = N_AGE * N_SEX;             // number of cells, total emuns disabled for all dimensions
    static const size_t N_ACC = 2;                          // number of accumulators
    static const size_t ACC_INCOME_ID = 0;                  // accumulator 0: income
    static const size_t ACC_OLD_AGE_ID = 1;                 // accumulator 1: income adjusted for old age
    static constexpr const char * NAME = "ageSexIncome";    // output table name: ageSexIncome

public:
    double * acc[N_ACC];                // acc[N_ACC][N_CELL];
    forward_list<unique_ptr<double> > acc_storage;

    AgeSexIncome(void)
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
        std::fill(acc[ACC_INCOME_ID], &acc[ACC_INCOME_ID][N_CELL], 0.0);
        std::fill(acc[ACC_OLD_AGE_ID], &acc[ACC_OLD_AGE_ID][N_CELL], 0.0);
    }
};

// seed for old age output table
class SeedOldAge
{
public:
    static const size_t N_CELL = 1;                     // number of cells =1 for scalar parameter
    static const size_t N_ACC = 1;                      // number of accumulators
    static const size_t ACC_ID = 0;                     // accumulator 0: seed value
    static constexpr const char * NAME = "seedOldAge";  // output table name: seedOldAge

public:
    double * acc[N_ACC];                // acc[N_ACC][N_CELL];
    forward_list<unique_ptr<double> > acc_storage;

    SeedOldAge(void)
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
        std::fill(acc[ACC_ID], &acc[ACC_ID][N_CELL], 0.0);
    }
};

// model output tables
extern thread_local unique_ptr<SalarySex> theSalarySex;         // salary by sex
extern thread_local unique_ptr<FullAgeSalary> theFullAgeSalary; // full time by age by salary bracket
extern thread_local unique_ptr<AgeSexIncome> theAgeSexIncome;   // age by sex income
extern thread_local unique_ptr<SeedOldAge> theSeedOldAge;       // age by sex income

#endif  // MODEL_ONE_H
