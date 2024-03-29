/**
 * @file    Attribute.h
 * Declares the Attribute template
 *         
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once

/**
 * Base template for attributes.
 * 
 * The return value casting operator is implemented in this template, so C++ code can use the
 * attribute in expressions. No assignment operators are implemented, making entities declared
 * using Attribute read-only. Side-effects are evaluated if a call to set() changes the value.
 * 
 * A bug in VC++ prevented the use of nullptr for NT_side_effects.  The parameter NT_se_present
 * is a work-around for this bug.  Ditto for NT_notify and NT_ntfy_present.
 *
 * @tparam T               Type of the attribute.
 * @tparam T2              Type of the type being wrapped (e.g. range has inner type int).
 * @tparam A               Type of containing entity.
 * @tparam NT_name         Name of the attribute (non-type parameter).
 * @tparam NT_is_time_like Attribute can change between events, like time.
 * @tparam NT_side_effects Function implementing assignment side effects (non-type parameter).
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
class Attribute 
{
public:
    // Copy constructor is deleted to prohibit creation of local variable Attribute objects.
    Attribute(const Attribute&) = delete; // copy constructor
    Attribute& operator=(const Attribute&) = delete; // copy assignment operator

    // default ctor
    Attribute()
    {
    }

    // converting ctor using T
    // 
    // For creating temporary r-values in expressions.
    Attribute(T assign_value)
    {
        // N.B. no side-effects
        value = assign_value;
    }

#if false // under test
    // converting ctor using T2
    // 
    // For creating temporary r-values in expressions.
    // Disabled if T2 is void.
    template <typename U = T2>
#if defined(CXX_17)
    Attribute(typename std::enable_if_t< !std::is_void_v<U>, U > assign_value)
#else
    Attribute(typename std::enable_if< !std::is_void<U>::value, U >::type assign_value)
#endif
    {
        // N.B. no side-effects
        value = assign_value;
    }
#endif // under test

    // initialization
    void initialize( T initial_value )
    {
        value = initial_value;
    }

    // operator: user-defined conversion to T
    // 
//    // Use SFINAE to disable if type is wrapped (T2 is void)
//    template <typename U = T2>
//#if defined(CXX_17)
//    operator typename std::enable_if_t< std::is_void_v<U>, T >() const
//#else
//    operator typename std::enable_if< std::is_void<U>::value, T >::type() const
//#endif
    operator T() const
    {
        return get();
    }

    // operator: user-defined conversion to T2 (when Attribute type is itself a wrapped type)
    // 
    // Disabled if type is not wrapped (T2 is void).
    // Not really necessary to disable, since C++ user-defined conversion to void is legal and ignored.
//    template <
//        typename U = T2,
//#if defined(CXX_17)
//        typename = typename std::enable_if_t< !std::is_void_v<U> >
//#else
//        typename = typename std::enable_if< !std::is_void<U>::value>::type
//#endif
//    >
    operator T2() const
    {
        return get();
    }

    // member template to pass conversion inwards to the contained type to do the cast
    //template<typename T1>
    //operator T1()
    //{
    //    return (T1)get();
    //}

    // get pointer to containing entity
    A *entity()
    {
        return (A *) ( (char *)this - offset_in_entity );
    }

    void set( T new_value )
    {
        if (om_verify_attribute_modification) { // is constexpr
            if (!BaseEntity::om_permit_attribute_modification) {
                // attribute assignment is forbidden in current phase of event lifecycle
                handle_prohibited_attribute_assignment(get_name());
            }
        }
        T old_value = get();
        if ( old_value != new_value ) {

            // Before the value of the attribute changes
            if (NT_ntfy_present) {
                (entity()->*NT_notify)();
            }

            // Change the attribute to the new value
            value = new_value;

            // After the attribute value has changed
            if (NT_se_present) {
                (entity()->*NT_side_effects)(old_value, new_value);
            }

        }

    }

    T get() const
    {
        if (om_verify_timelike_attribute_access) { // is constexpr
            if (!BaseEntity::om_permit_timelike_attribute_access && get_is_time_like()) {
                // time-like attribute access is forbidden in current phase of event lifecycle
                handle_prohibited_timelike_attribute_access(get_name());
            }
        }
        return value;
    }

    T direct_get() const
    {
        // bypasses checks, for diagnostic output only, eg in cover event time trace function
        return value;
    }

    /**
     * Gets the name of the attribute
     *
     * @return The name.
     */
    static const std::string & get_name()
    {
        return *NT_name;
    }

    /**
     * Gets the is_time_like property of the attribute
     *
     * @return bool
     */
    static const bool get_is_time_like()
    {
        return NT_is_time_like;
    }

    // storage
    T value;

	// offset to containing entity
	static size_t offset_in_entity;
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
    std::string const *NT_name,
    bool NT_is_time_like,
    void (A::*NT_side_effects)(T old_value, T new_value),
    bool NT_se_present,
    void (A::*NT_notify)(),
    bool NT_ntfy_present
