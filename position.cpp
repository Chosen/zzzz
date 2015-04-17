#include "position.h"
#include "Utilities.h"
#include "logger.h"

//#define LOCK_AVAIL_POS MutexGuard lk(m_posLock)
#define LOCK_AVAIL_POS

/// atomic update the destVal
static int atomic_update_val(int& destVal, int with, int updateType)
{
	switch (updateType)
	{
	case position::POS_OP_ADD:		return AtomicAdd(&destVal, with);
	case position::POS_OP_DEC:		return AtomicDec(&destVal, with);
	case position::POS_OP_ASSIGN:	return (destVal = with);
	default:
		return destVal;
	}
}

//tested
position::position()
{
	m_long_position_share=0;
	m_short_position_share=0;
	m_overnight_long_position_share=0;
	m_overnight_short_position_share=0;
	m_today_long_position_share=0;
	m_today_short_position_share=0;

	m_available_long_position_share=0;
	m_available_short_position_share=0;
	m_available_today_long_position_share=0;
	m_available_today_short_position_share=0;
	m_available_overnight_long_position_share=0;
	m_available_overnight_short_position_share=0;


	m_long_position_profit=0;
	m_short_position_profit=0;
	m_long_position_cost=0;
	m_short_position_cost=0;
	m_long_position_value=0;
	m_short_position_value=0;
	m_long_close_profit=0;
	m_short_close_profit=0;


	m_today_long_position_cost=0;
	m_today_short_position_cost=0;
	m_overnight_long_position_cost=0;
	m_overnight_short_position_cost=0;
}

void position::update_position(const CThostFtdcTradeField *pTrade,double lastPrice)
{
	int volume = pTrade->Volume;
	double price = pTrade->Price;
	detailCost cost;
	cost.price = price;
	cost.volume= volume;

	if(pTrade->Direction == THOST_FTDC_D_Buy && pTrade->OffsetFlag == THOST_FTDC_OF_Open)  //tested
	{
		m_long_position_share += volume;
		m_today_long_position_share += volume;
		//m_available_long_position_share += volume;
		//m_available_today_long_position_share += volume;
		update_avail_today_long_position_share(volume, position::POS_OP_ADD);
		update_available_long_position_share();

		m_today_long_position_cost += pTrade->Price * volume*m_VolumeMultiple;
		update_long_position_cost();
		update_long_position_profit(lastPrice);
		AsyncLOG(INFO, "update_position: b&o long_pos_cost, today=%.3lf,total=%.3lf", m_today_long_position_cost, m_long_position_cost);
		m_costQueueLongPosition.push(cost);
	}
	else if(pTrade->Direction == THOST_FTDC_D_Sell && pTrade->OffsetFlag == THOST_FTDC_OF_Open) //tested
	{
		m_short_position_share += volume;
		m_today_short_position_share += volume;
		//m_available_short_position_share += volume;
		//m_available_today_short_position_share += volume;
		update_avail_today_short_position_share(volume, position::POS_OP_ADD);
		update_available_short_position_share();
		
		m_today_short_position_cost+=pTrade->Price*volume*m_VolumeMultiple;
		update_short_position_cost();
		update_short_position_profit(lastPrice);
		AsyncLOG(INFO, "update_position: s&o short_pos_cost, today=%.3lf,total=%.3lf", m_today_short_position_cost, m_short_position_cost);
		m_costQueueShortPosition.push(cost);
	}
	else if(pTrade->Direction == THOST_FTDC_D_Buy && (pTrade->OffsetFlag == THOST_FTDC_OF_Close
		|| pTrade->OffsetFlag == THOST_FTDC_OF_CloseToday || pTrade->OffsetFlag == THOST_FTDC_OF_CloseYesterday))
	{
		m_short_position_share -= volume;
		if(volume > m_overnight_short_position_share)//tested
		{
			int tmp = m_overnight_short_position_share;
			m_overnight_short_position_share = 0;
			m_overnight_short_position_cost = 0;
			//m_available_overnight_short_position_share = 0;
			update_avail_overnight_short_position_share(0, position::POS_OP_ASSIGN);
					
			m_today_short_position_cost -= get_avg_today_short_position_cost() * (volume - tmp);
			m_today_short_position_share-= volume - tmp;
		}
		else
		{
			m_overnight_short_position_cost -= get_avg_overnight_short_position_cost() * volume;
			m_overnight_short_position_share-= volume;
		}
		update_short_position_cost();
		update_short_position_profit(lastPrice);
		update_short_close_profit(volume, price);
		AsyncLOG(INFO, "update_position: b&c short_pos_cost, today=%.3lf,total=%.3lf, rPnl=%.3lf\n", m_today_short_position_cost, m_short_position_cost, m_short_close_profit);
	}
	else if(pTrade->Direction == THOST_FTDC_D_Sell && (pTrade->OffsetFlag == THOST_FTDC_OF_Close
		|| pTrade->OffsetFlag == THOST_FTDC_OF_CloseToday || pTrade->OffsetFlag == THOST_FTDC_OF_CloseYesterday))
	{
		m_long_position_share -= volume;
		
		if(volume > m_overnight_long_position_share)//tested
		{
			int tmp = m_overnight_long_position_share;
			m_overnight_long_position_share				= 0;
			m_overnight_long_position_cost				= 0;
			m_available_overnight_long_position_share	= 0;
			
			m_today_long_position_cost	-= get_avg_today_long_position_cost() * (volume - tmp);
			m_today_long_position_share	-= volume - tmp;
		}
		else
		{
			m_overnight_long_position_cost	-= get_avg_overnight_long_position_cost() * volume;
			m_overnight_long_position_share	-= volume;
		}

		update_long_position_cost();
		update_long_position_profit(lastPrice);
		update_long_close_profit(volume, price);
		AsyncLOG(INFO, "update_position: s&c long_pos_cost, today=%.3lf,total=%.3lf, rPnl=%.3lf\n", m_today_long_position_cost, m_long_position_cost, m_long_close_profit);
	}

}

