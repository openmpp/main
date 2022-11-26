// ModelOne: special model to test openM++ runtime without openM++ compiler
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#ifndef MODEL_ONE_H
#define MODEL_ONE_H

#include <forward_list>
#include <memory>
using namespace std;

#include "libopenm/omModel.h"
#include "libopenm/common/xz_crc64.h"

// use large input parameters and output tables
//
// #define MODEL_ONE_LARGE

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

// age, salary and full or part time classification enums are not zero based
const int MIN_AGE = 10;         // min enum id for age group classification type
const int MIN_SALARY = 100;     // min enum id for salary classification type

// model type "full": full or part time job
enum jobKind : int
{
    fullTime = 22,
    partTime = 33
};

// model parameters
extern thread_local int om_value_startSeed;
#define startSeed ((const int)om_value_startSeed)

extern thread_local int om_value_baseSalary;
#define baseSalary ((const int)om_value_baseSalary)

extern thread_local string om_value_filePath;
#define filePath ((const string)om_value_filePath)

extern thread_local double * om_value_ageSex;
extern thread_local int * om_value_salaryAge;
extern thread_local int * om_value_salaryFull;
extern thread_local bool * om_value_isOldAge;

#define ageSex          (*reinterpret_cast<const double(*)[N_AGE][N_SEX]>(om_value_ageSex))
#define salaryAge       (*reinterpret_cast<const int(*)[N_SALARY][N_AGE]>(om_value_salaryAge))
#define salaryFull      (*reinterpret_cast<const int(*)[N_SALARY]>(om_value_salaryFull))
#define isOldAge        (*reinterpret_cast<const bool(*)[N_AGE]>(om_value_isOldAge))

// large model parameters
#ifdef MODEL_ONE_LARGE

const size_t N_YEARS = 201;
const size_t N_PERIOD = 201;
const size_t N_LOW = 10;
const size_t N_MIDDLE = 30;

extern thread_local double * om_value_salaryByYears;
extern thread_local double * om_value_salaryByPeriod;
extern thread_local double * om_value_salaryByLow;
extern thread_local double * om_value_salaryByMiddle;

#define salaryByYears   (*reinterpret_cast<const double(*)[N_AGE][N_SEX][N_SALARY][N_YEARS]>(om_value_salaryByYears))
#define salaryByPeriod  (*reinterpret_cast<const double(*)[N_AGE][N_SEX][N_SALARY][N_YEARS][N_PERIOD]>(om_value_salaryByPeriod))
#define salaryByLow     (*reinterpret_cast<const double(*)[N_AGE][N_SEX][N_SALARY][N_YEARS][N_LOW]>(om_value_salaryByLow))
#define salaryByMiddle  (*reinterpret_cast<const double(*)[N_AGE][N_SEX][N_SALARY][N_YEARS][N_MIDDLE]>(om_value_salaryByMiddle))

#endif  // MODEL_ONE_LARGE

// model entities
// 

// entity events, id must be and index in events array
extern const int BIRTH_EVENT_ID;
extern const int RETIRE_EVENT_ID;
extern const int DEATH_EVENT_ID;
extern const int OM_SS_EVENT_ID;

class Person
{
public:
    uint64_t entityId = -1;             // entity id, must be unique
    static const int entityKind = 0;    // entity metadata id in database

    // entity attributes
    int age = 0;                        // person age
    int age_group = MIN_AGE;            // age group classification enum id
    int sex = 0;                        // sex is a zero classification enums
    double income = 0.0;                // person income
    double salary = 0.0;                // person salary
    int salary_group = MIN_SALARY;      // salary level classification enum id
    int full_time = jobKind::partTime;  // full or part time classification enum id
    bool is_old_age = false;            // is old age boolean
    double pension = 0.0;               // person pension
    uint64_t crc = 0;                   // entity crc64

public:
    Person(int i_subId, int i_personNumber);
    Person(void) : Person(0, 0) {};
    ~Person(void) noexcept {}

