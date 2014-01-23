/**
* @file    test_templates.cpp
* Used for development testing of the run-time support classes
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

namespace mm
{
    extern void test_partition();
}


int main(int argc, char * argv[])
{
    mm::test_partition();

	return 0;
}

