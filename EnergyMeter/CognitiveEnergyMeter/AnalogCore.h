/*
 * AnalogCore.h
 *
 *  Created on: May 21, 2017
 *      Author: fernando
 */

#ifndef ANALOGCORE_H_
#define ANALOGCORE_H_

#ifdef __cplusplus
extern "C" {
#endif

#define AN_CHANNELS 8

struct tAnalogCoreSetup{
	float * channel[AN_CHANNELS];
};

int AnalogCore_Init(void);
void AnalogCore_Deinit(void);
int AnalogCore_Setup(struct tAnalogCoreSetup * pAnalog);
int AnalogCore_GetSamples(struct tAnalogCoreSetup * pAnalog);


#ifdef __cplusplus
}
#endif

#endif /* ANALOGCORE_H_ */
