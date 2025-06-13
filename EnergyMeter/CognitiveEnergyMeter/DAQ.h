/*
 * DAQ.h
 *
 *  Created on: Feb 13, 2017
 *      Author: fernando
 */

#ifndef DAQ_H_
#define DAQ_H_

#ifdef __cplusplus
extern "C" {
#endif

int DAQ_Setup(unsigned int addr);
void DAQ_Stop();
int DAQ_Run(void * to, void * from, int samples);

#ifdef __cplusplus
}
#endif

#endif /* DAQ_H_ */
