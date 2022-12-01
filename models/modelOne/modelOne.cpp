// ModelOne: special model to test openM++ runtime without openM++ compiler
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

// #include <chrono>

#include "modelOne_om.h"

using namespace openm;

// Model event loop: user code
void RunModel(IModel * const i_model)
{
    theLog->logMsg("Running simulation");

    i_model->updateProgress(0);             // update sub-value progress: 0% completed

    // create microdata entities
    vector <Person> personLst;
    vector<Other> otherLst;

    if (i_model->runOptions()->isDbMicrodata || i_model->runOptions()->isCsvMicrodata || i_model->runOptions()->isTraceMicrodata)
    {
        int nEnt = startSeed * i_model->subValueCount();

        for (int k = 0; k < nEnt; k++)
        {
            personLst.push_back(Person(i_model->subValueId(), k));
        }

        for (int k = 0; k < nEnt; k++)
        {
            otherLst.push_back(Other(i_model->subValueId(), k));
        }
    }

    // enter simulation events for all entities
    for (auto & p : personLst)
    {
        p.enter_simulation(i_model);
    }
    for (auto & th : otherLst)
    {
        th.enter_simulation(i_model);
    }

    // birth events for all entities
    for (auto & p : personLst)
    {
        p.birth_event(i_model);
    }
    for (auto & th : otherLst)
    {
        th.birth_event(i_model);
    }

    // calculate salary by sex, accumulator 0: sum
    // "sex" dimension has total enabled
    size_t nCell = 0;

    if (theSalarySex) { // if SalarySex output table not suppressed

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
    }
    // update sub-value progress: 25% completed
    // second parameter of updateProgress() is "value" of type double, usually it number of cases or Time for time-based models
    i_model->updateProgress(25, (double)nCell);
    theTrace->logFormatted("Sub-value: %d progress: %d %g", i_model->subValueId(), 25, (double)nCell);  // trace output: disabled by default, use command-line or model.ini to enable it

    // calculate full or part time salary by age, accumulator 0
    // "age" dimension has total enabled
    nCell = 0;

    if (theFullAgeSalary) { // if FullAgeSalary output table not suppressed

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
    }
    // update sub-value progress: 50% completed
    i_model->updateProgress(50, (double)nCell);
    theTrace->logFormatted("Sub-value: %d progress: %d %g", i_model->subValueId(), 50, (double)nCell);  // trace output: disabled by default, use command-line or model.ini to enable it

    if (theAgeSexIncome) { // if AgeSexIncome output table not suppressed

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
    }

    // calculate old age seed, accumulator 0, scalar output table, only one cell
    // if SeedOldAge output table not suppressed
    if (theSeedOldAge) {

        for (size_t nAge = 0; nAge < N_AGE; nAge++) {
            theSeedOldAge->acc[SeedOldAge::ACC_ID][0] += (double)(i_model->subValueId() + (isOldAge[nAge] ? 10 : 1));
        }
    }
    // update sub-value progress: 75% completed
    i_model->updateProgress(75, (double)nCell);
    theTrace->logFormatted("Sub-value: %d progress: %d %g", i_model->subValueId(), 75, (double)nCell);  // trace output: disabled by default, use command-line or model.ini to enable it

    // retirement events for all entities
    for (auto & p : personLst)
    {
        p.retire_event(i_model);
    }
    for (auto & th : otherLst)
    {
        th.retire_event(i_model);
    }

#ifdef MODEL_ONE_LARGE
    // calculate large tables, if not suppressed
    if (theIncomeByYear) {
        for (size_t nAcc = 0; nAcc < theIncomeByYear->N_ACC; nAcc++) {
            nCell = 0;
            for (size_t nAge = 0; nAge < N_AGE; nAge++) {
                for (size_t nSex = 0; nSex < N_SEX; nSex++) {
                    for (size_t nSalary = 0; nSalary < N_SALARY; nSalary++) {
                        for (size_t nYear = 0; nYear < N_YEARS; nYear++) {
                            theIncomeByYear->acc[nAcc][nCell++] = nAcc + startSeed + i_model->subValueId() + salaryByYears[nAge][nSex][nSalary][nYear];
                        }
                    }
                }
            }
        }
    }
    if (theIncomeByLow) {
        for (size_t nAcc = 0; nAcc < theIncomeByLow->N_ACC; nAcc++) {
            nCell = 0;
            for (size_t nAge = 0; nAge < N_AGE; nAge++) {
                for (size_t nSex = 0; nSex < N_SEX; nSex++) {
                    for (size_t nSalary = 0; nSalary < N_SALARY; nSalary++) {
                        for (size_t nYear = 0; nYear < N_YEARS; nYear++) {
                            for (size_t nLow = 0; nLow < N_LOW; nLow++) {
                                theIncomeByLow->acc[nAcc][nCell++] = nAcc + startSeed + i_model->subValueId() + salaryByLow[nAge][nSex][nSalary][nYear][nLow];
                            }
                        }
                    }
                }
            }
        }
    }
    if (theIncomeByMiddle) {
        for (size_t nAcc = 0; nAcc < theIncomeByMiddle->N_ACC; nAcc++) {
            nCell = 0;
            for (size_t nAge = 0; nAge < N_AGE; nAge++) {
                for (size_t nSex = 0; nSex < N_SEX; nSex++) {
                    for (size_t nSalary = 0; nSalary < N_SALARY; nSalary++) {
                        for (size_t nYear = 0; nYear < N_YEARS; nYear++) {
                            for (size_t nMiddle = 0; nMiddle < N_MIDDLE; nMiddle++) {
                                theIncomeByMiddle->acc[nAcc][nCell++] = nAcc + startSeed + i_model->subValueId() + salaryByMiddle[nAge][nSex][nSalary][nYear][nMiddle];
                            }
                        }
                    }
                }
            }
        }
    }
    if (theIncomeByPeriod) {
        for (size_t nAcc = 0; nAcc < theIncomeByPeriod->N_ACC; nAcc++) {
            nCell = 0;
            for (size_t nAge = 0; nAge < N_AGE; nAge++) {
                for (size_t nSex = 0; nSex < N_SEX; nSex++) {
                    for (size_t nSalary = 0; nSalary < N_SALARY; nSalary++) {
                        for (size_t nYear = 0; nYear < N_YEARS; nYear++) {
                            for (size_t nPeriod = 0; nPeriod < N_PERIOD; nPeriod++) {
                                theIncomeByPeriod->acc[nAcc][nCell++] = nAcc + startSeed + i_model->subValueId() + salaryByPeriod[nAge][nSex][nSalary][nYear][nPeriod];
                            }
                        }
                    }
                }
            }
        }
    }
#endif  // MODEL_ONE_LARGE

    // death events for all entities
    for (auto & p : personLst)
    {
        p.death_event(i_model);
    }
    for (auto & th : otherLst)
    {
        th.death_event(i_model);
    }

    i_model->updateProgress(100);               // update sub-value progress: 100% completed
    theTrace->logMsg("Event loop completed");   // trace output: disabled by default, use command-line or model.ini to enable it

    // exit simulation events for all entities
    if (!personLst.empty() || !otherLst.empty()) theLog->logFormatted("Microdata exit simualtion, persons: %zu, others: %zu", personLst.size(), otherLst.size());

    for (auto & p : personLst)
    {
        p.exit_simulation(i_model);
    }
    for (auto & th : otherLst)
    {
        th.exit_simulation(i_model);
    }

    /*
    // trace performance test
    theLog->logFormatted("Test trace, line count: %d", startSeed);

    chrono::time_point start_time = chrono::system_clock::now();

    for (int k = 0; k < startSeed; k++) {
        if (k % (200 * 1000) == 0) theLog->logFormatted("Trace at: %d", k);

        theTrace->logFormatted("[%d]:  -1=%d +1=%d 2k=%d", k, k - 1, k + 1, k + k);
    }
    chrono::time_point end_time = chrono::system_clock::now();
    chrono::minutes m = chrono::duration_cast<chrono::minutes>(end_time - start_time);
    auto sec = chrono::duration_cast<chrono::seconds>(end_time - start_time) % chrono::minutes(1);

    theLog->logFormatted("Test trace done: %ld minutes %ld seconds", m, sec);
    */
}