    void enter_simulation(openm::IModel * const i_model);   // enter simulation event
    void birth_event(openm::IModel * const i_model);        // birth event
    void retire_event(openm::IModel * const i_model);       // retire event
    void death_event(openm::IModel * const i_model);        // death event
    void exit_simulation(openm::IModel * const i_model);    // exit simulation event

    static thread_local Person om_null_entity;
};

class Other
{
public:
    uint64_t entityId = -1;             // entity id, must be unique
    static const int entityKind = 1;    // entity metadata id in database

    // entity attributes
    int age = 0;                        // other age
    int age_group = MIN_AGE;            // age group classification enum id
    double income = 0.0;                // other income
    uint64_t crc = 0;                   // entity crc64

public:
    Other(int i_subId, int i_otherNumber);
    Other(void) : Other(0, 0) {};
    ~Other(void) noexcept {}

    void enter_simulation(openm::IModel * const i_model);   // enter simulation event
    void birth_event(openm::IModel * const i_model);        // birth event
    void retire_event(openm::IModel * const i_model);       // retire event
    void death_event(openm::IModel * const i_model);        // death event
    void exit_simulation(openm::IModel * const i_model);    // exit simulation event

    // static empty instance, used to retrieve members offset
    static thread_local Other om_null_entity;
};

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

