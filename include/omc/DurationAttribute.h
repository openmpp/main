/**
 * @file    DurationAttribute.h
 * Declares the DurationAttribute template
 *         
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once

#include "Attribute.h"

using namespace std;

/**
 * Template for duration attributes.
 * 
 * No assignment operators are implemented, making the agentvar read-only in C++ code.  A
 * constant template argument holds the function used to evaluate the logical condition during
 * which duration will be cumulated.
 *
 * @tparam T               Type of the attribute.
 * @tparam T2              Type of the type being wrapped (e.g. range has inner type int).
 * @tparam A               Type of the containing entity.
 * @tparam NT_name         Name of the attribute (non-type argument).
 * @tparam NT_side_effects Function implementing assignment side effects (non-type parameter).
 * @tparam NT_se_present   Assignment side-effects are present (non-type parameter).
 * @tparam NT_notify       Function implementing pre-notification of change in value (non-type parameter).
 * @tparam NT_ntfy_present Assignment pre-notification actions are present (non-type parameter).
 * @tparam NT_condition    Function implementing the condition (non-type parameter).
 */
template<
    typename T,
    typename T2,
    typename A,
    string const *NT_name,
    void (A::*NT_side_effects)(T old_value, T new_value),
    bool NT_se_present,
    void (A::*NT_notify)(),
    bool NT_ntfy_present,
    bool (A::*NT_condition)() = nullptr
>
class DurationAgentVar : public AgentVar<T, T2, A, NT_name, NT_side_effects, NT_se_present, NT_notify, NT_ntfy_present>
{
public:
    // Copy constructor is deleted to prohibit creation of local variable Attribute objects.
    DurationAgentVar(const DurationAgentVar&) = delete; // copy constructor
    DurationAgentVar& operator=(const DurationAgentVar&) = delete; // copy initialization operator

    // update duration 
    void advance( T delta_time )
    {
        bool active = ( NT_condition == nullptr || ((this->agent())->*NT_condition)() );
        if ( active ) {
            this->set( this->get() + delta_time );
        }
    }

    // Reset duration to zero
    void reset()
    {
        this->set( 0 );
    }
};

// DurationAttribute participation in type resolution based on wrapped types
// by specializing std::common_type.
// e.g. in min/max/clamp mixed-mode templates

namespace std {

    // unwrap DurationAttribute with void T2
    template<typename Other, typename T, typename A, string const *NT_name, void (A::*NT_side_effects)(T, T), bool NT_se_present, void (A::*NT_notify)(), bool NT_ntfy_present, bool (A::*NT_condition)()>
    struct common_type<Other, DurationAgentVar<T, void, A, NT_name, NT_side_effects, NT_se_present, NT_notify, NT_ntfy_present, NT_condition>>
    {
        using type = typename common_type<Other, T>::type;
    };

    // unwrap DurationAttribute with void T2, opposite order
    template<typename Other, typename T, typename A, string const *NT_name, void (A::*NT_side_effects)(T, T), bool NT_se_present, void (A::*NT_notify)(), bool NT_ntfy_present, bool (A::*NT_condition)()>
    struct common_type<DurationAgentVar<T, void, A, NT_name, NT_side_effects, NT_se_present, NT_notify, NT_ntfy_present, NT_condition>, Other>
    {
        using type = typename common_type<Other, T>::type;
    };

    // unwrap DurationAttribute with non-void T2
    template<typename Other, typename T, typename T2, typename A, string const *NT_name, void (A::*NT_side_effects)(T, T), bool NT_se_present, void (A::*NT_notify)(), bool NT_ntfy_present, bool (A::*NT_condition)()>
    struct common_type<Other, DurationAgentVar<T, T2, A, NT_name, NT_side_effects, NT_se_present, NT_notify, NT_ntfy_present, NT_condition>>
    {
        using type = typename common_type<Other, T2>::type;
    };

    // unwrap DurationAttribute with non-void T2, opposite order
    template<typename Other, typename T, typename T2, typename A, string const *NT_name, void (A::*NT_side_effects)(T, T), bool NT_se_present, void (A::*NT_notify)(), bool NT_ntfy_present, bool (A::*NT_condition)()>
    struct common_type<DurationAgentVar<T, T2, A, NT_name, NT_side_effects, NT_se_present, NT_notify, NT_ntfy_present, NT_condition>, Other>
    {
        using type = typename common_type<Other, T2>::type;
    };

}