// create person entity
Person::Person(int i_subId, int i_personNumber)
{
    // create unique entity id
    if (i_subId < 0 || i_subId >= 32768) throw ModelException("Invalid sub-value %d, sub-value must be in a range [0, 32767]", i_subId);
    entityId = ((uint64_t)i_subId << 48) + i_personNumber;


    // set person attributes pseudo-randomly
    age = ((i_subId + 1 + i_personNumber) * startSeed) % 100;

    // age group classification enum id's: 10='10-20' 20='20-30' 30='30-40' 40='40+'
    age_group = MIN_AGE;
    if (age >= 20) age_group = 20;
    if (age >= 30) age_group = 30;
    if (age >= 40) age_group = 40;

    sex = (i_subId + 1 + i_personNumber) % 2; // sex classification is a zero based enum

    income = (age >= 18 && age < 65) ? (i_subId + 1 + i_personNumber) * startSeed : 0.0;
    salary = 0.75 * income;

    // salary level classification enum id's: 100='L', 200='M', 300='H'
    salary_group = MIN_SALARY;
    if (salary >= 54000) salary_group = 200;
    if (salary >= 98000) salary_group = 300;

    full_time = (age >= 18 && age < 65) ? jobKind::fullTime : jobKind::partTime;

    is_old_age = age >= 65;

    pension = 0.0;
    if (is_old_age) {
        pension = ((i_subId + 1 + i_personNumber) * startSeed) / 5.0;
        if (pension < 23000) pension = 23000;
        if (age < 75) salary = 0.3 * pension;
        income = pension + salary;
    }

    // calculate entity crc-64
    size_t nSize = sizeof(i_subId) + sizeof(i_personNumber);

    for (size_t k = 0; k < ENTITY_NAME_SIZE_ARR_LEN; k++)
    {
        if (EntityNameSizeArr[k].entityId == entityKind) nSize += EntityNameSizeArr[k].size;
    }

    unique_ptr<uint8_t> btArr(new uint8_t[nSize]);
    uint8_t * pVal = btArr.get();

    size_t nOff = 0;
    nOff = memCopyTo(pVal, nOff, &i_subId, sizeof(i_subId));
    nOff = memCopyTo(pVal, nOff, &i_personNumber, sizeof(i_personNumber));

    for (size_t k = 0; k < ENTITY_NAME_SIZE_ARR_LEN; k++)
    {
        if (EntityNameSizeArr[k].entityId == entityKind) {
            nOff = memCopyTo(pVal, nOff, (reinterpret_cast<uint8_t *>(this) + EntityNameSizeArr[k].offset), EntityNameSizeArr[k].size);
        }
    }

    crc = xz_crc64(pVal, nSize, 0);
}

