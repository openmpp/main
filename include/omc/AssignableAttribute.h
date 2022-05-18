/**
 * @file    AssignableAttribute.h
 * Declares the AssignableAttribute template
 *         
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once

#include "Attribute.h"

/**
 * Template for assignable attributes.
 * 
 * All C++ assignment operators are implemented in AssignableAttribute, allowing developer code to
 * assign values.  Side-effects are executed if the assignment changes the value.
 *
 * @tparam T               Generic type parameter.
 * @tparam T2              Type of the type being wrapped (e.g. range has inner type int).
 * @tparam A               Type of containing entity.
 * @tparam NT_name         Name of the attribute (non-type argument).
 * @tparam NT_is_time_like Attribute can change between events, like time.
 * @tparam NT_side_effects Function implementing assignment side effects (non-type argument).
 * @tparam NT_se_present   Assignment side-effects are present (non-type parameter).
 * @tparam NT_notify       Function implementing pre-notification of change in value (non-type parameter).
 * @tparam NT_ntfy_present Assignment pre-notification actions are present (non-type parameter).
 */
template<
    typename T,
    typename T2,
    typename A,
    std::string const *NT_name,
    bool NT_is_time_like,
    void (A::*NT_side_effects)(T old_value, T new_value),
    bool NT_se_present,
    void (A::*NT_notify)(),
    bool NT_ntfy_present
>
class AssignableAttribute : public Attribute<T, T2, A, NT_name, NT_is_time_like, NT_side_effects, NT_se_present, NT_notify, NT_ntfy_present>
{
public:
    // Copy constructor is deleted to prohibit creation of local variable Attribute objects.
    AssignableAttribute(const AssignableAttribute&) = delete; // copy constructor
    //AssignableAttribute& operator=(const AssignableAttribute&) = delete; // copy assignment operator

    // operator: copy assignment
    // Handle occasional situation where an Attribute is assigned to an identical Attribute in another Entity.
    // An example is assigning newborn's time to mother's time at creation of newborn.
    AssignableAttribute& operator=(const AssignableAttribute & other)
    {
        // Assign using the set function to trigger side-effects.
        this->set( other.get() );
        return *this;
    }

    // default ctor
    AssignableAttribute()
    {
    }

    // converting ctor for creating temporary r-values
    AssignableAttribute(T assign_value)
        : Attribute<T, T2, A, NT_name, NT_is_time_like, NT_side_effects, NT_se_present, NT_notify, NT_ntfy_present>(assign_value)
    {
    }

    // operator: direct assignment of wrapped type
    AssignableAttribute& operator=( T new_value )
    {
        this->set( new_value );
        return *this;
    }

    // operator: assignment by sum
    AssignableAttribute& operator+=( T modify_value )
    {
        T new_value = this->get() + modify_value;
        this->set( new_value );
        return *this;
    }

    // operator: assignment by difference
    AssignableAttribute& operator-=( T modify_value )
    {
        T new_value = this->get() - modify_value;
        this->set( new_value );
        return *this;
    }

    // operator: assignment by product
    AssignableAttribute& operator*=( T modify_value )
    {
        T new_value = this->get() * modify_value;
        this->set( new_value );
        return *this;
    }

    // operator: assignment by quotient
    AssignableAttribute& operator/=( T modify_value )
    {
        T new_value = this->get() / modify_value;
        this->set( new_value );
        return *this;
    }

    // operator: assignment by remainder
    AssignableAttribute& operator%=( T modify_value )
    {
        T new_value = this->get() % modify_value;
        this->set( new_value );
        return *this;
    }

    // operator: assignment by bitwise left shift
    AssignableAttribute& operator<<=( T modify_value )
    {
        T new_value = this->get() << modify_value;
        this->set( new_value );
        return *this;
    }

    // operator: assignment by bitwise right shift
    AssignableAttribute& operator>>=( T modify_value )
    {
        T new_value = this->get() >> modify_value;
        this->set( new_value );
        return *this;
    }

    // operator: assignment by bitwise AND
    AssignableAttribute& operator&=( T modify_value )
    {
        T new_value = this->get() & modify_value;
        this->set( new_value );
        return *this;
    }

    // operator: assignment by bitwise XOR
    AssignableAttribute& operator^=( T modify_value )
    {
        T new_value = this->get() ^ modify_value;
        this->set( new_value );
        return *this;
    }

    // operator: assignment by bitwise OR
    AssignableAttribute& operator|=( T modify_value )
    {
        T new_value = this->get() | modify_value;
        this->set( new_value );
        return *this;
    }

    // operator: prefix increment
    AssignableAttribute& operator++()
    {
        T new_value = this->get() + 1;
        this->set( new_value );
        return *this;
    }

    // operator: prefix decrement
    AssignableAttribute& operator--()
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

// AssignableAttribute participation in type resolution based on wrapped types
// by specializing std::common_type.
// e.g. in min/max/clamp mixed-mode templates

namespace std {

    // unwrap AssignableAttribute with void T2
    template<typename Other, typename T, typename A, string const *NT_name, bool NT_is_time_like, void (A::*NT_side_effects)(T, T), bool NT_se_present, void (A::*NT_notify)(), bool NT_ntfy_present>
    struct common_type<Other, AssignableAttribute<T, void, A, NT_name, NT_is_time_like, NT_side_effects, NT_se_present, NT_notify, NT_ntfy_present>>
    {
        using type = typename common_type<Other, T>::type;
    };

    // unwrap AssignableAttribute with void T2, opposite order
    template<typename Other, typename T, typename A, string const *NT_name, bool NT_is_time_like, void (A::*NT_side_effects)(T, T), bool NT_se_present, void (A::*NT_notify)(), bool NT_ntfy_present>
    struct common_type<AssignableAttribute<T, void, A, NT_name, NT_is_time_like, NT_side_effects, NT_se_present, NT_notify, NT_ntfy_present>, Other>
    {
        using type = typename common_type<Other, T>::type;
    };

    // unwrap AssignableAttribute with non-void T2
    template<typename Other, typename T, typename T2, typename A, string const *NT_name, bool NT_is_time_like, void (A::*NT_side_effects)(T, T), bool NT_se_present, void (A::*NT_notify)(), bool NT_ntfy_present>
    struct common_type<Other, AssignableAttribute<T, T2, A, NT_name, NT_is_time_like, NT_side_effects, NT_se_present, NT_notify, NT_ntfy_present>>
    {
        using type = typename common_type<Other, T2>::type;
    };

    // unwrap AssignableAttribute with non-void T2, opposite order
    template<typename Other, typename T, typename T2, typename A, string const *NT_name, bool NT_is_time_like, void (A::*NT_side_effects)(T, T), bool NT_se_present, void (A::*NT_notify)(), bool NT_ntfy_present>
    struct common_type<AssignableAttribute<T, T2, A, NT_name, NT_is_time_like, NT_side_effects, NT_se_present, NT_notify, NT_ntfy_present>, Other>
    {
        using type = typename common_type<Other, T2>::type;
    };

}