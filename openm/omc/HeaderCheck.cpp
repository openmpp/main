//#include "omc/Range.h"
//#include "omc/Partition.h"
//
//namespace mm {
//
//    typedef double real;
//
//    typedef Range<int, -1, 4> YEAR;
//
//    void Tester1()
//    {
//        YEAR year1;
//        year1 = 1;
//
//        YEAR year2 = 2;
//
//        YEAR year3(4);
//
//        year3++;
//
//        year3 = year2 - year1;
//
//        double a = year3;
//        year3 = 3.4;
//
//        year3.min;
//
//        year1.size;
//
//        YEAR::min;
//
//        for (YEAR my_year = YEAR::min; my_year <= YEAR::max; my_year++) {
//            my_year;
//        }
//
//        for (YEAR my_year = my_year.min; my_year <= my_year.max; my_year++) {
//            my_year;
//            int j = my_year.index();
//        }
//
//        int sum = 0;
//        for (int k : YEAR::indices) {
//            sum += k;
//        }
//
//        int n = YEAR::size;
//
//        double Something[YEAR::size];
//        //double Something[] = new dou
//
//        double Something2[year1.size];
//
//        Something;
//        Something2;
//
//    }
//
//    real om_cutpoints_AGE_GROUP[] = { 0.5, 10, 20, 30 };
//    typedef Partition<unsigned char, 5, om_cutpoints_AGE_GROUP> AGE_GROUP;
//
//    void Tester2()
//    {
//        AGE_GROUP a1 = 1;
//        a1++;
//
//        AGE_GROUP::size;
//        a1.max;
//        a1.size;
//
//        int sum = 0;
//        for (int k : AGE_GROUP::indices) {
//            sum += k;
//        }
//    }
//
//}
