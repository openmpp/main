/**
 * @file    AgentVar.h
 * Declares the AgentVar class and associated classes and templates
 *         
 */
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

using namespace std;

namespace mm {

    template<typename T, typename A, void (A::*side_effects)(T old_value, T new_value) = nullptr>
    class AgentVar 
    {
    public:
        // ctor
        explicit AgentVar()
        {
        }

        // initialization
        void initialize( T initial_value )
        {
            value = initial_value;
        }

        // operator: cast to T (use agentvar in C++ expression)
        operator T()
        {
            return get();
        }

        // get pointer to containing agent
        A *agent()
        {
            return (A *) ( (char *)this - offset_in_agent );
        }

        void set( T new_value )
        {
            T old_value = get();
            if ( old_value != new_value ) {
                value = new_value;
                if ( side_effects != nullptr ) {
                    (agent()->*side_effects)(old_value, new_value);
                }
            }
        }

        T get() const
        {
            return value;
        }

        // storage
        T value;

	    // offset to containing agent
	    static size_t offset_in_agent;
    };

    /**
    * AgentVar offset in Agent (definition)
    */

    template<typename T, typename A, void (A::*side_effects)(T old_value, T new_value)>
    size_t AgentVar<T, A, side_effects>::offset_in_agent = 0;

    template<typename T, typename A, void (A::*side_effects)(T old_value, T new_value) = nullptr>
    class SimpleAgentVar : public AgentVar<T, A, side_effects>
    {
    public:

        // operator: direct assignment
        SimpleAgentVar& operator=( T new_value )
        {
            this->set( new_value );
            return *this;
        }

        // operator: assignment by sum
        SimpleAgentVar& operator+=( T modify_value )
        {
            T new_value = this->get() + modify_value;
            this->set( new_value );
            return *this;
        }

        // operator: assignment by difference
        SimpleAgentVar& operator-=( T modify_value )
        {
            T new_value = this->get() - modify_value;
            this->set( new_value );
            return *this;
        }

        // operator: assignment by product
        SimpleAgentVar& operator*=( T modify_value )
        {
            T new_value = this->get() * modify_value;
            this->set( new_value );
            return *this;
        }

        // operator: assignment by quotient
        SimpleAgentVar& operator/=( T modify_value )
        {
            T new_value = this->get() / modify_value;
            this->set( new_value );
            return *this;
        }

        // operator: assignment by remainder
        SimpleAgentVar& operator%=( T modify_value )
        {
            T new_value = this->get() % modify_value;
            this->set( new_value );
            return *this;
        }

        // operator: assignment by bitwise left shift
        SimpleAgentVar& operator<<=( T modify_value )
        {
            T new_value = this->get() << modify_value;
            this->set( new_value );
            return *this;
        }

        // operator: assignment by bitwise right shift
        SimpleAgentVar& operator>>=( T modify_value )
        {
            T new_value = this->get() >> modify_value;
            this->set( new_value );
            return *this;
        }

        // operator: assignment by bitwise AND
        SimpleAgentVar& operator&=( T modify_value )
        {
            T new_value = this->get() & modify_value;
            this->set( new_value );
            return *this;
        }

        // operator: assignment by bitwise XOR
        SimpleAgentVar& operator^=( T modify_value )
        {
            T new_value = this->get() ^ modify_value;
            this->set( new_value );
            return *this;
        }

        // operator: assignment by bitwise OR
        SimpleAgentVar& operator|=( T modify_value )
        {
            T new_value = this->get() | modify_value;
            this->set( new_value );
            return *this;
        }

        // operator: prefix increment
        SimpleAgentVar& operator++()
        {
            T new_value = this->get() + 1;
            this->set( new_value );
            return *this;
        }

        // operator: prefix decrement
        SimpleAgentVar& operator--()
        {
            T new_value = this->get() - 1;
            this->set( new_value );
            return *this;
        }

        // operator: postfix increment
        T operator++(int)
        {
            T new_value = 1 + this->get();
            this->set( new_value );
            return new_value;
        }

        // operator: postfix decrement
        T operator--(int)
        {
            T new_value = this->get() - 1;
            this->set( new_value );
            return new_value;
        }
    };

    // SimpleAgentVar bool specialization
    // remove invalid and deprecated assignment operators for bool, e.g. ++,+=, etc.
    template<typename A, void (A::*side_effects)(bool old_value, bool new_value)>
    class SimpleAgentVar<bool, A, side_effects> : public AgentVar<bool, A, side_effects>
    {
    public:

        // operator: assignment
        SimpleAgentVar& operator=( bool new_value )
        {
            this->set( new_value );
            return *this;
        }

    };

    template<typename T, typename A, void (A::*side_effects)(T old_value, T new_value), T (A::*expression)()>
    class ExpressionAgentVar : public AgentVar<T, A, side_effects>
    {
    public:
        // evaluate expression and update
        void evaluate()
        {
            T new_value = ((this->agent())->*expression)();
            this->set( new_value );
        }
    };


    template<typename T, typename A, void (A::*side_effects)(T old_value, T new_value) = nullptr , bool (A::*condition)() = nullptr>
    class DurationAgentVar : public AgentVar<T, A, side_effects>
    {
    public:
        // update duration 
        void wait( T delta_time )
        {
            bool active = ( condition == nullptr || ((this->agent())->*condition)() );        if ( active ) {
                this->set( this->get() + delta_time );
            }
        }

        // Reset duration to zero
        void reset()
        {
            this->set( 0 );
        }
    };

} // namespace mm

