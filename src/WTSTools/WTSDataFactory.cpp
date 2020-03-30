/*!
 * \file WTSDataFactory.cpp
 * \project	WonderTrader
 *
 * \author Wesley
 * \date 2020/03/30
 * 
 * \brief 
 */
#include "WTSDataFactory.h"
#include "../Share/WTSDataDef.hpp"
#include "../Share/WTSContractInfo.hpp"
#include "../Share/WTSSessionInfo.hpp"
#include "../Share/TimeUtils.hpp"


WTSBarStruct* WTSDataFactory::updateKlineData(WTSKlineData* klineData, WTSTickData* tick, WTSSessionInfo* sInfo)
{
	if(klineData == NULL || tick == NULL)
		return NULL;

	if(strcmp(klineData->code(), tick->code()) != 0)
		return NULL;

	if(sInfo == NULL)
		return NULL;

	if (!sInfo->isInTradingTime(tick->actiontime() / 100000))
		return NULL;

	WTSKlinePeriod period = klineData->period();
	switch( period )
	{
	case KP_Tick:
		return updateSecData(sInfo, klineData, tick);
		break;
	case KP_Minute1:
		return updateMin1Data(sInfo, klineData, tick);
	case KP_Minute5:
		return updateMin5Data(sInfo, klineData, tick);
	case KP_DAY:
		return updateDayData(sInfo, klineData, tick);
	default:
		return NULL;
	}
}

WTSBarStruct* WTSDataFactory::updateKlineData(WTSKlineData* klineData, WTSBarStruct* newBasicBar, WTSSessionInfo* sInfo)
{
	if (klineData == NULL || newBasicBar == NULL)
		return NULL;

	if (sInfo == NULL)
		return NULL;

	WTSKlinePeriod period = klineData->period();
	switch (period)
	{
	case KP_Minute1:
		return updateMin1Data(sInfo, klineData, newBasicBar);
	case KP_Minute5:
		return updateMin5Data(sInfo, klineData, newBasicBar);
	default:
		return NULL;
	}
}

WTSBarStruct* WTSDataFactory::updateMin1Data(WTSSessionInfo* sInfo, WTSKlineData* klineData, WTSBarStruct* newBasicBar)
{
	if (sInfo == NULL)
		return NULL;

	if(klineData->times() == 1)
	{
		klineData->appendBar(*newBasicBar);
		klineData->setClosed(true);
		return klineData->at(-1);
	}

	//����ʱ�䲽��
	uint32_t steplen = klineData->times();

	const WTSBarStruct& curBar = *newBasicBar;

	uint32_t uTradingDate = curBar.date;
	uint32_t uDate = TimeUtils::minBarToDate(curBar.time);
	if (uDate == 19900000)
		uDate = uTradingDate;
	uint32_t uTime = TimeUtils::minBarToTime(curBar.time);
	uint32_t uMinute = sInfo->timeToMinutes(uTime) - 1;

	uint32_t uBarMin = (uMinute / steplen)*steplen + steplen;
	uint32_t uBarTime = sInfo->minuteToTime(uBarMin);
	//if(uBarTime > uTime && !sInfo->isInAuctionTime(uTime))
	//{
	//	//�������ֻ���������ڵ���
	//	uDate = TimeUtils::getNextDate(uDate, -1);
	//}
	uBarTime = TimeUtils::timeToMinBar(uDate, uBarTime);

	WTSBarStruct* lastBar = NULL;
	if (klineData->size() > 0)
	{
		lastBar = klineData->at(klineData->size() - 1);
	}

	bool bNewBar = false;
	if (lastBar == NULL || lastBar->date != uDate || lastBar->time != uBarTime)
	{
		//ֻҪ���ں�ʱ�䶼����������Ϊ�Ѿ���һ���µ�bar��
		lastBar = new WTSBarStruct();
		bNewBar = true;

		memcpy(lastBar, &curBar, sizeof(WTSBarStruct));
		lastBar->date = uDate;
		lastBar->time = uBarTime;
	}
	else
	{
		bNewBar = false;

		lastBar->high = max(lastBar->high, curBar.high);
		lastBar->low = min(lastBar->low, curBar.low);
		lastBar->close = curBar.close;
		lastBar->settle = curBar.settle;

		lastBar->vol += curBar.vol;
		lastBar->money += curBar.money;
		lastBar->add += curBar.add;
		lastBar->hold = curBar.hold;
	}

	if(lastBar->time > curBar.time)
	{
		klineData->setClosed(false);
	}
	else
	{
		klineData->setClosed(true);
	}

	if (bNewBar)
	{
		klineData->appendBar(*lastBar);
		delete lastBar;

		return klineData->at(-1);
	}

	return NULL;
}

