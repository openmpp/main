/**
 * @file    simulation.cpp
 * Residual framework code awaiting incorporation in omc / simulation run-time
 *         
 */
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "omc/omSimulation.h"

namespace openm {

    const int max_samples = 41;

    const long mg_modulus = 2147483647;

    const long case_seed_generators[max_samples] = {
		470583131,
		1278375574,
		1182424016,
		465267208,
		236156608,
		507096703,
		1030737213,
		1192442634,
		286354484,
		1963413634,
		929285805,
		1074439303,
		1866718706,
		1746251423,
		444178200,
		1076542630,
		289753891,
		490363125,
		803959450,
		37939113,
		1153920361,
		1010788020,
		1148043095,
		1422167303,
		1596996927,
		396692538,
		2125924067,
		290525234,
		1412033687,
		70608958,
		366654164,
		29727326,
		40186327,
		1271122795,
		940165244,
		735279377,
		1988769561,
		988683283,
		1943943356,
		1294875557,
		914624015,
    };

	const long model_stream_seed_generator = 376740290;

    const long model_stream_generators[max_samples] = {
		16807,
		1826645050,
		519701294,
		1912518406,
		87921397,
		755482893,
		673205363,
		727452832,
		630360016,
		1142281875,
		219667202,
		200558872,
		1185331463,
		573186566,
		396907481,
		1106264918,
		1605529283,
		1902548864,
		1444095898,
		1600915560,
		1987505485,
		1323051066,
		1715488211,
		1289290241,
		967740346,
		1644645313,
		2142074246,
		1397488348,
		97473033,
		1210640156,
		990191797,
		640039787,
		1141672104,
		2081478048,
		1236995837,
		1985494258,
		84845685,
		184528125,
		1303680654,
		61496220,
		1096609123,
	};

    const int model_streams = 10; // prototype only uses 1
    long model_stream_seeds[model_streams]; // seeds for stream generators

    long case_seed = 0; // the 'master seed' for the case
    long case_seed_generator = 0; // multiplier for the generator for the cases in the sample
    long model_stream_generator = 0; // multiplier for the generator for the model streams

	double RandUniform(int strm)
	{
        long seed = model_stream_seeds[strm];
        long long product = model_stream_generator;
        product *= seed;
        seed = product % mg_modulus;
        model_stream_seeds[strm] = seed;
		return (double) seed / (double) mg_modulus;
	}

} // namespace openm

using namespace openm;
void RunModel(IModel * i_model)
{
    theLog->logMsg("Running Simulation");

    int samples = i_model->subSampleCount();
    int sample = i_model->subSampleNumber();
    int seed = i_model->runOptions()->startingSeed;
    int cases = dynamic_cast<ICaseModel *>(i_model)->runOptions()->cases;

    // for sample numbers greater than 40, re-use generators cyclically
    // and increment the starting case_seed for the simulation of the sample
    case_seed = seed + (int) sample / max_samples;
    case_seed_generator = case_seed_generators[sample % max_samples];
    model_stream_generator = model_stream_generators[sample % max_samples];

    for ( int thisCase = 0; thisCase < cases; thisCase++ ) {

        {
            // initialize the stream generators
            long seed = case_seed;
            for ( int model_stream = 0; model_stream < model_streams; model_stream++ ) {
                model_stream_seeds[model_stream] = seed;
                long long product = model_stream_seed_generator;
                product *= seed;
                seed = product % mg_modulus;
            }
        }

        // simulate the case
        StartCase( thisCase );
        while ( DoNextEvent() );
        EndCase();

        {
            // generate the case seed for the next case
            long long product = case_seed_generator;
            product *= case_seed;
            case_seed = product % mg_modulus;
        }
    } // cases
}