// Person enter simulation event
void Person::enter_simulation(IModel * const i_model)
{
    i_model->writeCsvMicrodata(entityKind, entityId, OM_SS_EVENT_ID, true, this);
}

// Person birth event
void Person::birth_event(IModel * const i_model)
{
    i_model->writeCsvMicrodata(entityKind, entityId, BIRTH_EVENT_ID, true, this);
}

// Person retirement event
void Person::retire_event(IModel * const i_model)
{
    i_model->writeCsvMicrodata(entityKind, entityId, RETIRE_EVENT_ID, true, this);
}

// Person death event
void Person::death_event(IModel * const i_model)
{
    i_model->writeCsvMicrodata(entityKind, entityId, DEATH_EVENT_ID, true, this);
}

// Person exit simulation event
void Person::exit_simulation(IModel * const i_model)
{
    // write into database and into csv
    i_model->writeDbMicrodata(entityKind, entityId, this);
    i_model->writeCsvMicrodata(entityKind, entityId, OM_SS_EVENT_ID, true, this);
}

// create Other entity
Other::Other(int i_subId, int i_otherNumber)
{
    // create unique entity id
    if (i_subId < 0 || i_subId >= 32768) throw ModelException("Invalid sub-value %d, sub-value must be in a range [0, 32767]", i_subId);
    entityId = ((uint64_t)i_subId << 48) + i_otherNumber;

    // set attributes pseudo-randomly
    age = ((i_subId + 21 + i_otherNumber) * startSeed) % 100;

    // age group classification enum id's: 10='10-20' 20='20-30' 30='30-40' 40='40+'
    age_group = MIN_AGE;
    if (age >= 20) age_group = 20;
    if (age >= 30) age_group = 30;
    if (age >= 40) age_group = 40;

    income = (age >= 18 && age < 65) ? (i_subId + 21 + i_otherNumber) * startSeed : 0.0;

    // calculate entity crc-64
    size_t nSize = sizeof(i_subId) + sizeof(i_otherNumber);

    for (size_t k = 0; k < ENTITY_NAME_SIZE_ARR_LEN; k++)
    {
        if (EntityNameSizeArr[k].entityId == entityKind) nSize += EntityNameSizeArr[k].size;
    }

    unique_ptr<uint8_t> btArr(new uint8_t[nSize]);
    uint8_t * pVal = btArr.get();

    size_t nOff = 0;
    nOff = memCopyTo(pVal, nOff, &i_subId, sizeof(i_subId));
    nOff = memCopyTo(pVal, nOff, &i_otherNumber, sizeof(i_otherNumber));

    for (size_t k = 0; k < ENTITY_NAME_SIZE_ARR_LEN; k++)
    {
        if (EntityNameSizeArr[k].entityId == entityKind) {
            nOff = memCopyTo(pVal, nOff, (reinterpret_cast<uint8_t *>(this) + EntityNameSizeArr[k].offset), EntityNameSizeArr[k].size);
        }
    }

    crc = xz_crc64(pVal, nSize, 0);
}

// Other enter simulation event
void Other::enter_simulation(IModel * const i_model)
{
    i_model->writeCsvMicrodata(entityKind, entityId, OM_SS_EVENT_ID, true, this);
}

// Other birth event
void Other::birth_event(IModel * const i_model)
{
    i_model->writeCsvMicrodata(entityKind, entityId, BIRTH_EVENT_ID, true, this);
}

// Other retirement event
void Other::retire_event(IModel * const i_model)
{
    i_model->writeCsvMicrodata(entityKind, entityId, RETIRE_EVENT_ID, true, this);
}

// Other death event
void Other::death_event(IModel * const i_model)
{
    i_model->writeCsvMicrodata(entityKind, entityId, DEATH_EVENT_ID, true, this);
}

// Other exit simulation event
void Other::exit_simulation(IModel * const i_model)
{
    // write into database and into csv
    i_model->writeDbMicrodata(entityKind, entityId, this);
    i_model->writeCsvMicrodata(entityKind, entityId, OM_SS_EVENT_ID, true, this);
}
