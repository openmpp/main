/**
* @file    test_link.cpp
* Used for development testing of the run-time support classes
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include "om_types0.h"
#include "omc/link.h"

class Person {
public:
    void age_agent(Time t) {
        time = t;
    }

    // the 'simulated' call-back function which
    // implements the expression agentvar family_income
    void om_expression_family_income() {
        family_income = income + lSpouse->income;
    }

    // local time of the Person
    Time time;

    // a 'simulated' simple agent var
    real income;

    // a 'simulated' expression agent_var, updated by
    // the expression function 'om_expression_family_income'
    // The expression declaration would be:
    // real family_income = income + lSpouse->family_income;
    real family_income;

    // single link under test
    link<Person> lSpouse;

    static Person om_null_agent;

    // global time of the simulation
    static Time global_time;
};

Person Person::om_null_agent;

Time Person::global_time = 2013.5;

void test_link()
{
    // One-time initialization of the 'global' zero person.
    // This agent is used by the 'smart pointer' link 
    // to return values from empty links (nullptr).
    Person::om_null_agent.income = 0;

    Person person1;
    person1.time = 2013.0;
    person1.income = 50000;
    person1.family_income = 0;

    Person person2;
    person2.time = 2013.0;
    person2.income = 62000;
    person2.family_income = 0;

    person1.lSpouse = &person2;
    person2.lSpouse = &person1;

    // test automatic update of local time of person2
    // before referencing agentvars of person2 (just-in-time algorithm)
    person1.age_agent(Person::global_time);
    person1.om_expression_family_income();
    assert(person2.time == 2013.5);
    assert(person1.family_income == 112000);

    // use of links as local variables
    link<Person> pReferencePerson(&person1);
    link<Person> pSpouseOfReferencePerson(pReferencePerson->lSpouse);
    real sum_income = pReferencePerson->income + pSpouseOfReferencePerson->income;
    assert(sum_income == 112000);

    // test of nullptr links (all members are zero)
    person1.lSpouse = nullptr;
    person2.lSpouse = nullptr;
    person1.om_expression_family_income();
    assert(person1.family_income == 50000);
}
