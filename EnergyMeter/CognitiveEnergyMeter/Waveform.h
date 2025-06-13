/*
 * Waveform.h
 *
 *  Created on: Aug 4, 2017
 *      Author: fernando
 */

#ifndef WAVEFORM_H_
#define WAVEFORM_H_

#ifdef __cplusplus
extern "C" {
#endif

struct VI_Signature{
	float *  V_Waveform;
	float * I_Waveform;
	int Samples;
};

void SaveWaveform (char * fname, float * wf, int len);
int FindZero(float * waveform, int len);
void WaveformDiff(struct VI_Signature * prior, struct VI_Signature * current, struct VI_Signature * out, int offset12, int start_end);
void CopyWaveform(int offset, float * vchannel, float * ichannel, float *wvf_v, float *wvf_i);

#ifdef __cplusplus
}
#endif

#endif /* WAVEFORM_H_ */
