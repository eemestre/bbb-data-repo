/*
 * Goertzel.c
 *
 *  Created on: May 6, 2017
 *      Author: fernando
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "Goertzel.h"

#define GOERTZEL_PI 3.141592653589793238462643383279

int Goertzel_Init(struct Goertzel * me, int points, float Fsample, float Fdtm)
{
	if(me != NULL){

		if(points > 0){

			me->Points = points;
			me->Fsample = Fsample;
			me->Fdtm = Fdtm;
			me->coeff = (float *)malloc(me->Points * sizeof(float));

			if(me->coeff != NULL)
			{
				printf("Goertzel ok!\n");
				return 0;
			}
		}
	}

	printf("Goertzel error!\n");


	return -1;
}

int Goertzel_EvaluateCoefficients(struct Goertzel * me)
{
	int j;
	float normalizedfreq, k, w;

	if(me != NULL){

		for(j = 0; j < me->Points; j++){

		    normalizedfreq = me->Fdtm*(j) / me->Fsample;

		    k = floor(0.5f + (me->Points * normalizedfreq));

		    w = (2 * GOERTZEL_PI * k) / (float)me->Points;

		    *(me->coeff + j) = 2*cos(w);
		}

		return 0;
	}

	return -1;
}

int Goertzel_CalculateAll(struct Goertzel * me, float * input, float * output)
{
	int i,j;
	float s_prev, s_prev2, s, power, coeff;

	if(me != NULL){

		for(j = 0; j < (me->Points/2); j++){

		    s_prev = 0;
		    s_prev2 = 0;
		    coeff = *(me->coeff+j);

		    for(i = 0; i < me->Points; i++){
		        s = *(input+i) + (coeff * s_prev) - s_prev2;
		        s_prev2 = s_prev;
		        s_prev = s;
		    }

		    power = (s_prev2 * s_prev2) + (s_prev * s_prev) - (coeff * s_prev * s_prev2);
		    power = sqrt(power/(float)me->Points);

		    *(output + j) = power;
		}

		return 0;
	}

	return -1;
}

int Goertzel_CalculateOdd(struct Goertzel * me, float * input, float * output, int n)
{
	int i,j, k;
	float s_prev, s_prev2, s, power, coeff;


	if(me != NULL){

		k = 0;

		for(j = 1; k < n; j+=2){

		    s_prev = 0;
		    s_prev2 = 0;
		    coeff = *(me->coeff+j);

		    for(i = 0; i < me->Points; i++){
		        s = *(input+i) + (coeff * s_prev) - s_prev2;
		        s_prev2 = s_prev;
		        s_prev = s;
		    }

		    power = (s_prev2 * s_prev2) + (s_prev * s_prev) - (coeff * s_prev * s_prev2);
		    power = (sqrt(power)/((float)me->Points)) * ((float)1414213562/(float)1000000000);

		    *(output + k) = power;
		    k++;
		}

		return 0;
	}

	return -1;
}
