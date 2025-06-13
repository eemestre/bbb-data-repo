#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "Mem.h"
#include "DAQ.h"
#include "Board.h"
#include "CPT.h"
#include "Goertzel.h"
#include "MetrologyCore.h"
#include "AnalogCore.h"
#include "Waveform.h"
#include "MAF.h"

#define NOTIFY_ERROR(MSG) fprintf(stderr, "EnergyMeter: %s", MSG)
#define LOGFILE "./raw.bin"

int WriteLog(float * ptr_params, int cnt_params, int chunks);

typedef enum _e_attributes{
	ATTR_VOLTAGE_RMS,
	ATTR_CURRENT_RMS,
	ATTR_POWER_APPARENT,
	ATTR_POWER_ACTIVE,
	ATTR_ENERGY_REACTIVE,
	ATTR_CURRENT_ACTIVE,
	ATTR_CURRENT_REACTIVE,
	ATTR_CURRENT_VOID,
	ATTR_PF,
	ATTR_QF,
	ATTR_VF,
	ATTR_MAX
}e_attributes;

#define APP_PHASES 3
#define NUMBER_OF_MAF_BUFFERS ATTR_MAX*APP_PHASES
#define RAW_PARAM (ATTR_MAX+1) //tive que deixar para para não ocorrer BUS_ERROR no momento de gravar o arquivo raw.bin
#define RAW_PARAM_FREQ 40
#define RAW_PARAM_FREQ_COUNT (ENERGY_METER_SAMPLE_RATE/RAW_PARAM_FREQ)


#if APP_PHASES == 3
	static const int phase_order[3] = {0, 1, 2};
#elif APP_PHASES == 1
	static const int phase_order[3] = {1, 0, 2}; //somente fase B
#else
	#error "PHASES"
#endif

static const char phase[] = {'A', 'B', 'C'};

/*Buffers para calcular os valores médios*/
static struct tBuffer Buffers[NUMBER_OF_MAF_BUFFERS];
static struct tMAF _maf[NUMBER_OF_MAF_BUFFERS];
static const int PARAMS_LEN = RAW_PARAM*APP_PHASES;
static float params[RAW_PARAM_FREQ*PARAMS_LEN];
static void update_mean_values(struct tCPT * CPT);
static void update_raw_values(struct tCPT * CPT, int offset);
static void send_data(int socket_desc);