>
size_t Attribute<T, T2, A, NT_name, NT_is_time_like, NT_side_effects, NT_se_present, NT_notify, NT_ntfy_present>::offset_in_entity = 0;

// Attribute participation in type resolution based on wrapped types
// by specializing std::common_type.
// e.g. in min/max/clamp mixed-mode templates

namespace std {

    // unwrap Attribute with void T2
    template<typename Other, typename T, typename A, string const *NT_name, bool NT_is_time_like, void (A::*NT_side_effects)(T, T), bool NT_se_present, void (A::*NT_notify)(), bool NT_ntfy_present>
    struct common_type<Other, Attribute<T, void, A, NT_name, NT_is_time_like, NT_side_effects, NT_se_present, NT_notify, NT_ntfy_present>>
    {
        using type = typename common_type<Other, T>::type;
    };

    // unwrap Attribute with void T2, opposite order
    template<typename Other, typename T, typename A, string const *NT_name, bool NT_is_time_like, void (A::*NT_side_effects)(T, T), bool NT_se_present, void (A::*NT_notify)(), bool NT_ntfy_present>
    struct common_type<Attribute<T, void, A, NT_name, NT_is_time_like, NT_side_effects, NT_se_present, NT_notify, NT_ntfy_present>, Other>
    {
        using type = typename common_type<Other, T>::type;
    };

    // unwrap Attribute with non-void T2
    template<typename Other, typename T, typename T2, typename A, string const *NT_name, bool NT_is_time_like, void (A::*NT_side_effects)(T, T), bool NT_se_present, void (A::*NT_notify)(), bool NT_ntfy_present>
    struct common_type<Other, Attribute<T, T2, A, NT_name, NT_is_time_like, NT_side_effects, NT_se_present, NT_notify, NT_ntfy_present>>
    {
        using type = typename common_type<Other, T2>::type;
    };

    // unwrap Attribute with non-void T2, opposite order
    template<typename Other, typename T, typename T2, typename A, string const *NT_name, bool NT_is_time_like, void (A::*NT_side_effects)(T, T), bool NT_se_present, void (A::*NT_notify)(), bool NT_ntfy_present>
    struct common_type<Attribute<T, T2, A, NT_name, NT_is_time_like, NT_side_effects, NT_se_present, NT_notify, NT_ntfy_present>, Other>
    {
        using type = typename common_type<Other, T2>::type;
        //using type = decltype(true ? declval<Other>() : declval<T2>());
    };

    // unwrap two Attributes, both with non-void T2
    //template<typename T, typename T2, typename A, string const *NT_name, void (A::*NT_side_effects)(T, T), bool NT_se_present, void (A::*NT_notify)(), bool NT_ntfy_present,
    //         typename O_T, typename O_T2, typename O_A, string const *O_NT_name, void (A::*O_NT_side_effects)(T, T), bool O_NT_se_present, void (A::*O_NT_notify)(), bool O_NT_ntfy_present>
    //struct common_type<Attribute<T, T2, A, NT_name, NT_side_effects, NT_se_present, NT_notify, NT_ntfy_present>,
    //                   Attribute<O_T, O_T2, O_A, O_NT_name, O_NT_side_effects, O_NT_se_present, O_NT_notify, O_NT_ntfy_present>>
    //{
    //    using type = typename common_type<T2, O_T2>::type;
    //};

}