/*
 * Waveform.c
 *
 *  Created on: Aug 4, 2017
 *      Author: fernando
 */

#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdio.h>


#include "Waveform.h"

void SaveWaveform(char * fname, float * wf, int len)
{
	FILE *file;
	static char fn[30];

	sprintf(fn,"%s.wvf", fname);

	file = fopen(fn, "w+");

	if (file == NULL) {
		return;
	}
	else
	{
		fwrite(wf, sizeof(float), len, file);
		fclose(file);
		printf("Arquivo gravado: %s\n", fn);
	}
}

int FindZero(float * waveform, int len)
{
	float c1_v1;
	int k;

	k = 0;

	c1_v1 = *waveform;

	if (c1_v1 < 0)
	{
		do
		{
			c1_v1 = *waveform++;
			k++;
		} while ((c1_v1 < 0) && (k < len));

		waveform--;
		k--;
	}

	do
	{
		c1_v1 = *waveform++;
		k++;
	} while ((c1_v1 > 0) && (k < len));

	return (k - 1);
}


void WaveformDiff(struct VI_Signature * prior, struct VI_Signature * current, struct VI_Signature * out, int offset12, int start_end)
{
	float *v;
	float * i1 = prior->I_Waveform;
	float * i2 = current->I_Waveform;
	float * out_i = out->I_Waveform;
	float * out_v = out->V_Waveform;
	int k, k1, k2;

	if(start_end == 1){
		i1 = i1 + prior->Samples - out->Samples - 512;
		i2 = i2 + current->Samples - out->Samples - 512;
	}

	k1 = FindZero(prior->V_Waveform, 512);
	k2 = FindZero(current->V_Waveform, 512);

	printf("\nV1 %f\nV2 %f\n", *(prior->V_Waveform+k1-1), *(current->V_Waveform+k2-1));
	printf("\nV1 %f\nV2 %f\n\n", *(prior->V_Waveform+k1), *(current->V_Waveform+k2));

	i1 = i1 + k1;
	i2 = i2 + k2;

	if(offset12 == 2){
		v = current->V_Waveform + k2;

		if(start_end == 1){
			v = v + current->Samples - out->Samples - 512;
		}
	}
	else{
		v = prior->V_Waveform + k1;

		if(start_end == 1){
			v = v + prior->Samples - out->Samples - 512;
		}
	}

	for (k = 0; k < out->Samples; k++)
	{
		*out_i++ = *i2++ - *i1++;
		*out_v++ = *v++;
	}
}

void CopyWaveform(int offset, float * vchannel, float * ichannel, float *wvf_v, float *wvf_i)
{
	int i;
	float *pts_Isamples, *pts_Vsamples;
	float *pte_Isamples, *pte_Vsamples;
	float *dst_V, *dst_I;

	/*Get correct sequence of waveform*/
	pts_Vsamples = vchannel + offset - 256;
	pte_Vsamples = vchannel + offset;
	pts_Isamples = ichannel + offset - 256;
	pte_Isamples = ichannel + offset;


	dst_V = wvf_v;
	dst_I = wvf_i;


	for(i = 0; i < 256; i++)
	{
		*dst_V++ = *pts_Vsamples;
		*dst_I++ = *pts_Isamples;

		pts_Vsamples += 1;
		pts_Isamples += 1;
	}

	for(i = 256; i < 512; i++)
	{
		*dst_V++ = *pte_Vsamples;
		*dst_I++ = *pte_Isamples;

		pte_Vsamples += 1;
		pte_Isamples += 1;
	}
}