int main(int argc, const char **argv)
{
	/*CPT data struct*/
	struct tCPT CPT;

	struct tAnalogCoreSetup AnalogCore;
	struct tMetrologyCoreSetup MetrologyCore;

    /*Variables used to check if there is a missed packet*/
    unsigned int lastEventN = 0;
    unsigned int EventN;

    /*Variables used to control flow and operation mode*/
    int stop_daq = 1;
    int code = EXIT_FAILURE;

    int status;
	
    /* verifica se o programa foi executado como root */
    if (getuid() != 0) {
    	NOTIFY_ERROR("This program needs to run as root.\n");
        code = -1;
        goto exit_failure;
    }

    {
		struct sched_param sched_param;
		sched_param.sched_priority = 1;

		/*habilita escalonamento para modo de tempo real*/
		if(sched_setscheduler(getpid(), SCHED_RR, &sched_param)==-1){
			NOTIFY_ERROR("sched_setscheduler failed");
		   code = -1;
		   goto exit_failure;
		}
    }

    /*Desativa swap de memória*/
    if(mlockall(MCL_FUTURE|MCL_CURRENT)){
    	NOTIFY_ERROR("Failed to lock memory\n");
    	code = -1;
    	goto exit_failure;
    }

    status = AnalogCore_Init();

    if(status != 0) {
    	NOTIFY_ERROR("Error analog core.\n");
    	code = -1;
    	goto exit_failure_ram;
    }

	Board_Init();

    status = MetrologyCore_Init(&MetrologyCore, ENERGY_METER_SAMPLES_PER_CYCLE, ENERGY_METER_SAMPLE_RATE, ENERGY_METER_FREQ);

    if(status == -1){
    	NOTIFY_ERROR("MetrologyCore error.\n");
        code = -1;
        goto exit_failure;
    }

	/*Setup PRU before start daq*/
    status = AnalogCore_Setup(&AnalogCore);

    if(status != 0) {
    	NOTIFY_ERROR("Error setting up PRU.\n");
    	code = -1;
        goto exit_failure_pru;
    }

	//canais de tensão A e C estão invertidos.
	MetrologyCore.VChannels[0].fsamples = AnalogCore.channel[2]; //3
	MetrologyCore.VChannels[1].fsamples = AnalogCore.channel[1]; //2
	MetrologyCore.VChannels[2].fsamples = AnalogCore.channel[0]; //1
	MetrologyCore.IChannels[0].fsamples = AnalogCore.channel[3]; //4
	MetrologyCore.IChannels[1].fsamples = AnalogCore.channel[6]; //6
	MetrologyCore.IChannels[2].fsamples = AnalogCore.channel[7]; //7
 
    status = CPT_Config(&CPT, ENERGY_METER_SAMPLE_RATE, ENERGY_METER_SAMPLES_PER_CYCLE, 3);

    if(status != 0) {
		NOTIFY_ERROR("Error setting CPT.\n");
		code = -1;
		goto exit_failure_cpt;
	}

	
	//criar os buffers para calcular o valor médio dos atributos a cada 1 Hz
	for(int i = 0; i < NUMBER_OF_MAF_BUFFERS; i++){
		Buffers[i].Data = (double*)malloc(sizeof(double) * ENERGY_METER_FREQ);

		if(Buffers[i].Data == NULL){
			goto exit_failure_mafs;
		}

		Buffers[i].Length = ENERGY_METER_FREQ;
		MovingAverageFilter_Init(&_maf[i], &Buffers[i], 0, 0, 0);
	}


	int socket_desc;
	struct sockaddr_in server;
	
	//Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1){
		printf("Could not create socket");
		goto sock_error;
	}
		
	server.sin_addr.s_addr = inet_addr("192.168.15.82");
	server.sin_family = AF_INET;
	server.sin_port = htons(3333);

	//Connect to remote server
	if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0){
		puts("connect error");
		goto sock_conn_error;
	}


	
    while(stop_daq){
		EventN = AnalogCore_GetSamples(&AnalogCore);

		/*verify if application lost packets*/
		if((EventN - lastEventN) > 1){
			if(lastEventN != 0){
				NOTIFY_ERROR("\nPRU events: perdeu pacote!!!\n");
			}
		}

		MetrologyCore_Scale(&MetrologyCore);
		//MetrologyCore_CalculateHarmonics(&MetrologyCore);
		

		for(int idx = 0, m = 0, n = 0, offset = 0; idx < ENERGY_METER_SAMPLE_RATE; idx++){
			int j;
			struct tPAC pac;
			for(j = 0; j < 3; j++){
				pac.V[j] = MetrologyCore.VChannels[j].fsamples[idx];
				pac.I[j] = MetrologyCore.IChannels[j].fsamples[idx];
			}

			CPT_Update(&CPT, &pac);

			m++;
			if(m >= ENERGY_METER_SAMPLES_PER_CYCLE){
				m = 0;
				update_mean_values(&CPT);
			}

			n++;
			if(n >= RAW_PARAM_FREQ_COUNT){
				n = 0;
				update_raw_values(&CPT, offset);
				offset += PARAMS_LEN;
			}
		}
		
		/*
		SaveWaveform("logs/va",MetrologyCore.VChannels[0].fsamples, ENERGY_METER_SAMPLE_RATE);
		SaveWaveform("logs/vb",MetrologyCore.VChannels[1].fsamples, ENERGY_METER_SAMPLE_RATE);
		SaveWaveform("logs/vc",MetrologyCore.VChannels[2].fsamples, ENERGY_METER_SAMPLE_RATE);
		SaveWaveform("logs/ia",MetrologyCore.IChannels[0].fsamples, ENERGY_METER_SAMPLE_RATE);
		SaveWaveform("logs/ib",MetrologyCore.IChannels[1].fsamples, ENERGY_METER_SAMPLE_RATE);
		SaveWaveform("logs/ic",MetrologyCore.IChannels[2].fsamples, ENERGY_METER_SAMPLE_RATE);
		//stop_daq = 0;
		*/
		
		send_data(socket_desc);

		WriteLog(params, PARAMS_LEN, RAW_PARAM_FREQ);
		
		
		sched_yield();
	}

	

    printf("Exiting.\n");

    code = EXIT_SUCCESS;

sock_conn_error:
	close(socket_desc);
sock_error:
exit_failure_mafs:
exit_failure_cpt:
exit_failure_pru:
    DAQ_Stop();
exit_failure_ram:
    AnalogCore_Deinit();
exit_failure:
	if(code != EXIT_SUCCESS){
		exit(code);
	}

    return(code);
}

int WriteLog(float * ptr_params, int cnt_params, int chunks){
    FILE *file;
	int i;

    file = fopen(LOGFILE, "a");

    if (file == NULL) {
		return -1;
	}
    

	for(i = 0; i < chunks; i++){
    	fwrite(ptr_params+cnt_params*i, sizeof(float), cnt_params, file);
	}

    fclose(file);

    return 0;
}


