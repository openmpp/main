/**
 * @file   custom.h
 * Model-specific declarations and includes (late)
 *  
 * This file is included in all compiler-generated files for the model.
 * This file comes late in the include order, after entity declarations.
 */

#pragma once
#include <algorithm> // for std:min and std::max
#include "case_based.h"

#if defined(MODGEN)
namespace mm {
    //typedef EMOTION EMOTION_t;
    //typedef TIME TIME_t;
}
#else
//typedef om_enum_EMOTION EMOTION_t;
//typedef double TIME_t;
#endif

#if defined(OPENM)

#pragma message ("really got here, inside OPENM")

// unwrap fixed_precision_float
//template<typename T, typename U>
//struct std::common_type<fixed_precision_float<T>, U>
//{
//       using type = typename std::common_type<T, U>::type;
//};

namespace std {

    // unwrap fixed_precision_float
    template<typename T, typename U>
    struct common_type<fixed_precision_float<T>, U>
    {
        using type = typename common_type<T, U>::type;
    };

    // unwrap fixed_precision_float, opposite order
    template<typename T, typename U>
    struct common_type<T, fixed_precision_float<U>>
    {
        using type = typename common_type<T, U>::type;
    };

    // unwrap AgentVar, with non-void T2
    template<typename Other, typename T, typename T2, typename A, string const *NT_name, void (A::*NT_side_effects)(T, T), bool NT_se_present, void (A::*NT_notify)(), bool NT_ntfy_present>
    struct common_type<Other, AgentVar<T, T2, A, NT_name, NT_side_effects, NT_se_present, NT_notify, NT_ntfy_present>>
    {
        using type = typename common_type<Other, T2>::type;
    };

    // unwrap AgentVar, with void T2
    template<typename Other, typename T, typename A, string const *NT_name, void (A::*NT_side_effects)(T, T), bool NT_se_present, void (A::*NT_notify)(), bool NT_ntfy_present>
    struct common_type<Other, AgentVar<T, void, A, NT_name, NT_side_effects, NT_se_present, NT_notify, NT_ntfy_present>>
    {
        using type = typename common_type<Other, T>::type;
    };
    // unwrap AgentVar, opposite order calls other specializations
    template<typename Other, typename T, typename T2, typename A, string const *NT_name, void (A::*NT_side_effects)(T, T), bool NT_se_present, void (A::*NT_notify)(), bool NT_ntfy_present>
    struct common_type<AgentVar<T, T2, A, NT_name, NT_side_effects, NT_se_present, NT_notify, NT_ntfy_present>, Other>
    {
        using type = typename common_type<Other, AgentVar<T, T2, A, NT_name, NT_side_effects, NT_se_present, NT_notify, NT_ntfy_present>>::type;
    };

    // unwrap AssignableAgentVar, calls AgentVar specializations
    template<typename Other, typename T, typename T2, typename A, string const *NT_name, void (A::*NT_side_effects)(T, T), bool NT_se_present, void (A::*NT_notify)(), bool NT_ntfy_present>
    struct common_type<Other, AssignableAgentVar<T, T2, A, NT_name, NT_side_effects, NT_se_present, NT_notify, NT_ntfy_present>>
    {
        using type = typename common_type<Other, AgentVar<T, T2, A, NT_name, NT_side_effects, NT_se_present, NT_notify, NT_ntfy_present>>::type;
    };

    // unwrap AssignableAgentVar, opposite order, calls AssignableAgentVar specialization
    template<typename Other, typename T, typename T2, typename A, string const *NT_name, void (A::*NT_side_effects)(T, T), bool NT_se_present, void (A::*NT_notify)(), bool NT_ntfy_present>
    struct common_type<AssignableAgentVar<T, T2, A, NT_name, NT_side_effects, NT_se_present, NT_notify, NT_ntfy_present>, Other>
    {
        using type = typename common_type<Other, AssignableAgentVar<T, T2, A, NT_name, NT_side_effects, NT_se_present, NT_notify, NT_ntfy_present>>::type;
    };

}

// Replacement versions of std:min and std::max which enable type conversion of arguments
// to a common shared type.
//template <typename T, typename U, typename CT = typename std::common_type<T, U>::type>
//inline CT min(T a, U b)
//{
//    return (CT)a < (CT)b ? (CT)a : (CT)b;
//}
//
//template <typename T, typename U, typename CT = typename std::common_type<T, U>::type>
//inline CT max(T a, U b)
//{
//    return (CT)a < (CT)b ? (CT)b : (CT)a;
//}
//
//template <typename T, typename U, typename V, typename CT = typename std::common_type<T, U, V>::type>
//inline CT clamp(T inf, U sup, V x)
//{
//    if ((CT)inf > (CT)x) return (CT)inf;
//    else if ((CT)sup < (CT)x) return (CT)sup;
//    else return (CT)x;
//}

#endif //defined(OPENM)

