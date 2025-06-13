/*
 * CPT.h
 *
 *  Created on: Mar 22, 2017
 *      Author: fernando
 */

#ifndef CPT_H_
#define CPT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "MAF.h"

struct tPAC
{
	/*Phase A voltage*/
	/*Phase B voltage*/
	/*Phase C voltage*/
	double V[3];

	/*Phase A current*/
	/*Phase B current*/
	/*Phase C current*/
	double I[3];
};



struct tPhase
{
	struct tMAF V_maf;
	struct tMAF I_maf;
	struct tMAF P_maf;
	struct tMAF W_maf;
	struct tMAF UI_maf;
	struct tMAF INT_maf;

	struct tMAF Iba_maf;
	struct tMAF Ibr_maf;
	struct tMAF Iv_maf;

	double V_pow2;
	double V_rms;
	double I_pow2;
	double I_rms;

	double Integral;
	double Integral_lastInput;

	double UnbiasedIntegral;
	double UnbiasedIntegral_pow2;
	double UnbiasedIntegral_rms;

	double W_inst;
	double P_inst;

	double W_avg;
	double P_avg;
	double Q;

	double Ia_ActiveCurrent;
	double Iba_BalancedActiveCurrent;
	double Iua_UnbalancedActiveCurrent;

	double Ir_ReactiveCurrent;
	double Ibr_BalancedReactiveCurrent;
	double Iur_UnbalancedReactiveCurrent;

	double Iv_VoidCurrent;
	double Iu_UnbalancedCurrent;

	double Iba_rms;
	double Iua_rms;
	double Ibr_rms;
	double Iur_rms;
	double Iv_rms;

	double A;
	double Na;
	double Nr;
	double N;
	double V;

	double PF;
	double NF;
	double QF;
	double VF;
};

struct tPhaseBuffer
{
	double * V_buffer;
	double * I_buffer;
	double * P_buffer;
	double * W_buffer;
	double * UI_buffer;
	double * INT_buffer;
	int Length;			/*Size of buffers*/
};

struct tCPT
{
	struct tPhase * Phase[3];

	struct tMAF Iba_maf;
	struct tMAF Iua_maf;
	struct tMAF Ibr_maf;
	struct tMAF Iur_maf;
	struct tMAF Iv_maf;
	double Iba_rms;
	double Iua_rms;
	double Ibr_rms;
	double Iur_rms;
	double Iv_rms;

	struct tMAF V_maf;
	struct tMAF I_maf;
	struct tMAF UI_maf;
	double V_rms;
	double I_rms;
	double UI_rms;
	double P_avg;
	double W_avg;

	double A;
	double Q;
	double Na;
	double Nr;
	double N;
	double V;

	double PF;
	double NF;
	double QF;
	double VF;

	double TSH;
	uint samples;
	uint n;
};

int CPT_Config(struct tCPT * me, uint freq, uint samples, uint n);
void CPT_Update(struct tCPT * me, struct tPAC * pac);

#ifdef __cplusplus
}
#endif

#endif /* CPT_H_ */
