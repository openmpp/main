/**
* @file    test_templates.cpp
* Used for development testing of the run-time support classes
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

extern void test_partition();
extern void test_range();
extern void test_classification();
extern void test_link();


int main(int argc, char * argv[])
{
    test_partition();
    test_range();
    test_classification();
    test_link();

	return 0;
}