//tested
void position::set_instrument_info_for_position(const CThostFtdcInstrumentField *pInstrument)
{
	m_PriceTick		=pInstrument->PriceTick;
	m_VolumeMultiple=pInstrument->VolumeMultiple;
}

void position::update_avail_today_long_position_share(int volume, int updateType) 
{
	atomic_update_val(m_available_today_long_position_share, volume, updateType);
}

void position::update_avail_today_short_position_share(int volume, int updateType) 
{
	atomic_update_val(m_available_today_short_position_share, volume, updateType);
}

void position::update_avail_overnight_long_position_share(int volume, int updateType) 
{
	atomic_update_val(m_available_overnight_long_position_share, volume, updateType);
}

void position::update_avail_overnight_short_position_share(int volume, int updateType) 
{
	atomic_update_val(m_available_overnight_short_position_share, volume, updateType);
}

void position::update_available_long_position_share()
{
	LOCK_AVAIL_POS;
	m_available_long_position_share = m_available_today_long_position_share + m_available_overnight_long_position_share;
}

void position::update_available_short_position_share()
{
	LOCK_AVAIL_POS;
	m_available_short_position_share = m_available_today_short_position_share + m_available_overnight_short_position_share;
}

//tested
void position::freeze_available_long_position_share_today(int volume)
{
	// 更新多头总仓量和今仓量
	LOCK_AVAIL_POS;
	m_available_long_position_share -= volume;
	m_available_today_long_position_share -= volume;
}

//
void position::freeze_available_short_position_share_today(int volume)
{
	// 更新空头总仓量和今仓量
	LOCK_AVAIL_POS;
	m_available_short_position_share -= volume;
	m_available_today_short_position_share -= volume;
}

void position::freeze_available_short_position_share_overnight(int volume, const OrderIdentifyKey& orderKey)
{
	// 更新空头总仓量和昨仓量
	LOCK_AVAIL_POS;
	m_available_short_position_share -= volume;
	m_available_overnight_short_position_share -= volume;

	m_frozenYdPos2.push_back(frozenYdPosition2(volume, orderKey));
}

void position::freeze_available_long_position_share_overnight(int volume, const OrderIdentifyKey& orderKey)
{
	// 更新多头总仓量和昨仓量
	LOCK_AVAIL_POS;
	m_available_long_position_share -= volume;
	m_available_overnight_long_position_share -= volume;

	m_frozenYdPos2.push_back(frozenYdPosition2(volume, orderKey));
}

//tested
void position::free_available_long_position_share(const CThostFtdcOrderField* pOrder)
{
	LOCK_AVAIL_POS;
	m_available_long_position_share += pOrder->VolumeTotal;   //总剩余的被暂用仓位  
	
	int tmp = 0;
	find_frozen_ydPosition(tmp, pOrder);

	if(pOrder->VolumeTraded <= tmp)//tested
	{
		tmp -= pOrder->VolumeTraded;   //剩余的昨被占用仓位
		m_available_overnight_long_position_share += tmp;
	}
	else  //pOrder->VolumeTraded>tmp tested
	{
		m_available_overnight_long_position_share = 0;
	}

	// 今可用=总可用-昨可用
	m_available_today_long_position_share = m_available_long_position_share - m_available_overnight_long_position_share;

}

//tested
void position::free_available_short_position_share(const CThostFtdcOrderField* pOrder)
{
	LOCK_AVAIL_POS;
	m_available_short_position_share += pOrder->VolumeTotal;   //总剩余的被暂用仓位
	int tmp = 0;
	find_frozen_ydPosition(tmp, pOrder);

	if(pOrder->VolumeTraded <= tmp)
	{
		tmp -= pOrder->VolumeTraded;   //剩余的昨被占用仓位
		m_available_overnight_short_position_share += tmp;		

	}else  //pOrder->VolumeTraded>tmp
	{
		m_available_overnight_short_position_share = 0;
	}

	m_available_today_short_position_share = m_available_short_position_share - m_available_overnight_short_position_share;

}

