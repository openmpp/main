/**
* @file    framework.h
* Globals in fmk namespace available to all models
*/
// Copyright (c) 2013-2023 OpenM++ Contributors
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once

namespace fmk {

    /**
     * The run-time API
     */
    thread_local inline openm::IModel* i_model = nullptr;

    /**
     * The number of simulations in the run
     */
    int inline simulation_members = 0;

    /**
     * The current simulation member.
     */
    thread_local inline int simulation_member = 0;

    /**
     * The entity counter in this simulation member.
     */
    thread_local inline long member_entity_counter = 0;

    /**
     * The modulus used by linear congruential random number generators
     * (possibly to seed a different RNG).
     * Equal to 2^31 - 1
     */
    const long inline lcg_modulus = 2147483647;

    /**
     * Multiplier of linear congruential generator used to initialize model stream seeds.
     */
    const long inline model_stream_seed_generator = 376740290;

    /**
     * The master seed encoded, with high order bits containing the simulation member id.
     *
     * The random number generators can vary by simulation member, so both
     * master seed and simulation member are required to reproduce
     * a case (for case-based models) or a replicate (for time-based models)
     * independtly of other cases / replicates.
     */
    thread_local inline long long combined_seed = 0;

    /**
     * The member portion of SimulationSeed, encoded in high-order bits
     *
     * Conceptually, this should be identical for all members,
     * but SimulationSeed is a parameter, so is provided by the framework
     * independently for each member.
     * So we declare it thread_local like a parameter.
     */
    thread_local inline long long SimulationSeed_member_part = 0;

    /**
     * The seed portion of SimulationSeed, encoded in low-order bits
     *
     * Conceptually, this should be identical for all members,
     * but SimulationSeed is a parameter, so is provided by the framework
     * independently for each member.
     * So we declare it thread_local like a parameter.
     */
    thread_local inline long long SimulationSeed_seed_part = 1;

    /**
     * The master seed, derived from the parameter SimulationSeed.
     *
     * The master seed is used to seed the individual random streams
     * in the model before simulating a case (for case-based models)
     * or simulating a member (aka replicate) (for time-based models).
     * It is also used to seed the individual random number streams
     * before execution of PreSimulation functions.
     *
     * For case-based models, the master seed is used for a case,
     * and is set by a separate case seed random number generator.
     *
     * For time-based models, the master seed is used for a member (aka replicate),
     * and is set to SimulationSeed.
     *
     * For use in PreSimulation functions,
     * the master seed is set to SimulationSeed.
     *
     * The actual RNG stream varies by member, in an RNG-specific way.
     */
    thread_local inline int master_seed = 0;

    /**
     * The value for population scaling.
     *
     * Specified by model code through a call to SetPopulation() in a PreSimulation function.
     */
    long inline set_population_value = 0;

    /**
     * A signal flag to the framework to exit all entities from the simulation.
     *
     * Set by model code through a call to signal_exit_simulation_all().
     * NB: DEPRECATED - not needed or useful in ompp design
     */
    thread_local inline bool do_exit_simulation_all = false;

    /**
     * Default value of % progress messages during simulation
     */
    const inline int progress_percent_default = 1;
}
