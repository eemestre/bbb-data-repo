#ifndef MAF_H_
#define MAF_H_

#ifdef __cplusplus
extern "C" {
#endif

struct tBuffer
{
	double * Data;	/*Pointer to buffer to operate on*/
	int Length;		/*Size of buffer*/
};

struct tMAF
{
	double MeanValue;	/*Result of the last update*/
	double Accumulator;	/*Accumulator*/
	int Head;			/*Head index - The incoming bytes get written to this index*/
	int Threshold_Enabled;
	double Threshold_Value;
	double Min;
	struct tBuffer Buffer; /*Pointer to buffer to operate on*/
};

double MovingAverageFilter_Update(struct tMAF * me, double inputData);
void MovingAverageFilter_Init(struct tMAF * me, struct tBuffer * buffer, int threshold, double cmp, double min);

#ifdef __cplusplus
}
#endif

#endif /* MAF_H_ */