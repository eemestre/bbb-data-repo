/*
 * CPT.c
 *
 *  Created on: Mar 22, 2017
 *      Author: fernando
 */



// CPT.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "CPT.h"
#include "MAF.h"

#define USE_THRESHOLD (1)
#define THRESHOLD ((double)0)
#define THR_MIN (0.0001)

static inline double RMS_Evaluate(double input);

static void Phase_Init(struct tPhase * me);



static void CPT_EvaluateFactors(struct tCPT * me, struct tPAC * pac);
static void CPT_PowerDecomposition(struct tCPT * me, struct tPAC * pac);
static void CPT_CurrentDecomposition(struct tCPT * me, struct tPAC * pac);
static void CPT_UpdateCollectiveValues(struct tCPT * me, struct tPAC * pac);
static void CPT_UpdatePowerAndEnergy(struct tCPT * me, struct tPAC * pac);
static void CPT_UpdateVoltagesAndCurrents(struct tCPT * me, struct tPAC * pac);



static double RMS_Evaluate(double input)
{
	if (input <= (double)0)
		input = ((double)1/(double)100000);

	return sqrt(input);
}

static void Phase_Init(struct tPhase * me)
{
	if (me != NULL)
	{
		me->V_rms = 0;
		me->I_rms = 0;

		me->Integral = 0;
		me->Integral_lastInput = 0;
		me->UnbiasedIntegral = 0;
		me->UnbiasedIntegral_rms = 0;

		me->P_avg = 0;
		me->W_avg = 0;
	}
}

static void CPT_UpdateVoltagesAndCurrents(struct tCPT * me, struct tPAC * pac)
{
	int i;

	for (i = 0; i < me->n; i++)
	{
		struct tPhase * Phase = me->Phase[i];

		/*RMS value for voltage*/
		Phase->V_pow2 = pac->V[i] * pac->V[i];
		MovingAverageFilter_Update(&Phase->V_maf, Phase->V_pow2);
		Phase->V_rms = RMS_Evaluate(Phase->V_maf.MeanValue);

		/*RMS value for current*/
		Phase->I_pow2 = pac->I[i] * pac->I[i];
		MovingAverageFilter_Update(&Phase->I_maf, Phase->I_pow2);
		Phase->I_rms = RMS_Evaluate(Phase->I_maf.MeanValue);

		/*Voltage Integral*/
		Phase->Integral = Phase->Integral + (me->TSH / (double)2)*(pac->V[i] + Phase->Integral_lastInput);
		Phase->Integral_lastInput = pac->V[i];
		MovingAverageFilter_Update(&Phase->INT_maf, Phase->Integral);

		/*Unbiased Voltage Integral*/
		Phase->UnbiasedIntegral = Phase->Integral - Phase->INT_maf.MeanValue;
		Phase->UnbiasedIntegral_pow2 = Phase->UnbiasedIntegral * Phase->UnbiasedIntegral;
		MovingAverageFilter_Update(&Phase->UI_maf, Phase->UnbiasedIntegral_pow2);
		Phase->UnbiasedIntegral_rms = RMS_Evaluate(Phase->UI_maf.MeanValue);
	}
}

static void CPT_UpdatePowerAndEnergy(struct tCPT * me, struct tPAC * pac)
{
	int i;

	for (i = 0; i < me->n; i++)
	{
		struct tPhase * Phase = me->Phase[i];

		Phase->P_inst = pac->V[i] * pac->I[i];

		/*Average of Active Power*/
		Phase->P_avg = MovingAverageFilter_Update(&Phase->P_maf, Phase->P_inst);

		if (Phase->P_avg < ((double)1/(double)100000))
			Phase->P_avg = ((double)1/(double)100000);

		Phase->W_inst = Phase->UnbiasedIntegral * pac->I[i];

		/*Average of Reactive Energy*/
		Phase->W_avg = MovingAverageFilter_Update(&Phase->W_maf, Phase->W_inst);


		/*Average of Reactive Power*/
		Phase->Q = Phase->W_avg *(Phase->V_rms / Phase->UnbiasedIntegral_rms);
	}
}