WTSBarStruct* WTSDataFactory::updateMin1Data(WTSSessionInfo* sInfo, WTSKlineData* klineData, WTSTickData* tick)
{
	//uint32_t curTime = tick->actiontime()/100000;

	uint32_t steplen = klineData->times();

	uint32_t uDate = tick->actiondate();
	uint32_t uTime = tick->actiontime() / 100000;
	uint32_t uMinute = sInfo->timeToMinutes(uTime);
	if(uMinute == INVALID_UINT32)
	{
		if(tick->volumn() != 0)
		{
			WTSBarStruct *bar = klineData->at(klineData->size()-1);
			bar->close = tick->price();
			bar->high = max(bar->high,tick->price());
			bar->low = min(bar->low,tick->price());
			bar->vol += tick->volumn();
			bar->money += tick->turnover();
			bar->hold = tick->openinterest();
			bar->add += tick->additional();
		}

		return NULL;
	}
	if (sInfo->isLastOfSection(uTime))
	{
		uMinute--;
	}
	uint32_t uBarMin = (uMinute/steplen)*steplen + steplen;
	uint32_t uOnlyMin = sInfo->minuteToTime(uBarMin);
	if(uOnlyMin == 0)
	{
		uDate = TimeUtils::getNextDate(uDate);
	}
	uint32_t uBarTime = TimeUtils::timeToMinBar(uDate, uOnlyMin);

	uint32_t lastTime = klineData->time(-1);
	uint32_t lastDate = klineData->date(-1);
	if (lastTime == INVALID_UINT32 || uBarTime > lastTime || tick->tradingdate() > lastDate)
	{
		//���ʱ�䲻һ�£�������һ��K��
		WTSBarStruct *day = new WTSBarStruct;
		day->date = tick->tradingdate();
		day->time = uBarTime;
		day->open = tick->price();
		day->high = tick->price();
		day->low = tick->price();
		day->close = tick->price();
		day->vol = tick->volumn();
		day->money = tick->turnover();
		day->hold = tick->openinterest();
		day->add = tick->additional();

		//klineData->getDataRef().push_back(day);

		return day;
	}
	else if (lastTime != INVALID_UINT32 && uBarTime < lastTime)
	{
		//���������ҪΪ�˷�ֹ���ڷ�������
		return NULL;
	}
	else
	{
		WTSBarStruct *bar = klineData->at(klineData->size()-1);
		bar->close = tick->price();
		bar->high = max(bar->high,tick->price());
		bar->low = min(bar->low,tick->price());
		bar->vol += tick->volumn();
		bar->money += tick->turnover();
		bar->hold = tick->openinterest();
		bar->add += tick->additional();

		return NULL;
	}
}

