#include "omc/Range.h"

namespace mm {

    typedef Range<int, -1, 4> YEAR;

    void Tester()
    {
        YEAR year1;
        year1 = 1;

        YEAR year2 = 2;

        YEAR year3(4);

        year3++;

        year3 = year2 - year1;

        double a = year3;
        year3 = 3.4;

        year3.min;

        year1.size;

        YEAR::min;

        for (YEAR my_year = YEAR::min; my_year <= YEAR::max; my_year++) {
            my_year;
        }

        for (YEAR my_year = my_year.min; my_year <= my_year.max; my_year++) {
            my_year;
            int j = my_year.index();
        }

        int sum = 0;
        for (int k : YEAR::indices) {
            sum += k;
            //k.pos();
        }

        int n = YEAR::size;

        double Something[YEAR::size];
        //double Something[] = new dou

        double Something2[year1.size];

        Something;
        Something2;

    }

}