//partially tested
bool position::find_frozen_ydPosition(int& vol, const CThostFtdcOrderField* pOrder)
{
	deque<frozenYdPosition>::iterator it;
	for(it=m_frozenYdPos.begin();it!=m_frozenYdPos.end();++it)//tested
	{
		if(    pOrder->FrontID==it->second.FrontID
			&& pOrder->SessionID==it->second.SessionID
			&& strcmp(pOrder->OrderRef,it->second.OrderRef)==0)
		{
			vol = it->first;
			return true;
		}
	}

	// deprecated the below searching
	for(it = m_frozenYdPos.begin();it!=m_frozenYdPos.end();++it)
	{
		if(    strcmp(pOrder->ExchangeID,it->second.ExchangeID)==0
			&& strcmp(pOrder->OrderSysID,it->second.OrderSysID)==0)
		{
			vol=it->first;
			return true;
		}
	}

	for(it=m_frozenYdPos.begin();it!=m_frozenYdPos.end();++it)
	{
		if(     strcmp(pOrder->ExchangeID,it->second.ExchangeID)==0
			&& strcmp(pOrder->TraderID,it->second.TraderID)==0
			&&	strcmp(pOrder->OrderLocalID,it->second.OrderLocalID)==0
			)
		{
			vol=it->first;
			return true;
		}
	}
	return false;


}


//tested
void position::update_long_close_profit(int volume,double price)
{
	int residual;
	while(volume > 0 && !m_costQueueLongPosition.empty())
	{
		detailCost& detCost = m_costQueueLongPosition.front();
		residual = detCost.volume - volume;
		if(residual > 0)//tested
		{
			detCost.volume = residual;
			m_long_close_profit += volume*(price- detCost.price)*m_VolumeMultiple;
			volume = 0;
		}else//tested
		{
			volume -= detCost.volume;
			m_long_close_profit += detCost.volume *(price - detCost.price)*m_VolumeMultiple;
			m_costQueueLongPosition.pop();
		}
	}
	update_total_profit();

}

//tested
void position::update_short_close_profit(int volume, double price)
{
	int residual;
	while(volume > 0 && !m_costQueueShortPosition.empty())
	{
		detailCost& detCost = m_costQueueShortPosition.front();
		residual = detCost.volume - volume;
		if(residual>0)
		{
			detCost.volume = residual;
			m_short_close_profit += -volume*(price - detCost.price) * m_VolumeMultiple;
			volume=0;
		}else
		{
			volume -= detCost.volume;
			m_short_close_profit += -detCost.volume	* (price - detCost.price) * m_VolumeMultiple;
			m_costQueueShortPosition.pop();
		}
	}
	update_total_profit();
}

//tested
void position::update_long_position_profit(TThostFtdcPriceType lastPrice)
{
	m_long_position_value	= lastPrice * m_long_position_share * m_VolumeMultiple;
	m_long_position_profit	= m_long_position_value - m_long_position_cost;
	update_total_profit();
	//Log2DebugView("long_pos_profit: val=%.3lf,profit=%.3lf,total=%.3lf\n", m_long_position_value, m_long_position_profit, m_totalProfit);
}

//tested
void position::update_short_position_profit(TThostFtdcPriceType lastPrice)
{
	m_short_position_value	= lastPrice * m_short_position_share * m_VolumeMultiple;
	m_short_position_profit	= m_short_position_cost - m_short_position_value;
	update_total_profit();
	//Log2DebugView("short_pos_profit: val=%.3lf,profit=%.3lf,total=%.3lf\n", m_short_position_value, m_short_position_profit, m_totalProfit);
}

//tested
void position::update_total_profit()
{
	m_totalProfit = m_short_position_profit + m_short_close_profit + m_long_position_profit + m_long_close_profit;
}

//tested
double position::get_avg_long_position_cost()
{
	return m_long_position_cost/(m_long_position_share+1e-20);
}

double position::get_avg_overnight_long_position_cost()
{
	return m_overnight_long_position_cost/(m_overnight_long_position_share+1e-20);
}

double position::get_avg_today_long_position_cost()
{
	return m_today_long_position_cost/(m_today_long_position_share+1e-20);
}

double position::get_avg_short_position_cost()
{
	return m_short_position_cost/(m_short_position_share+1e-20);
}

double position::get_avg_overnight_short_position_cost()
{
	return m_overnight_short_position_cost/(m_overnight_short_position_share+1e-20);
}

double position::get_avg_today_short_position_cost()
{
	return m_today_short_position_cost/(m_today_short_position_share+1e-20);
}



void position::update_today_long_position_cost(double price)
{
	m_today_long_position_cost=m_today_long_position_share*m_VolumeMultiple*price;
	update_long_position_cost();
}

//tested
void position::update_long_position_cost()
{
	m_long_position_cost = m_today_long_position_cost + m_overnight_long_position_cost;
}

void position::update_today_short_position_cost(double price)
{
	m_today_short_position_cost=m_today_short_position_share*m_VolumeMultiple*price;
	update_short_position_cost();
}

void position::update_short_position_cost()
{
	m_short_position_cost=m_today_short_position_cost+m_overnight_short_position_cost;
}