WTSBarStruct* WTSDataFactory::updateMin5Data(WTSSessionInfo* sInfo, WTSKlineData* klineData, WTSBarStruct* newBasicBar)
{
	if (sInfo == NULL)
		return NULL;

	if (klineData->times() == 1)
	{
		klineData->appendBar(*newBasicBar);
		return klineData->at(-1);
	}

	//����ʱ�䲽��
	uint32_t steplen = 5 * klineData->times();

	const WTSBarStruct& curBar = *newBasicBar;

	uint32_t uTradingDate = curBar.date;
	uint32_t uDate = TimeUtils::minBarToDate(curBar.time);
	if (uDate == 19900000)
		uDate = uTradingDate;
	uint32_t uTime = TimeUtils::minBarToTime(curBar.time);
	uint32_t uMinute = sInfo->timeToMinutes(uTime) - 5;


	uint32_t uBarMin = (uMinute / steplen)*steplen + steplen;
	uint32_t uBarTime = sInfo->minuteToTime(uBarMin);
	uBarTime = TimeUtils::timeToMinBar(uDate, uBarTime);

	WTSBarStruct* lastBar = NULL;
	if (klineData->size() > 0)
	{
		lastBar = klineData->at(klineData->size() - 1);
	}

	bool bNewBar = false;
	if (lastBar == NULL || lastBar->date != uDate || lastBar->time != uBarTime)
	{

		//ֻҪ���ں�ʱ�䶼����������Ϊ�Ѿ���һ���µ�bar��
		lastBar = new WTSBarStruct();
		bNewBar = true;

		memcpy(lastBar, &curBar, sizeof(WTSBarStruct));
		lastBar->date = uTradingDate;
		lastBar->time = uBarTime;
	}
	else
	{
		bNewBar = false;

		lastBar->high = max(lastBar->high, curBar.high);
		lastBar->low = min(lastBar->low, curBar.low);
		lastBar->close = curBar.close;
		lastBar->settle = curBar.settle;

		lastBar->vol += curBar.vol;
		lastBar->money += curBar.money;
		lastBar->add += curBar.add;
		lastBar->hold = curBar.hold;
	}

	if (lastBar->time > curBar.time)
	{
		klineData->setClosed(false);
	}
	else
	{
		klineData->setClosed(true);
	}

	if (bNewBar)
	{
		klineData->appendBar(*lastBar);
		delete lastBar;

		return klineData->at(-1);
	}

	return NULL;
}

WTSBarStruct* WTSDataFactory::updateMin5Data(WTSSessionInfo* sInfo, WTSKlineData* klineData, WTSTickData* tick)
{
	uint32_t steplen = 5*klineData->times();

	uint32_t uDate = tick->actiondate();
	uint32_t uTime = tick->actiontime()/100000;
	uint32_t uMinute = sInfo->timeToMinutes(uTime);
	if (sInfo->isLastOfSection(uTime))
	{
		uMinute--;
	}
	uint32_t uBarMin = (uMinute/steplen)*steplen + steplen;
	uint32_t uOnlyMin = sInfo->minuteToTime(uBarMin);
	if (uOnlyMin == 0)
	{
		uDate = TimeUtils::getNextDate(uDate);
	}
	uint32_t uBarTime = TimeUtils::timeToMinBar(uDate, uOnlyMin);

	uint32_t lastTime = klineData->time(klineData->size()-1);
	if(lastTime == INVALID_UINT32 || uBarTime != lastTime)
	{
		//���ʱ�䲻һ�£�������һ��K��
		WTSBarStruct *day = new WTSBarStruct;
		day->date = tick->tradingdate();
		day->time = uBarTime;
		day->open = tick->price();
		day->high = tick->price();
		day->low = tick->price();
		day->close = tick->price();
		day->vol = tick->volumn();
		day->money = tick->turnover();
		day->hold = tick->openinterest();
		day->add = tick->additional();

		//klineData->getDataRef().push_back(day);

		return day;
	}
	else
	{
		WTSBarStruct *bar = klineData->at(klineData->size()-1);
		bar->close = tick->price();
		bar->high = max(bar->high,tick->price());
		bar->low = min(bar->low,tick->price());
		bar->vol += tick->volumn();
		bar->money += tick->turnover();
		bar->hold = tick->openinterest();
		bar->add = tick->additional();

		return NULL;
	}
}