public:
    double * acc[N_ACC];                // acc[N_ACC][N_CELL];
    forward_list<unique_ptr<double[]> > acc_storage;

    SalarySex(void)
    {
        auto it = acc_storage.before_begin();
        for (size_t k = 0; k < N_ACC; k++) {
            it = acc_storage.insert_after(it, unique_ptr<double[]>(new double[N_CELL]));
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

public:
    double * acc[N_ACC];                // acc[N_ACC][N_CELL];
    forward_list<unique_ptr<double[]> > acc_storage;

    FullAgeSalary(void)
    {
        auto it = acc_storage.before_begin();
        for (size_t k = 0; k < N_ACC; k++) {
            it = acc_storage.insert_after(it, unique_ptr<double[]>(new double[N_CELL]));
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

public:
    double * acc[N_ACC];                // acc[N_ACC][N_CELL];
    forward_list<unique_ptr<double[]> > acc_storage;

    AgeSexIncome(void)
    {
        auto it = acc_storage.before_begin();
        for (size_t k = 0; k < N_ACC; k++) {
            it = acc_storage.insert_after(it, unique_ptr<double[]>(new double[N_CELL]));
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

public:
    double * acc[N_ACC];                // acc[N_ACC][N_CELL];
    forward_list<unique_ptr<double[]> > acc_storage;

    SeedOldAge(void)
    {
        auto it = acc_storage.before_begin();
        for (size_t k = 0; k < N_ACC; k++) {
            it = acc_storage.insert_after(it, unique_ptr<double[]>(new double[N_CELL]));
            acc[k] = it->get();
        }
    }

    // initailize accumulators
    void initialize_accumulators(void)
    {
        std::fill(acc[ACC_ID], &acc[ACC_ID][N_CELL], 0.0);
    }
};


#ifdef MODEL_ONE_LARGE

// large model output tables
//

// income by [age, sex, salary, years]: 4824 * 4 expression cells
class IncomeByYear
{
public:
    static const size_t N_CELL = N_AGE * N_SEX * N_SALARY * N_YEARS;    // number of cells, total emuns disabled for all dimensions
    static const size_t N_ACC = 2;                                      // number of native accumulators

public:
    double * acc[N_ACC];                // acc[N_ACC][N_CELL];
    forward_list<unique_ptr<double[]> > acc_storage;

    IncomeByYear(void)
    {
        auto it = acc_storage.before_begin();
        for (size_t k = 0; k < N_ACC; k++) {
            it = acc_storage.insert_after(it, unique_ptr<double[]>(new double[N_CELL]));
            acc[k] = it->get();
        }
    }

    // initailize accumulators
    void initialize_accumulators(void)
    {
        for (size_t k = 0; k < N_ACC; k++) {
            std::fill(acc[k], &acc[k][N_CELL], 0.0);
        }
    }
};

// income by [age, sex, salary, years, low]: 48240 * 4 expression cells
class IncomeByLow
{
public:
    static const size_t N_CELL = N_AGE * N_SEX * N_SALARY * N_YEARS * N_LOW;    // number of cells, total emuns disabled for all dimensions
    static const size_t N_ACC = 2;                                              // number of native accumulators

public:
    double * acc[N_ACC];                // acc[N_ACC][N_CELL];
    forward_list<unique_ptr<double[]> > acc_storage;

    IncomeByLow(void)
    {
        auto it = acc_storage.before_begin();
        for (size_t k = 0; k < N_ACC; k++) {
            it = acc_storage.insert_after(it, unique_ptr<double[]>(new double[N_CELL]));
            acc[k] = it->get();
        }
    }

    // initailize accumulators
    void initialize_accumulators(void)
    {
        for (size_t k = 0; k < N_ACC; k++) {
            std::fill(acc[k], &acc[k][N_CELL], 0.0);
        }
    }
};

// income by [age, sex, salary, years, middle]: 144720 * 4 expression cells
class IncomeByMiddle
{
public:
    static const size_t N_CELL = N_AGE * N_SEX * N_SALARY * N_YEARS * N_MIDDLE; // number of cells, total emuns disabled for all dimensions
    static const size_t N_ACC = 2;                                              // number of native accumulators

public:
    double * acc[N_ACC];                // acc[N_ACC][N_CELL];
    forward_list<unique_ptr<double[]> > acc_storage;

    IncomeByMiddle(void)
    {
        auto it = acc_storage.before_begin();
        for (size_t k = 0; k < N_ACC; k++) {
            it = acc_storage.insert_after(it, unique_ptr<double[]>(new double[N_CELL]));
            acc[k] = it->get();
        }
    }

    // initailize accumulators
    void initialize_accumulators(void)
    {
        for (size_t k = 0; k < N_ACC; k++) {
            std::fill(acc[k], &acc[k][N_CELL], 0.0);
        }
    }
};

// income by [age, sex, salary, years, middle]: 969624 * 4 expression cells
class IncomeByPeriod
{
public:
    static const size_t N_CELL = N_AGE * N_SEX * N_SALARY * N_YEARS * N_PERIOD; // number of cells, total emuns disabled for all dimensions
    static const size_t N_ACC = 2;                                              // number of native accumulators

public:
    double * acc[N_ACC];                // acc[N_ACC][N_CELL];
    forward_list<unique_ptr<double[]> > acc_storage;

    IncomeByPeriod(void)
    {
        auto it = acc_storage.before_begin();
        for (size_t k = 0; k < N_ACC; k++) {
            it = acc_storage.insert_after(it, unique_ptr<double[]>(new double[N_CELL]));
            acc[k] = it->get();
        }
    }

    // initailize accumulators
    void initialize_accumulators(void)
    {
        for (size_t k = 0; k < N_ACC; k++) {
            std::fill(acc[k], &acc[k][N_CELL], 0.0);
        }
    }
};

#endif  // MODEL_ONE_LARGE

// model output tables
extern thread_local unique_ptr<SalarySex> theSalarySex;             // salary by sex
extern thread_local unique_ptr<FullAgeSalary> theFullAgeSalary;     // full time by age by salary bracket
extern thread_local unique_ptr<AgeSexIncome> theAgeSexIncome;       // age by sex income
extern thread_local unique_ptr<SeedOldAge> theSeedOldAge;           // seed for old age

// large model output tables
#ifdef MODEL_ONE_LARGE

extern thread_local unique_ptr<IncomeByYear> theIncomeByYear;       // income by age, sex, salary, year
extern thread_local unique_ptr<IncomeByLow> theIncomeByLow;         // income by age, sex, salary, year, period
extern thread_local unique_ptr<IncomeByMiddle> theIncomeByMiddle;   // income by age, sex, salary, year, low
extern thread_local unique_ptr<IncomeByPeriod> theIncomeByPeriod;   // income by age, sex, salary, year, middle

#endif  // MODEL_ONE_LARGE

#endif  // MODEL_ONE_H