static void update_mean_values(struct tCPT * CPT){
	for(int i = 0; i < APP_PHASES; i++){	
		int j = phase_order[i];
		int k = i*ATTR_MAX;
		MovingAverageFilter_Update(&_maf[k+ATTR_VOLTAGE_RMS], CPT->Phase[j]->V_rms);
		MovingAverageFilter_Update(&_maf[k+ATTR_CURRENT_RMS], CPT->Phase[j]->I_rms);
		MovingAverageFilter_Update(&_maf[k+ATTR_POWER_APPARENT], CPT->Phase[j]->A);
		MovingAverageFilter_Update(&_maf[k+ATTR_POWER_ACTIVE], CPT->Phase[j]->P_avg);
		MovingAverageFilter_Update(&_maf[k+ATTR_ENERGY_REACTIVE], CPT->Phase[j]->Q);
		MovingAverageFilter_Update(&_maf[k+ATTR_CURRENT_ACTIVE], CPT->Phase[j]->Iba_rms);
		MovingAverageFilter_Update(&_maf[k+ATTR_CURRENT_REACTIVE], CPT->Phase[j]->Ibr_rms);
		MovingAverageFilter_Update(&_maf[k+ATTR_CURRENT_VOID], CPT->Phase[j]->Iv_rms);
		MovingAverageFilter_Update(&_maf[k+ATTR_PF], CPT->Phase[j]->PF);
		MovingAverageFilter_Update(&_maf[k+ATTR_QF], CPT->Phase[j]->QF);
		MovingAverageFilter_Update(&_maf[k+ATTR_VF], CPT->Phase[j]->VF);
	}
}

static void update_raw_values(struct tCPT * CPT, int offset){
	for(int i = 0; i < APP_PHASES; i++){	
		int j = phase_order[i];
		int k = offset + i*RAW_PARAM;
		params[k+ATTR_VOLTAGE_RMS] = CPT->Phase[j]->V_rms;
		params[k+ATTR_CURRENT_RMS] = CPT->Phase[j]->I_rms;
		params[k+ATTR_POWER_APPARENT] = CPT->Phase[j]->A;
		params[k+ATTR_POWER_ACTIVE] = CPT->Phase[j]->P_avg;
		params[k+ATTR_ENERGY_REACTIVE] = CPT->Phase[j]->Q;
		params[k+ATTR_CURRENT_ACTIVE] = CPT->Phase[j]->Iba_rms;
		params[k+ATTR_CURRENT_REACTIVE] = CPT->Phase[j]->Ibr_rms;
		params[k+ATTR_CURRENT_VOID] = CPT->Phase[j]->Iv_rms;
		params[k+ATTR_PF] = CPT->Phase[j]->PF;
		params[k+ATTR_QF] = CPT->Phase[j]->QF;
		params[k+ATTR_VF] = CPT->Phase[j]->VF;
	}
}

static void send_data(int socket_desc){
	static char message[1000];
	static const char * param_name_format[ATTR_MAX] = {
		[ATTR_VOLTAGE_RMS] = "V_rms_%c:%.2f",
		[ATTR_CURRENT_RMS] = "I_rms_%c:%.2f",
		[ATTR_POWER_APPARENT] = "A_%c:%.2f",
		[ATTR_POWER_ACTIVE] = "P_%c:%.2f",
		[ATTR_ENERGY_REACTIVE] = "Q_%c:%.2f",
		[ATTR_CURRENT_ACTIVE] = "Iba_rms_%c:%.2f",
		[ATTR_CURRENT_REACTIVE] = "Ibr_rms_%c:%.2f",
		[ATTR_CURRENT_VOID] = "Iv_rms_%c:%.2f",
		[ATTR_PF] = "PF_%c:%.2f",
		[ATTR_QF] = "QF_%c:%.2f",
		[ATTR_VF] = "VF_%c:%.2f"	
	};

	int offset = sprintf(message, "%s", "$bbb:1,");

	for (int i = 0; i < APP_PHASES; i++){
		for (int j = 0; j < ATTR_MAX; j++){
			int k = phase_order[i];
			offset += sprintf(message+offset, param_name_format[j], phase[k], _maf[j + i*ATTR_MAX].MeanValue);
			message[offset++] = ',';
		}
	}
	if(offset > 0)
		message[offset-1] = 0;

	printf("message[%i] == %s\r\n", offset, message);
	
	if( send(socket_desc , message , strlen(message) , 0) < 0){
		puts("Send failed");
	}
}