WTSBarStruct* WTSDataFactory::updateDayData(WTSSessionInfo* sInfo, WTSKlineData* klineData, WTSTickData* tick)
{
	uint32_t curDate = tick->tradingdate();
	uint32_t lastDate = klineData->date(klineData->size()-1);

	if(lastDate == INVALID_UINT32 || curDate != lastDate)
	{
		//���ʱ�䲻һ�£�������һ��K��
		WTSBarStruct *day = new WTSBarStruct;
		day->date = curDate;
		day->time = 0;
		day->open = tick->price();
		day->high = tick->price();
		day->low = tick->price();
		day->close = tick->price();
		day->vol = tick->volumn();
		day->money = tick->turnover();
		day->hold = tick->openinterest();
		day->add = tick->additional();

		//klineData->getDataRef().push_back(day);

		return day;
	}
	else
	{
		WTSBarStruct *bar = klineData->at(klineData->size()-1);
		bar->close = tick->price();
		bar->high = max(bar->high,tick->price());
		bar->low = min(bar->low,tick->price());
		bar->vol += tick->volumn();
		bar->money += tick->turnover();
		bar->hold = tick->openinterest();
		bar->add += tick->additional();

		return NULL;
	}
}

WTSBarStruct* WTSDataFactory::updateSecData(WTSSessionInfo* sInfo, WTSKlineData* klineData, WTSTickData* tick)
{
	uint32_t seconds = klineData->times();
	uint32_t curSeconds = sInfo->timeToSeconds(tick->actiontime()/1000);
	uint32_t barSeconds = (curSeconds/seconds)*seconds + seconds;
	uint32_t barTime = sInfo->secondsToTime(barSeconds);

	if(klineData->isUnixTime())
	{
		uint32_t uDate = tick->actiondate();
		if (barTime < tick->actiontime() / 1000)
			uDate = TimeUtils::getNextDate(uDate);
		barTime = (uint32_t)(TimeUtils::makeTime(uDate, barTime * 1000) / 1000);
	}	

	uint32_t lastTime = klineData->time(klineData->size()-1);
	if(lastTime == INVALID_UINT32 || lastTime != barTime)
	{
		WTSBarStruct *day = new WTSBarStruct;
		day->date = tick->tradingdate();
		day->time = barTime;
		day->open = tick->price();
		day->high = tick->price();
		day->low = tick->price();
		day->close = tick->price();
		day->vol = tick->volumn();
		day->money = tick->turnover();
		day->hold = tick->openinterest();
		day->add = tick->additional();

		return day;
	}
	else
	{
		WTSBarStruct *bar = klineData->at(klineData->size()-1);
		bar->close = tick->price();
		bar->high = max(bar->high,tick->price());
		bar->low = min(bar->low,tick->price());
		bar->vol += tick->volumn();
		bar->money += tick->turnover();
		bar->hold = tick->openinterest();
		bar->add += tick->additional();

		return NULL;
	}
}

uint32_t WTSDataFactory::getPrevMinute(uint32_t curMinute, int period /* = 1 */)
{
	uint32_t h = curMinute/100;
	uint32_t m = curMinute%100;
	if(m == 0)
	{
		m = 60;
		if(h == 0) h = 24;

		return (h-1)*100 + (m-period);
	}
	else
	{
		return h*100 + m - period;
	}
}

WTSKlineData* WTSDataFactory::extractKlineData(WTSKlineData* baseKline, WTSKlinePeriod period, uint32_t times, WTSSessionInfo* sInfo, bool bIncludeOpen /* = true */)
{
	if(baseKline == NULL || baseKline->size() == 0)
		return NULL;

	//һ��������Ҫת��
	if(times <= 1 || period == KP_Tick)
	{
		baseKline->retain();
		return baseKline;
	}

	if(period == KP_DAY)
	{
		return extractDayData(baseKline, times, bIncludeOpen);
	}
	else if(period == KP_Minute1)
	{
		return extractMin1Data(baseKline, times, sInfo, bIncludeOpen);
	}
	else if(period == KP_Minute5)
	{
		return extractMin5Data(baseKline, times, sInfo, bIncludeOpen);
	}
	
	return NULL;
}

