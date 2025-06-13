/*
 * MetrologyCore.c
 *
 *  Created on: May 21, 2017
 *      Author: fernando
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "MetrologyCore.h"
#include "Board.h"


static int CalculateOffset(float * buffer, int samples, float * output);


int MetrologyCore_Init(struct tMetrologyCoreSetup * me, int points, int fs, int f1)
{
	int status;

	if (me == NULL)
		return -1;

    status = Goertzel_Init(&me->goertzel, points, fs, f1);

    if(status == -1){
    	return -1;
    }

	me->Points = points;
	me->fs = fs;
	me->F = f1;

    Goertzel_EvaluateCoefficients(&me->goertzel);

    return 0;
}


void MetrologyCore_CalculateHarmonics(struct tMetrologyCoreSetup * me)
{
	int i, j;
	/*
	for(j = 0; j < V_INPUTS; j++){
		for(i = 0; i < me->F; i++){
			Goertzel_CalculateOdd(&me->goertzel, me->VChannels[j].samples+(i*me->Points), me->VChannels[j].dft[i], DFT_POINTS);
		}
	}
	 */
	
	for(j = 0; j < I_INPUTS; j++){
		for(i = 0; i < me->F; i++){
			Goertzel_CalculateOdd(&me->goertzel, me->IChannels[j].fsamples+(i*me->Points), &me->IChannels[j].dft[i][0], DFT_POINTS);
		}
	}
}


void MetrologyCore_Scale(struct tMetrologyCoreSetup * me)
{
	int i;
	int j;
	int k;
	int t;

	for(j = 0; j < V_INPUTS; j++){
		float offsetV = 0;

		//CalculateOffset(me->VChannels[j].fsamples, me->Points, &offsetV);
		k = 0;
		t = 0;
		for(i = 0; i < me->fs; i++)
		{
			k++;
			if(k >= me->Points){
				k = 0;
				t++;
				//CalculateOffset(me->VChannels[j].fsamples+me->Points*t, me->Points, &offsetV);
			}

			me->VChannels[j].fsamples[i] = ((float)(me->VChannels[j].fsamples[i] - offsetV)) * (160/2.4);
		}
	}

	for(j = 0; j < I_INPUTS; j++){
		float offsetI = 0;

		//CalculateOffset(me->IChannels[j].fsamples, me->Points, &offsetI);
		k = 0;
		t = 0;
		for(i = 0; i < me->fs; i++)
		{
			
			k++;
			if(k >= me->Points){
				k = 0;
				t++;
				//CalculateOffset(me->IChannels[j].fsamples+me->Points*t, me->Points, &offsetI);
			}
			

			me->IChannels[j].fsamples[i] = ((float)(me->IChannels[j].fsamples[i] - offsetI)) * (50/2.4);
		}
	}
}

static int CalculateOffset(float * buffer, int samples, float * output)
{
	int i;
	float offset = 0;

	if(buffer == NULL || samples <= 0){
		return -1;
	}

	for(i = 0; i < samples; i++){
		offset = offset + *buffer++;
	}

	*output = offset/samples;

	return 0;
}
