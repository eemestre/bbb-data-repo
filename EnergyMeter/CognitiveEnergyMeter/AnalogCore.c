/*
 * AnalogCore.c
 *
 *  Created on: May 21, 2017
 *      Author: fernando
 */

/*This system collects data at 15360 Hz.
 *
 *Each pru_packet gets four samples of all channels and store it at 100 bytes of ram.
 *This packet has the following sequence:
 *
 * ------------------------------------------------------------------------------------
 * |CH00|CH01|CH02|...|CH07|CH10|CH11|CH12|...|CH17|...|CHM0|CHM1|CHM2|...|CHM7|STATUS|
 * ------------------------------------------------------------------------------------
 *
 * Each sequence of 24 bytes represents instantaneous sampling of 8 channels.
 * 4x24bytes = 96 bytes.
 * The remaining bytes are relative to the status word of the last conversion.
 *
 * It means that 3840 pru_packets are equal to one cycle of signal. Therefore, the size of
 * buffer to store this information is: 3840x100bytes.
 *
 * The pru engine to capture samples is divided into two steps. So, the whole process gets
 * two periods of signal.
 *
 * */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "AnalogCore.h"
#include <sys/mman.h>
#include "Mem.h"
#include "DAQ.h"
#include "Board.h"

#define ADC_RES (8388607.0)
#define ADC_REF (4.0)

#define PKG_SIZE 100
#define PKG_SAMPLES 4
#define PKG_COUNT (ENERGY_METER_SAMPLE_RATE/PKG_SAMPLES)
#define PKG_TOTAL PKG_COUNT
#define CHANNELS 8

#define PRU_PACKET_SIZE (PKG_COUNT*PKG_SIZE)
#define PRU_BUFFER_SIZE (2*PRU_PACKET_SIZE)

static void ConvertRawValues(unsigned char * const from, int n);

static float ch_data[CHANNELS][ENERGY_METER_SAMPLE_RATE];

/*PRU memory address*/
static void *mem_map;
static void *ram_addr;
static unsigned int addr;


/*Raw data values*/
static unsigned char dbuffer[PRU_PACKET_SIZE];
static int buffer_ix; /*control variable to perform double buffer operations*/


int AnalogCore_Init(void)
{
	int status;

	buffer_ix = 0;

    /*Try to allocate ram memory to share with PRU*/
    status = Allocate_RAM(&mem_map, &ram_addr, &addr, PRU_BUFFER_SIZE);


    if(status != 0) {
    	printf("Error allocating RAM.\n");
    	return -1;
    }

    return 0;
}

void AnalogCore_Deinit(void)
{
	Release_RAM(mem_map, PRU_BUFFER_SIZE);
}

int AnalogCore_Setup(struct tAnalogCoreSetup * pAnalog)
{
	int status = DAQ_Setup(addr);
	int idx = 0;
	buffer_ix = 0;

	if(pAnalog != NULL){
		for(idx = 0; idx < AN_CHANNELS; idx++){
			pAnalog->channel[idx] = &ch_data[idx][0];
		}
	}

	return status;

}

int AnalogCore_GetSamples(struct tAnalogCoreSetup * pAnalog)
{
	int EventN = 0;

	/*check which buffer will be used to store next 1 second of samples*/
	if(buffer_ix == 0){
		EventN = DAQ_Run(dbuffer, (void*)ram_addr, PRU_PACKET_SIZE);
		buffer_ix = 1;
	}
	else{
		EventN = DAQ_Run(dbuffer, (void *)((unsigned char*)ram_addr + PRU_PACKET_SIZE), PRU_PACKET_SIZE);
		buffer_ix = 0;
	}

	/*
	DAQ_Stop();
	FILE *file;
	file = fopen("logs/raw.bin", "w+");
	fwrite(dbuffer, sizeof(char), PRU_PACKET_SIZE, file);
	fclose(file);
	*/

	/*Convert adc raw values to voltage and store it in channel buffer*/
	ConvertRawValues(dbuffer, 8);

	return EventN;
}


static void ConvertRawValues(unsigned char * const from, int n)
{
	struct tSample {
		union tRawValue{
			unsigned char bn[4];
			int32_t value;
		}raw;
		float value;

	}sample;

	int i, j, k;
	int p_idx = 0;
	volatile int s_idx = 0;

	unsigned char * pbuffer_endiann = (unsigned char *)from;

	sample.raw.value = 0;

	/*for each pru_packet 'i' contained in buffer.*/
	for (i = 0; i < PKG_TOTAL; i++)
	{
		/*point to the next packet - the first four bytes of each packet are discarded*/
		p_idx = i*PKG_SIZE + 4;

		/*for each packet of samples contained in pru_packet[i].*/
		for (j = 0; j < PKG_SAMPLES; j++)
		{
			pbuffer_endiann = (unsigned char *)from + p_idx;

			/*for each channel in packet[j]*/
			for (k = 0; k < n; k++)
			{
				sample.raw.bn[3] = 0;
				sample.raw.bn[2] = *(pbuffer_endiann + 0);
				sample.raw.bn[1] = *(pbuffer_endiann + 1);
				sample.raw.bn[0] = *(pbuffer_endiann + 2);

				pbuffer_endiann += 3;

				sample.raw.value = (sample.raw.value << 8);
				sample.raw.value = (sample.raw.value >> 8);

				sample.value = (float)(sample.raw.value * ADC_REF);
				sample.value = (float)sample.value / ADC_RES;

				ch_data[k][s_idx] = sample.value;
			}

			/*point to the next samples*/
			p_idx = p_idx + 24;
			s_idx++;
		}
	}
}