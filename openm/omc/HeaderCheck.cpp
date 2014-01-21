#include "omc/Range.h"

namespace mm {

    typedef Range<int, -10, 10> YEAR;

    void main()
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
    }

}