static void CPT_UpdateCollectiveValues(struct tCPT * me, struct tPAC * pac)
{
	int i;

	me->P_avg = 0;
	me->W_avg = 0;
	me->V_rms = 0;
	me->I_rms = 0;
	me->UI_rms = 0;

	for (i = 0; i < me->n; i++)
	{
		struct tPhase * Phase = me->Phase[i];

		me->P_avg = me->P_avg + Phase->P_avg;
		me->W_avg = me->W_avg + Phase->W_avg;

		me->V_rms = me->V_rms + Phase->V_pow2;
		me->I_rms = me->I_rms + Phase->I_pow2;
		me->UI_rms = me->UI_rms + Phase->UnbiasedIntegral_pow2;
	}

	me->I_rms = MovingAverageFilter_Update(&me->I_maf, me->I_rms);
	me->V_rms = MovingAverageFilter_Update(&me->V_maf, me->V_rms);
	me->UI_rms = MovingAverageFilter_Update(&me->UI_maf, me->UI_rms);

	me->V_rms = RMS_Evaluate(me->V_rms);
	me->I_rms = RMS_Evaluate(me->I_rms);
	me->UI_rms = RMS_Evaluate(me->UI_rms);
}

static void CPT_CurrentDecomposition(struct tCPT * me, struct tPAC * pac)
{
	int i;

	me->Iv_rms = 0;

	for (i = 0; i < me->n; i++)
	{
		struct tPhase * Phase = me->Phase[i];

		if (Phase->V_rms != 0)
		{
			Phase->Ia_ActiveCurrent = (Phase->P_avg / (Phase->V_rms * Phase->V_rms)) * pac->V[i];
		}

		if (Phase->UnbiasedIntegral_rms != 0)
		{
			Phase->Ir_ReactiveCurrent = (Phase->W_avg / (Phase->UnbiasedIntegral_rms * Phase->UnbiasedIntegral_rms)) * Phase->UnbiasedIntegral;
		}

		Phase->Iv_VoidCurrent = pac->I[i] - Phase->Ia_ActiveCurrent - Phase->Ir_ReactiveCurrent;
		Phase->Iv_rms = (Phase->Iv_VoidCurrent * Phase->Iv_VoidCurrent);

		me->Iv_rms = me->Iv_rms + Phase->Iv_rms;

		MovingAverageFilter_Update(&Phase->Iv_maf, Phase->Iv_rms);
		Phase->Iv_rms = RMS_Evaluate(Phase->Iv_maf.MeanValue);
	}

	MovingAverageFilter_Update(&me->Iv_maf, me->Iv_rms);
	me->Iv_rms = RMS_Evaluate(me->Iv_maf.MeanValue);


	if (me->V_rms != 0)
	{
		for (i = 0; i < me->n; i++)
		{
			struct tPhase * Phase = me->Phase[i];
			Phase->Iba_BalancedActiveCurrent = (me->P_avg / (me->V_rms * me->V_rms)) * pac->V[i];
		}
	}

	if (me->UI_rms != 0)
	{
		for (i = 0; i < me->n; i++)
		{
			struct tPhase * Phase = me->Phase[i];
			Phase->Ibr_BalancedReactiveCurrent = (me->W_avg / (me->UI_rms * me->UI_rms)) * Phase->UnbiasedIntegral;
		}
	}


	/*Balanced current*/
	me->Iba_rms = 0;
	me->Ibr_rms = 0;
	for (i = 0; i < me->n; i++)
	{
		struct tPhase * Phase = me->Phase[i];

		Phase->Iba_rms = (Phase->Iba_BalancedActiveCurrent * Phase->Iba_BalancedActiveCurrent);
		Phase->Ibr_rms = (Phase->Ibr_BalancedReactiveCurrent * Phase->Ibr_BalancedReactiveCurrent);
		me->Iba_rms = me->Iba_rms + Phase->Iba_rms;
		me->Ibr_rms = me->Ibr_rms + Phase->Ibr_rms;

		MovingAverageFilter_Update(&Phase->Iba_maf, Phase->Iba_rms);
		Phase->Iba_rms = RMS_Evaluate(Phase->Iba_maf.MeanValue);

		MovingAverageFilter_Update(&Phase->Ibr_maf, Phase->Ibr_rms);
		Phase->Ibr_rms = RMS_Evaluate(Phase->Ibr_maf.MeanValue);
	}


	MovingAverageFilter_Update(&me->Iba_maf, me->Iba_rms);
	me->Iba_rms = RMS_Evaluate(me->Iba_maf.MeanValue);
	MovingAverageFilter_Update(&me->Ibr_maf, me->Ibr_rms);
	me->Ibr_rms = RMS_Evaluate(me->Ibr_maf.MeanValue);


	if(me->n != 1){ /*check if CPT is running only for mono*/

		/*Unbalanced current*/
		for (i = 0; i < me->n; i++)
		{
			struct tPhase * Phase = me->Phase[i];

			Phase->Iua_UnbalancedActiveCurrent = Phase->Ia_ActiveCurrent - Phase->Iba_BalancedActiveCurrent;
			Phase->Iur_UnbalancedReactiveCurrent = Phase->Ir_ReactiveCurrent - Phase->Ibr_BalancedReactiveCurrent;
			Phase->Iu_UnbalancedCurrent = Phase->Iua_UnbalancedActiveCurrent + Phase->Iur_UnbalancedReactiveCurrent;
		}


		me->Iua_rms = 0;
		me->Iur_rms = 0;

		for (i = 0; i < me->n; i++)
		{
			struct tPhase * Phase = me->Phase[i];
			Phase->Iua_rms = (Phase->Iua_UnbalancedActiveCurrent * Phase->Iua_UnbalancedActiveCurrent);
			Phase->Iur_rms = (Phase->Iur_UnbalancedReactiveCurrent * Phase->Iur_UnbalancedReactiveCurrent);

			me->Iua_rms = me->Iua_rms + Phase->Iua_rms;
			me->Iur_rms = me->Iur_rms + Phase->Iur_rms;
		}

		MovingAverageFilter_Update(&me->Iua_maf, me->Iua_rms);
		me->Iua_rms = RMS_Evaluate(me->Iua_maf.MeanValue);
		MovingAverageFilter_Update(&me->Iur_maf, me->Iur_rms);
		me->Iur_rms = RMS_Evaluate(me->Iur_maf.MeanValue);
	}
}

