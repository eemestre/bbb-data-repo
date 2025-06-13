/*
 * Goertzel.h
 *
 *  Created on: May 6, 2017
 *      Author: fernando
 */

#ifndef GOERTZEL_H_
#define GOERTZEL_H_

#ifdef __cplusplus
extern "C" {
#endif

struct Goertzel
{
	int Points;
	float Fdtm;
	float Fsample;
	float * coeff;
};


int Goertzel_Init(struct Goertzel * me, int points, float Fsample, float Fdtm);
int Goertzel_EvaluateCoefficients(struct Goertzel * me);
int Goertzel_CalculateAll(struct Goertzel * me, float * input, float * output);
int Goertzel_CalculateOdd(struct Goertzel * me, float * input, float * output, int n);

#ifdef __cplusplus
}
#endif

#endif /* GOERTZEL_H_ */
