// ModelOne: special model to test openM++ runtime without openM++ compiler
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "modelOne_om.h"
using namespace openm;

// Model event loop: user code
void RunModel(IModel * const i_model)
{
    theLog->logMsg("Running simulation");
    i_model->updateProgress(0);             // update sub-value progress: 0% completed

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
    // update sub-value progress: 50% completed
    i_model->updateProgress(75, (double)nCell);
    theTrace->logFormatted("Sub-value: %d progress: %d %g", i_model->subValueId(), 75, (double)nCell);  // trace output: disabled by default, use command-line or model.ini to enable it

    // calculate old age seed, accumulator 0, scalar output table, only one cell
    // if SeedOldAge output table not suppressed
    if (theSeedOldAge) {

        for (size_t nAge = 0; nAge < N_AGE; nAge++) {
            theSeedOldAge->acc[SeedOldAge::ACC_ID][0] += (double)(i_model->subValueId() + (isOldAge[nAge] ? 10 : 1));
        }
    }

    i_model->updateProgress(100);               // update sub-value progress: 100% completed
    theTrace->logMsg("Event loop completed");   // trace output: disabled by default, use command-line or model.ini to enable it
}