static void CPT_PowerDecomposition(struct tCPT * me, struct tPAC * pac)
{
	int i;

	for (i = 0; i < me->n; i++)
	{
		struct tPhase * Phase = me->Phase[i];

		Phase->A = Phase->V_rms * Phase->I_rms;
		Phase->Q = Phase->V_rms * Phase->Ibr_rms;
		Phase->Na = Phase->V_rms * Phase->Iua_rms;
		Phase->Nr = Phase->V_rms * Phase->Iur_rms;
		Phase->N = sqrt((Phase->Na * Phase->Na) + (Phase->Nr * Phase->Nr));
		Phase->V = Phase->V_rms * Phase->Iv_rms;
	}

	me->A = me->V_rms * me->I_rms;
	me->Q = me->V_rms * me->Ibr_rms;
	me->Na = me->V_rms * me->Iua_rms;
	me->Nr = me->V_rms * me->Iur_rms;
	me->N = sqrt((me->Na * me->Na) + (me->Nr * me->Nr));
	me->V = me->V_rms * me->Iv_rms;
}

static void CPT_EvaluateFactors(struct tCPT * me, struct tPAC * pac)
{
	if (me->A != 0)
	{
		me->PF = me->P_avg / me->A;
		me->VF = me->V / me->A;
	}

	double P2 = me->P_avg * me->P_avg;
	double Q2 = me->Q * me->Q;
	double N2 = me->N * me->N;

	if ((P2 + Q2 + N2) > 0)
	{
		me->NF = me->N / sqrt(P2 + Q2 + N2);
	}

	if ((P2 + Q2) > 0)
	{
		me->QF = me->Q / sqrt(P2 + Q2);
	}

	int i;

	for (i = 0; i < me->n; i++)
	{
		struct tPhase * Phase = me->Phase[i];

		if (Phase->A != 0)
		{
			Phase->PF = Phase->P_avg / Phase->A;
			Phase->VF = Phase->V / Phase->A;

			if(Phase->VF>1)
				Phase->VF = 1;
		}

		P2 = Phase->P_avg * Phase->P_avg;
		Q2 = Phase->Q * Phase->Q;
		N2 = Phase->N * Phase->N;

		if ((P2 + Q2 + N2) > 0)
		{
			Phase->NF = Phase->N / sqrt(P2 + Q2 + N2);
		}

		if ((P2 + Q2) > 0)
		{
			Phase->QF = Phase->Q / sqrt(P2 + Q2);
		}
	}
}



void CPT_Update(struct tCPT * me, struct tPAC * pac)
{
	CPT_UpdateVoltagesAndCurrents(me, pac);
	CPT_UpdatePowerAndEnergy(me, pac);
	CPT_UpdateCollectiveValues(me, pac);
	CPT_CurrentDecomposition(me, pac);
	CPT_PowerDecomposition(me, pac);
	CPT_EvaluateFactors(me, pac);
}


