/**
* @file    test_link.cpp
* Used for development testing of the run-time support classes
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include "om_types0.h"
#include "omc/link.h"

class BaseEvent {
public:
    static bool just_in_time;
};
bool BaseEvent::just_in_time = true;


class BaseAgent {
public:
    static Time global_time;
};
Time BaseAgent::global_time = 2013.5;

class Person {
public:
    void age_agent(Time t) {
        time = t;
    }

    // local time of the Person
    // Following is like an agentvar
    struct time_av {
        Time value;
    public:
        operator Time() const {
            return value;
        }

        time_av& operator=(Time new_value) {
            value = new_value;
            return *this;
        }

        bool operator==(Time rhs_value) {
            return value == rhs_value;
        }

        void set(Time t) {
            value = t;
        }
    } time;

    // the 'simulated' call-back function which
    // implements the expression agentvar family_income
    void om_expression_family_income() {
        family_income = income + lSpouse->income;
    }

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
};

Person Person::om_null_agent;

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

    Person person3;
    person3.time = 2013.0;
    person3.income = 62000;
    person3.family_income = 0;

    Person person4;
    person4.time = 2013.0;
    person4.income = 62000;
    person4.family_income = 0;

    // assignment of pointer to link
    person1.lSpouse = &person2;
    person2.lSpouse = &person1;

    // assignment of link to link
    person3.lSpouse = person2.lSpouse;

    // link as a local variable
    link<Person> p3;
    p3 = person1.lSpouse;
    person4.lSpouse = p3;

    // test automatic update of local time of person2
    // before referencing agentvars of person2 (just-in-time algorithm)
    person1.age_agent(BaseAgent::global_time);
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