WTSKlineData* WTSDataFactory::extractMin1Data(WTSKlineData* baseKline, uint32_t times, WTSSessionInfo* sInfo, bool bIncludeOpen /* = true */)
{
	//���ݺ�Լ�����ȡ�г���Ϣ
	if(sInfo == NULL)
		return NULL;

	//����ʱ�䲽��
	uint32_t steplen = times;

	WTSKlineData* ret = WTSKlineData::create(baseKline->code(), 0);
	ret->setPeriod(KP_Minute1, times);

	int count = 0;
	WTSKlineData::WTSBarList& bars = baseKline->getDataRef();
	WTSKlineData::WTSBarList::const_iterator it = bars.begin();
	for(; it != bars.end(); it++,count++)
	{
		const WTSBarStruct& curBar = *it;

		uint32_t uTradingDate = curBar.date;
		uint32_t uDate = TimeUtils::minBarToDate(curBar.time);
		if(uDate == 19900000)
			uDate = uTradingDate;
		uint32_t uTime = TimeUtils::minBarToTime(curBar.time);
		uint32_t uMinute = sInfo->timeToMinutes(uTime)-1;

		uint32_t uBarMin = (uMinute/steplen)*steplen + steplen;
		uint32_t uBarTime = sInfo->minuteToTime(uBarMin);
		//if(uBarTime > uTime && !sInfo->isInAuctionTime(uTime))
		//{
		//	//�������ֻ���������ڵ���
		//	uDate = TimeUtils::getNextDate(uDate, -1);
		//}
		uBarTime = TimeUtils::timeToMinBar(uDate, uBarTime);

		WTSBarStruct* lastBar = NULL;
		if(ret->size() > 0)
		{
			lastBar = ret->at(ret->size()-1);
		}

		bool bNewBar = false;
		if(lastBar == NULL || lastBar->date != uDate || lastBar->time != uBarTime)
		{
			//if(lastBar)
			//{
			//	lastBar->time = sInfo->originalTime(lastBar->time);
			//}

			//ֻҪ���ں�ʱ�䶼����������Ϊ�Ѿ���һ���µ�bar��
			lastBar = new WTSBarStruct();
			bNewBar = true;

			memcpy(lastBar, &curBar, sizeof(WTSBarStruct));
			lastBar->date = uDate;
			lastBar->time = uBarTime;
		}
		else
		{
			bNewBar = false;

			lastBar->high = max(lastBar->high, curBar.high);
			lastBar->low = min(lastBar->low, curBar.low);
			lastBar->close = curBar.close;
			lastBar->settle = curBar.settle;

			lastBar->vol += curBar.vol;
			lastBar->money += curBar.money;
			lastBar->add += curBar.add;
			lastBar->hold = curBar.hold;
		}

		if(bNewBar)
		{
			ret->appendBar(*lastBar);
			delete lastBar;
		}
	}

	//������һ������
	{
		WTSBarStruct* lastRawBar = baseKline->at(-1);
		WTSBarStruct* lastDesBar = ret->at(-1);
		//���Ŀ��K�ߵ����һ�����ݵ����ڻ���ʱ�����ԭʼK�����һ�������ڻ�ʱ��
		if ( lastDesBar->date > lastRawBar->date || lastDesBar->time > lastRawBar->time)
		{
			if (!bIncludeOpen)
				ret->getDataRef().resize(ret->size() - 1);
			else
				ret->setClosed(false);
		}
	}
	

	return ret;
}

