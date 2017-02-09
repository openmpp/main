/**
 * @file   custom.h
 * Model-specific declarations and includes (late)
 *  
 * This file is included in all compiler-generated files for the model.
 * This file comes late in the include order, after entity declarations.
 */

#pragma once
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
template<typename T, typename U>
struct std::common_type<fixed_precision_float<T>, U>
{
       using type = typename std::common_type<T, U>::type;
};

// unwrap fixed_precision_float, opposite order, calls other specialization
template<typename T, typename U>
struct std::common_type<T, fixed_precision_float<U>>
{
       using type = typename std::common_type<fixed_precision_float<U>, T>::type;
};

// unwrap AgentVar, with non-void T2
template<typename Other, typename T, typename T2, typename A, string const *NT_name, void (A::*NT_side_effects)(T, T), bool NT_se_present, void (A::*NT_notify)(), bool NT_ntfy_present>
struct std::common_type<Other, AgentVar<T, T2, A, NT_name, NT_side_effects, NT_se_present, NT_notify, NT_ntfy_present>>
{
       using type = typename std::common_type<Other, T2>::type;
};

// unwrap AgentVar, with void T2
template<typename Other, typename T, typename A, string const *NT_name, void (A::*NT_side_effects)(T, T), bool NT_se_present, void (A::*NT_notify)(), bool NT_ntfy_present>
struct std::common_type<Other, AgentVar<T, void, A, NT_name, NT_side_effects, NT_se_present, NT_notify, NT_ntfy_present>>
{
       using type = typename std::common_type<Other, T>::type;
};

// unwrap AgentVar, opposite order calls other specializations
template<typename Other, typename T, typename T2, typename A, string const *NT_name, void (A::*NT_side_effects)(T, T), bool NT_se_present, void (A::*NT_notify)(), bool NT_ntfy_present>
struct std::common_type<AgentVar<T, T2, A, NT_name, NT_side_effects, NT_se_present, NT_notify, NT_ntfy_present>, Other>
{
       using type = typename std::common_type<Other, AgentVar<T, T2, A, NT_name, NT_side_effects, NT_se_present, NT_notify, NT_ntfy_present>>::type;
};

// unwrap AssignableAgentVar, calls AgentVar specializations
template<typename Other, typename T, typename T2, typename A, string const *NT_name, void (A::*NT_side_effects)(T, T), bool NT_se_present, void (A::*NT_notify)(), bool NT_ntfy_present>
struct std::common_type<Other, AssignableAgentVar<T, T2, A, NT_name, NT_side_effects, NT_se_present, NT_notify, NT_ntfy_present>>
{
       using type = typename std::common_type<Other, AgentVar<T, T2, A, NT_name, NT_side_effects, NT_se_present, NT_notify, NT_ntfy_present>>::type;
};

// unwrap AssignableAgentVar, opposite order, calls AssignableAgentVar specialization
template<typename Other, typename T, typename T2, typename A, string const *NT_name, void (A::*NT_side_effects)(T, T), bool NT_se_present, void (A::*NT_notify)(), bool NT_ntfy_present>
struct std::common_type<AssignableAgentVar<T, T2, A, NT_name, NT_side_effects, NT_se_present, NT_notify, NT_ntfy_present>, Other>
{
       using type = typename std::common_type<Other, AssignableAgentVar<T, T2, A, NT_name, NT_side_effects, NT_se_present, NT_notify, NT_ntfy_present>>::type;
};
#endif //defined(OPENM)