int CPT_Config(struct tCPT * me, uint freq, uint samples, uint n)
{
	#define PHASE_RAW_BUFFERS 9
	#define CPT_RAW_BUFFERS 8

	int i,j;

	struct tBuffer * Phase_Buffers = malloc(sizeof(struct tBuffer) * PHASE_RAW_BUFFERS * n);

	if(Phase_Buffers == NULL){
		printf("Error Phase Buffer allocation\n");
		goto exit_error_alloc;
	}

	struct tBuffer * CPT_Buffers = malloc(sizeof(struct tBuffer) * CPT_RAW_BUFFERS * n);

	if(CPT_Buffers == NULL){
		printf("Error CPT Buffer allocation\n");
		goto exit_error_alloc;
	}

	for(j = 0 ; j < 3; j++){
		me->Phase[j] = NULL;

		if(j < n){
			me->Phase[j] = malloc(sizeof(struct tPhase));
			if(me->Phase[j] == NULL){
				printf("Error Phase allocation\n");
				goto exit_error_alloc;
			}
		}
	}

	for(j = 0 ; j < n; j++){
		for (i = 0; i < PHASE_RAW_BUFFERS; i++){
			int k = (j*PHASE_RAW_BUFFERS) + i;

			Phase_Buffers[k].Data = malloc(samples * sizeof(double));
			Phase_Buffers[k].Length = samples;

			if(Phase_Buffers[k].Data == NULL){
				printf("Error Phase_Raw allocation\n");
				goto exit_error_alloc;
			}
		}
	}

	for(j = 0 ; j < n; j++){
		for (i = 0; i < CPT_RAW_BUFFERS; i++){
			int k = (j*CPT_RAW_BUFFERS) + i;

			CPT_Buffers[k].Data = malloc(samples * sizeof(double));
			CPT_Buffers[k].Length = samples;

			if(CPT_Buffers[k].Data == NULL){
				printf("Error CPT_Raw allocation\n");
				goto exit_error_alloc;
			}

		}
	}

	for(j = 0 ; j < 3; j++){
		int k = (j*PHASE_RAW_BUFFERS);
		MovingAverageFilter_Init(&me->Phase[j]->V_maf, &Phase_Buffers[k+0], USE_THRESHOLD, THRESHOLD, THR_MIN);
		MovingAverageFilter_Init(&me->Phase[j]->I_maf, &Phase_Buffers[k+1], USE_THRESHOLD, THRESHOLD, THR_MIN);
		MovingAverageFilter_Init(&me->Phase[j]->P_maf, &Phase_Buffers[k+2], 0, 0, 0);
		MovingAverageFilter_Init(&me->Phase[j]->W_maf, &Phase_Buffers[k+3], 0, 0, 0);
		MovingAverageFilter_Init(&me->Phase[j]->UI_maf, &Phase_Buffers[k+4], USE_THRESHOLD, THRESHOLD, THR_MIN);
		MovingAverageFilter_Init(&me->Phase[j]->INT_maf, &Phase_Buffers[k+5], 0, 0, 0);

		MovingAverageFilter_Init(&me->Phase[j]->Iba_maf, &Phase_Buffers[k+6], USE_THRESHOLD, THRESHOLD, THR_MIN);
		MovingAverageFilter_Init(&me->Phase[j]->Ibr_maf, &Phase_Buffers[k+7], USE_THRESHOLD, THRESHOLD, THR_MIN);
		MovingAverageFilter_Init(&me->Phase[j]->Iv_maf, &Phase_Buffers[k+8], USE_THRESHOLD, THRESHOLD, THR_MIN);

		Phase_Init(me->Phase[j]);
	}

	MovingAverageFilter_Init(&me->Iba_maf, &CPT_Buffers[0], USE_THRESHOLD, THRESHOLD, THR_MIN);
	MovingAverageFilter_Init(&me->Iua_maf, &CPT_Buffers[1], USE_THRESHOLD, THRESHOLD, THR_MIN);
	MovingAverageFilter_Init(&me->Ibr_maf, &CPT_Buffers[2], USE_THRESHOLD, THRESHOLD, THR_MIN);
	MovingAverageFilter_Init(&me->Iur_maf, &CPT_Buffers[3], USE_THRESHOLD, THRESHOLD, THR_MIN);
	MovingAverageFilter_Init(&me->Iv_maf, &CPT_Buffers[4], USE_THRESHOLD, THRESHOLD, THR_MIN);
	MovingAverageFilter_Init(&me->I_maf, &CPT_Buffers[5], USE_THRESHOLD, THRESHOLD, THR_MIN);
	MovingAverageFilter_Init(&me->V_maf, &CPT_Buffers[6], USE_THRESHOLD, THRESHOLD, THR_MIN);
	MovingAverageFilter_Init(&me->UI_maf, &CPT_Buffers[7], USE_THRESHOLD, THRESHOLD, THR_MIN);

	me->TSH = (double)1/(double)freq;
	me->samples = samples;
	me->n = n;
	me->Iba_rms = 0;
	me->Iua_rms = 0;
	me->Ibr_rms = 0;
	me->Iur_rms = 0;
	me->Iv_rms = 0;

	return 0;

	exit_error_alloc:
	return -1;
}
