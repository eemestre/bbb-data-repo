/*
 * Board.h
 *
 *  Created on: July 28, 2019
 *      Author: fernando
 */

#ifndef BOARD_H_
#define BOARD_H_

#define DFT_POINTS 10
#define V_INPUTS 3
#define I_INPUTS 3

#define ENERGY_METER_SAMPLE_RATE 15360
#define ENERGY_METER_FREQ 60
#define ENERGY_METER_SAMPLES_PER_CYCLE (uint32_t)((ENERGY_METER_SAMPLE_RATE)/(ENERGY_METER_FREQ))

#ifdef __cplusplus
extern "C" {
#endif


int Board_Init();

#ifdef __cplusplus
}
#endif

#endif /* BOARD_H_ */
