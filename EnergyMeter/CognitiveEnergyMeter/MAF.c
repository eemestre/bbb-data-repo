#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "MAF.h"

void MovingAverageFilter_Init(struct tMAF * me, struct tBuffer * buffer, int threshold, double cmp, double min)
{
	if (me != NULL && buffer != NULL)
	{
		me->Buffer.Data = buffer->Data;
		me->Buffer.Length = buffer->Length;
		me->Accumulator = 0;
		me->MeanValue = 0;
		me->Head = 0;
		me->Threshold_Enabled = threshold;
		me->Threshold_Value = cmp;
		me->Min = min;

		int i;

		for (i = 0; i < me->Buffer.Length; i++)
		{
			me->Buffer.Data[i] = 0;
		}
	}
}

double MovingAverageFilter_Update(struct tMAF * me, double inputData)
{
	if (me != NULL)
	{
		int head = me->Head;
		double lastValue = me->Buffer.Data[head];

		me->Accumulator = me->Accumulator - lastValue;
		me->Accumulator = me->Accumulator + inputData;

		me->Buffer.Data[head] = inputData;

		head = head + 1;

		if (head >= me->Buffer.Length)
		{
			head = 0;
		}

		me->Head = head;

		/*
		if(me->Threshold_Enabled == 1){
			if(me->Accumulator <= me->Threshold_Value){
				me->Accumulator = me->Min;
			}
		}*/

		me->MeanValue = me->Accumulator / (double)me->Buffer.Length;

		if(me->Threshold_Enabled == 1){
			if(me->MeanValue <= me->Threshold_Value){
				me->MeanValue = me->Min;
			}
		}

		return me->MeanValue;
	}

	return 0;
}