WTSKlineData* WTSDataFactory::extractMin5Data(WTSKlineData* baseKline, uint32_t times, WTSSessionInfo* sInfo, bool bIncludeOpen /* = true */)
{
	if(sInfo == NULL)
		return NULL;

	//����ʱ�䲽��
	uint32_t steplen = 5*times;

	WTSKlineData* ret = WTSKlineData::create(baseKline->code(), 0);
	ret->setPeriod(KP_Minute5, times);

	WTSKlineData::WTSBarList& bars = baseKline->getDataRef();
	WTSKlineData::WTSBarList::const_iterator it = bars.begin();
	for(; it != bars.end(); it++)
	{
		const WTSBarStruct& curBar = *it;

		uint32_t uTradingDate = curBar.date;
		uint32_t uDate = TimeUtils::minBarToDate(curBar.time);
		if(uDate == 19900000)
			uDate = uTradingDate;
		uint32_t uTime = TimeUtils::minBarToTime(curBar.time);
		uint32_t uMinute = sInfo->timeToMinutes(uTime)-5;


		uint32_t uBarMin = (uMinute/steplen)*steplen+steplen;
		uint32_t uBarTime = sInfo->minuteToTime(uBarMin);
		uBarTime = TimeUtils::timeToMinBar(uDate, uBarTime);

		WTSBarStruct* lastBar = NULL;
		if(ret->size() > 0)
		{
			lastBar = ret->at(ret->size()-1);
		}

		bool bNewBar = false;
		if(lastBar == NULL || lastBar->date != uDate || lastBar->time != uBarTime)
		{
			//if(lastBar)
			//{
			//	lastBar->time = sInfo->originalTime(lastBar->time);
			//}

			//ֻҪ���ں�ʱ�䶼����������Ϊ�Ѿ���һ���µ�bar��
			lastBar = new WTSBarStruct();
			bNewBar = true;

			memcpy(lastBar, &curBar, sizeof(WTSBarStruct));
			lastBar->date = uTradingDate;
			lastBar->time = uBarTime;
		}
		else
		{
			bNewBar = false;

			lastBar->high = max(lastBar->high, curBar.high);
			lastBar->low = min(lastBar->low, curBar.low);
			lastBar->close = curBar.close;
			lastBar->settle = curBar.settle;

			lastBar->vol += curBar.vol;
			lastBar->money += curBar.money;
			lastBar->add += curBar.add;
			lastBar->hold = curBar.hold;
		}

		if(bNewBar)
		{
			ret->appendBar(*lastBar);
			delete lastBar;
		}
	}

	//������һ������
	{
		WTSBarStruct* lastRawBar = baseKline->at(-1);
		WTSBarStruct* lastDesBar = ret->at(-1);
		//���Ŀ��K�ߵ����һ�����ݵ����ڻ���ʱ�����ԭʼK�����һ�������ڻ�ʱ��
		if (lastDesBar->date > lastRawBar->date || lastDesBar->time > lastRawBar->time)
		{
			if (!bIncludeOpen)
				ret->getDataRef().resize(ret->size() - 1);
			else
				ret->setClosed(false);
		}
	}

	return ret;
}

WTSKlineData* WTSDataFactory::extractDayData(WTSKlineData* baseKline, uint32_t times, bool bIncludeOpen /* = true */)
{
	//����ʱ�䲽��
	uint32_t steplen = times;

	WTSKlineData* ret = WTSKlineData::create(baseKline->code(), 0);
	ret->setPeriod(KP_DAY, times);

	uint32_t count = 0;
	WTSKlineData::WTSBarList& bars = baseKline->getDataRef();
	WTSKlineData::WTSBarList::const_iterator it = bars.begin();
	for(; it != bars.end(); it++,count++)
	{
		const WTSBarStruct& curBar = *it;

		uint32_t uDate = curBar.date;

		WTSBarStruct* lastBar = NULL;
		if(ret->size() > 0)
		{
			lastBar = ret->at(ret->size()-1);
		}

		bool bNewBar = false;
		if(lastBar == NULL || count == steplen)
		{
			//ֻҪ���ں�ʱ�䶼����������Ϊ�Ѿ���һ���µ�bar��
			lastBar = new WTSBarStruct();
			bNewBar = true;

			memcpy(lastBar, &curBar, sizeof(WTSBarStruct));
			lastBar->date = uDate;
			lastBar->time = 0;
			count = 0;
		}
		else
		{
			bNewBar = false;

			lastBar->high = max(lastBar->high, curBar.high);
			lastBar->low = min(lastBar->low, curBar.low);
			lastBar->close = curBar.close;
			lastBar->settle = curBar.settle;

			lastBar->vol += curBar.vol;
			lastBar->money += curBar.money;
			lastBar->add = curBar.add;
			lastBar->hold = curBar.hold;
		}

		if(bNewBar)
		{
			ret->appendBar(*lastBar);
			delete lastBar;
		}
	}

	return ret;
}

