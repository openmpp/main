/**
 * @file   custom.h
 * Custom include file for the model
 *  
 * This file is included in all compiler-generated files for the model.
 * Place any required globals declarations in this file, or include other
 * files in this file, for correct placement in compiler-generated files.
 */

 // The following are temporary, pending integration in ompp

#if defined(OPENM)
void Set_actor_weight(double weight);
void Set_actor_subsample_weight(double weight);
#endif
