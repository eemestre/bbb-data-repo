/*
 * MetrologyCore.h
 *
 *  Created on: May 21, 2017
 *      Author: fernando
 */

#ifndef METROLOGYCORE_H_
#define METROLOGYCORE_H_

#include "Goertzel.h"
#include "Board.h"

struct tMetrologyCoreChannel{
	float dft[ENERGY_METER_FREQ][DFT_POINTS];
    //int32_t * isamples;
    //float fsamples[ENERGY_METER_SAMPLE_RATE];
    float * fsamples;
};

struct tMetrologyCoreSetup{
	struct tMetrologyCoreChannel IChannels[I_INPUTS];
    struct tMetrologyCoreChannel VChannels[V_INPUTS];
    struct Goertzel goertzel;
    int Points;
    int fs;
    int F;
};


#ifdef __cplusplus
extern "C" {
#endif

int MetrologyCore_Init(struct tMetrologyCoreSetup * me, int points, int fs, int f1);
void MetrologyCore_Scale(struct tMetrologyCoreSetup * me);
void MetrologyCore_CalculateHarmonics(struct tMetrologyCoreSetup * me);



#ifdef __cplusplus
}
#endif

#endif /* METROLOGYCORE_H_ */