WTSKlineData* WTSDataFactory::extractKlineData(WTSArray* ayTicks, uint32_t seconds, WTSSessionInfo* sInfo, bool bUnixTime /* = false */)
{
	if(ayTicks == NULL || ayTicks->size() == 0)
		return NULL;
	
	WTSTickData* firstTick = STATIC_CONVERT(ayTicks->at(0), WTSTickData*);

	if(sInfo == NULL)
		return NULL;

	WTSKlineData* ret = WTSKlineData::create(firstTick->code(),0);
	ret->setPeriod(KP_Tick, seconds);
	ret->setUnixTime(bUnixTime);

	WTSArray::Iterator it = ayTicks->begin();
	for(; it != ayTicks->end(); it++)
	{
		WTSBarStruct* lastBar = NULL;
		if(ret->size() > 0)
		{
			lastBar = ret->at(ret->size()-1);
		}

		WTSTickData* curTick = STATIC_CONVERT(*it, WTSTickData*);
		uint32_t uDate = curTick->tradingdate();
		uint32_t curSeconds = sInfo->timeToSeconds(curTick->actiontime()/1000);
		uint32_t barSeconds = (curSeconds/seconds)*seconds + seconds;
		uint32_t barTime = sInfo->secondsToTime(barSeconds);

		//������������K��ʱ���С��tick���ݵ�ʱ���
		if(bUnixTime)
		{
			uint32_t actDt = curTick->actiondate();
			if (barTime < curTick->actiontime() / 1000)
			{
				actDt = TimeUtils::getNextDate(actDt);
			}
			barTime = (uint32_t)(TimeUtils::makeTime(actDt, barTime * 1000) / 1000);
		}

		bool bNewBar = false;
		if (lastBar == NULL || uDate != lastBar->date || barTime != lastBar->time)
		{
			lastBar = new WTSBarStruct();
			bNewBar = true;

			lastBar->date = uDate;
			lastBar->time = barTime;

			lastBar->open = curTick->price();
			lastBar->high = curTick->price();
			lastBar->low = curTick->price();
			lastBar->close = curTick->price();
			lastBar->vol = curTick->volumn();
			lastBar->money = curTick->turnover();
			lastBar->hold = curTick->openinterest();
			lastBar->add = curTick->additional();
		}
		else
		{
			lastBar->close = curTick->price();
			lastBar->high = max(lastBar->high,curTick->price());
			lastBar->low = min(lastBar->low,curTick->price());
			lastBar->vol += curTick->volumn();
			lastBar->money += curTick->turnover();
			lastBar->hold = curTick->openinterest();
			lastBar->add += curTick->additional();
		}

		if(bNewBar)
		{
			ret->appendBar(*lastBar);
			delete lastBar;
		}
	}

	return ret;
}

bool WTSDataFactory::mergeKlineData(WTSKlineData* klineData, WTSKlineData* newKline)
{
	if (klineData == NULL || newKline == NULL)
		return false;

	if (strcmp(klineData->code(), newKline->code()) != 0)
		return false;

	if (!(klineData->period() == newKline->period() && klineData->times() == newKline->times()))
		return false;

	WTSKlineData::WTSBarList& bars = klineData->getDataRef();
	WTSKlineData::WTSBarList& newBars = newKline->getDataRef();
	if(bars.empty())
	{
		bars.swap(newBars);
		newBars.clear();
		return true;
	}
	else
	{
		uint32_t sTime, eTime;
		if(klineData->period() == KP_DAY)
		{
			sTime = bars[0].date;
			eTime = bars[bars.size() - 1].date;
		}
		else
		{
			sTime = bars[0].time;
			eTime = bars[bars.size() - 1].time;
		}

		WTSKlineData::WTSBarList tempHead, tempTail;
		uint32_t count = newKline->size();
		for (uint32_t i = 0; i < count; i++)
		{
			WTSBarStruct& curBar = newBars[i];

			uint32_t curTime;
			if (klineData->period() == KP_DAY)
				curTime = curBar.date;
			else
				curTime = curBar.time;

			if(curTime < sTime)
			{
				tempHead.push_back(curBar);
			}
			else if(curTime > eTime)
			{
				tempTail.push_back(curBar);
			}
		}

		bars.insert(bars.begin(), tempHead.begin(), tempHead.end());
		bars.insert(bars.end(), tempTail.begin(), tempTail.end());
	}
	
	return true;
}