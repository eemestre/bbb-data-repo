#ifndef PRU_H
#define PRU_H

#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <prussdrv.h>
#include <pruss_intc_mapping.h>

/* Setup PRU driver: start up prussdrv, open interrupt events, */
/* and initialize intc                                         */
int pru_setup();

/* Map PRU DATARAM */
int pru_mmap(int pru_number, uint32_t **pru_mem);

/* Load binary file and start PRU */
int pru_start(int pru_number, char *program);

/* Stop PRU */
int pru_stop(int pru_number);

/* Clean up: stop prussdrv and release PRU clocks */
int pru_cleanup();


#ifdef __cplusplus
}
#endif
#endif PRU_H