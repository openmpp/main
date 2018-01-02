/**
 * @file    Attribute.h
 * Declares the Attribute template
 *         
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once

using namespace std;

/**
 * Base template for attributes.
 * 
 * The return value casting operator is implemented in this template, so C++ code can use the
 * attribute in expressions. No assignment operators are implemented, making entities declared
 * using AgentVar read-only. Side-effects are evaluated if a call to set() changes the value.
 * 
 * A bug in VC++ prevented the use of nullptr for NT_side_effects.  The parameter NT_se_present
 * is a work-around for this bug.  Ditto for NT_notify and NT_ntfy_present.
 *
 * @tparam T               Type of the attribute.
 * @tparam T2              Type of the type being wrapped (e.g. range has inner type int).
 * @tparam A               Type of containing entity.
 * @tparam NT_name         Name of the attribute (non-type parameter).
 * @tparam NT_side_effects Function implementing assignment side effects (non-type parameter).
 * @tparam NT_se_present   Assignment side-effects are present (non-type parameter).
 * @tparam NT_notify       Function implementing pre-notification of change in value (non-type parameter).
 * @tparam NT_ntfy_present Assignment pre-notification actions are present (non-type parameter).
 */
template<
    typename T,
    typename T2,
    typename A,
    string const *NT_name,
    void (A::*NT_side_effects)(T old_value, T new_value),
    bool NT_se_present,
    void (A::*NT_notify)(),
    bool NT_ntfy_present
>
class AgentVar 
{
public:
    //AgentVar(const AgentVar&) = delete;
    //AgentVar& operator=(const AgentVar&) = delete;

    // ctor
    AgentVar()
    {
    }

    // 1-argument ctor for creating r-values for assignment to 'real' AgentVar's in Agent
    AgentVar(T assign_value)
    {
        // N.B. no side-effects
        value = assign_value;
    }

    // 1-argument ctor for creating r-values for assignment to 'real' AgentVar's in Agent, for wrapped types
    //AgentVar(T2 assign_value)
    //{
    //    // N.B. no side-effects
    //    value = assign_value;
    //}

    // initialization
    void initialize( T initial_value )
    {
        value = initial_value;
    }

    // operator: cast to T (use agentvar containing fundamental type)
    operator T()
    {
        return (T) get();
    }

    operator T2()
    {
        return (T2) get();
    }

    // member template to pass conversion inwards to the contained type to do the cast
    //template<typename T1>
    //operator T1()
    //{
    //    return (T1)get();
    //}

    // get pointer to containing agent
    A *agent()
    {
        return (A *) ( (char *)this - offset_in_agent );
    }

    void set( T new_value )
    {
        T old_value = get();
        if ( old_value != new_value ) {

            // Before the value of the attribute changes
            if (NT_ntfy_present) {
                (agent()->*NT_notify)();
            }

            // Change the attribute to the new value
            value = new_value;

            // After the attribute value has changed
            if (NT_se_present) {
                (agent()->*NT_side_effects)(old_value, new_value);
            }

        }

    }

    T get() const
    {
        return value;
    }

    /**
     * Gets the name of the attribute
     *
     * @return The name.
     */
    static const string & get_name()
    {
        return NT_name;
    }

    // storage
    T value;

	// offset to containing agent
	static size_t offset_in_agent;
};

/**
 * Attribute offset in entity (static definition)
 * 
 * The offset is used within an instance of attribute to gain access to the enclosing entity to
 * call the side-effects function in the context of the entity and with access to all attributes
 * in the entity.  Attributes do not contain a pointer to the containing entity because the memory
 * cost would be prohibitive for simulations with many entities containing many attributes.
 */
template<
    typename T,
    typename T2,
    typename A,
    string const *NT_name,
    void (A::*NT_side_effects)(T old_value, T new_value),
    bool NT_se_present,
    void (A::*NT_notify)(),
    bool NT_ntfy_present
>
size_t AgentVar<T, T2, A, NT_name, NT_side_effects, NT_se_present, NT_notify, NT_ntfy_present>::offset_in_agent = 0;
