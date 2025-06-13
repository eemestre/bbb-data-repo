/*
 * DAQ.c
 *
 *  Created on: Feb 13, 2017
 *      Author: fernando
 */

#include "DAQ.h"
#include "pru.h"
#include <prussdrv.h>
#include <pruss_intc_mapping.h>
#include <pthread.h>
#include <string.h>

#define PRU0 0
#define PRU1 1


int DAQ_Setup(unsigned int addr)
{
	uint32_t *pru0_mem;

   /***** PRU SET UP *****/
	printf("Setting up PRUs... ");
	fflush(stdout);

	if (pru_setup() != 0) {
		fprintf(stderr, "Error setting up the PRU.\n");
		goto error;
	}

	/* Set up the PRU data RAMs */
	pru_mmap(0, &pru0_mem);
	*(pru0_mem) = addr;

	printf("OK!\n");


	/***** BEGIN MAIN PROGRAM *****/
	printf("Starting main program.\n");

	/* Start up PRU0 */
	if (pru_start(PRU0, "pru/ads131e08s_pru0.bin") != 0) {
		fprintf(stderr, "Error starting PRU0.\n");
		goto error;

	}

	/* Start up PRU1 */
	if (pru_start(PRU1, "pru/ads131e08s_pru1.bin") != 0) {
		fprintf(stderr, "Error starting PRU1.\n");
		goto error;
	}


	return 0;

	error:
	return -1;
}

void DAQ_Stop()
{
   /* PRU CLEAN UP */
	printf("Stopping PRUs.\n");
	pru_stop(PRU1);
	pru_stop(PRU0);
	pru_cleanup();
}


int DAQ_Run(void * to, void * from, int samples)
{
	/* Wait for PRU_EVTOUT_0 and send shared RAM data */
	int n = prussdrv_pru_wait_event(PRU_EVTOUT_0);
	prussdrv_pru_clear_event(PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);
	memcpy(to, from, samples);

	return n;
}
