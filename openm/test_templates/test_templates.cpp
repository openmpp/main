/**
* @file    test_templates.cpp
* Used for development testing of the run-time support classes
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

namespace mm
{
    extern void test_partition();
    extern void test_range();
    extern void test_classification();
}


int main(int argc, char * argv[])
{
    mm::test_partition();
    mm::test_range();
    mm::test_classification();

	return 0;
}

