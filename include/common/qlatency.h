/********************************************************************************************
**
** Copyright (C) 2010-2015 Terry Niu (Beijing, China)
** Filename:	qlatency.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2015/07/03
**
*********************************************************************************************/

#ifndef __QLATENCY_H_
#define __QLATENCY_H_

#include "qglobal.h"
#include "qalgorithm.h"

Q_BEGIN_NAMESPACE

#define DEFAULT_LATENCY_EVENTID     (-1)
#define DEFAULT_LATENCY_MAX         (60*60*1000)
#define DEFAULT_LATENCY_MAX_SAMPLES (1048576)
#define DEFAULT_LATENCY_REPORT      ("__latency.rep")
#define DEFAULT_LATENCY_FSIZE       (1<<8)
#define DEFAULT_LATENCY_STAT_COLUMN (16)

class QLatency {
	public:
		inline QLatency() :
			average(0),
			samplesMax(DEFAULT_LATENCY_MAX_SAMPLES),
			samples(0),
			period(0),
			eventid_array(NULL),
			latency_array(NULL)
		{
			memset(latencyStat, 0, sizeof(uint32_t)*DEFAULT_LATENCY_STAT_COLUMN);
		}

		virtual ~QLatency()
		{
			if(eventid_array)
				q_delete_array<uint64_t>(eventid_array);

			if(latency_array)
				q_delete_array<uint32_t>(latency_array);
		}

		int32_t init(int32_t samplesMax=DEFAULT_LATENCY_MAX_SAMPLES, const char* latencyReport=DEFAULT_LATENCY_REPORT)
		{
			if(samplesMax<=0||latencyReport==NULL||*latencyReport=='\0')
				return -1;

			this->average=0;
			this->samplesMax=samplesMax;
			this->samples=0;
			this->period=0;

			eventid_array=q_new_array<uint64_t>(this->samplesMax);
			if(eventid_array==NULL)
				return -2;

			latency_array=q_new_array<uint32_t>(this->samplesMax);
			if(latency_array==NULL) {
				q_delete_array<uint64_t>(eventid_array);
				return -3;
			}

			strcpy(this->latencyReport, latencyReport);

			return 0;
		}

		int32_t addLatencySample(uint64_t event_id, uint32_t latency)
		{
			if(samples+1>=samplesMax)
				return -1;

			latencyMutex.lock();

			eventid_array[samples]=event_id;
			latency_array[samples]=latency;

			++samples;

			period+=latency;
			average=(samples==0)?0:period/samples;

			if(latency/1000>DEFAULT_LATENCY_STAT_COLUMN-1) {
				++latencyStat[DEFAULT_LATENCY_STAT_COLUMN-1];
			} else {
				++latencyStat[latency/1000];
			}

			latencyMutex.unlock();

			return 0;
		}

		int32_t rearrangeLatency()
		{
			Q_Unrecursion_1K_1P<uint32_t, uint64_t>(0, samples-1, latency_array, eventid_array);
			return 0;
		}

		int32_t createLatencyReport()
		{
			FILE* fp=fopen(latencyReport, "w");
			if(fp==NULL)
				return -1;

			latencyMutex.lock();

			fprintf(fp, "Latency Statistics:\n");
			fprintf(fp, "\n");

			fprintf(fp, "Samples num:          %u\n", samples);
			fprintf(fp, "Total consumed:       %ums\n", period);
			fprintf(fp, "Average latency:      %ums\n", average);
			fprintf(fp, "\n");

			fprintf(fp, "Sorted result:\n");
			for(int32_t i=0; i<samples; ++i)
				fprintf(fp, "EventID %llu: %ums\n", eventid_array[i], latency_array[i]);
			fprintf(fp, "\n");

			fprintf(fp, "Peak event id:        %llu\n", eventid_array[samples-1]);
			fprintf(fp, "Peak event latency:   %ums\n", latency_array[samples-1]);
			fprintf(fp, "Valley event id:      %llu\n", eventid_array[0]);
			fprintf(fp, "Valley event latency: %ums\n", latency_array[0]);
			fprintf(fp, "\n");

			fprintf(fp, "Latency stat(ms):\n");
			for(int32_t i=0; i<DEFAULT_LATENCY_STAT_COLUMN; ++i)
				fprintf(fp, "%05d - %05d(ms): %d\n", i*1000, (i+1)*1000-1, latencyStat[i]);
			fprintf(fp, "\n");

			latencyMutex.unlock();

			return 0;
		}

		int32_t reset()
		{
			latencyMutex.lock();

			average=0;
			samples=0;
			period=0;

			memset(latencyStat, 0, sizeof(uint32_t)*DEFAULT_LATENCY_STAT_COLUMN);

			latencyMutex.unlock();

			return 0;
		}

	protected:
		/* Latency statistics parameters */
		uint32_t average;		/* Average of curret samples */
		uint32_t samplesMax;		/* Max samples */
		uint32_t samples;		/* Number of samples */
		uint32_t period;		/* Number of seconds since first event and now */

		/* Latency Samples */
		uint64_t* eventid_array;
		uint32_t* latency_array;

		/* Latency resport */
		char latencyReport[DEFAULT_LATENCY_FSIZE];

		/* Latency statistics */
		uint32_t latencyStat[DEFAULT_LATENCY_STAT_COLUMN];
		QMutexLock latencyMutex;
};

Q_END_NAMESPACE

#endif // __QLATENCY_H_
