#pragma once
#include "./ThostTraderApi/ThostFtdcTraderApi.h"
#include <string.h>
#include <queue>
#include <iostream>
#include "lock.h"
using namespace std;

typedef struct OrderIdentifyKey
{
	TThostFtdcOrderRefType		OrderRef;
	TThostFtdcSessionIDType		SessionID;
	TThostFtdcFrontIDType		FrontID;
	
#if 0
	TThostFtdcExchangeIDType	ExchangeID;
	TThostFtdcOrderSysIDType	OrderSysID;
	TThostFtdcTraderIDType		TraderID;
	TThostFtdcOrderLocalIDType	OrderLocalID;
#endif//0
}OrderIdentifyKey;

typedef pair<int,CThostFtdcOrderField> frozenYdPosition;

typedef pair<int, OrderIdentifyKey> frozenYdPosition2;
typedef deque<frozenYdPosition2> frozenYdPostionList;
typedef deque<frozenYdPosition2>::iterator frozenYdPostionListIter;

class position
{
public:
	position();

	enum {
		POS_OP_ADD,
		POS_OP_DEC,
		POS_OP_ASSIGN
	};

//private:
	// for recording actual positions
	int m_long_position_share;						// total long position
	int m_short_position_share;						// total short position
	int m_overnight_long_position_share;
	int m_overnight_short_position_share;
	int m_today_long_position_share;
	int m_today_short_position_share;

	// for recording available positions which can be closed
	int m_available_long_position_share;			// total avail long position
	int m_available_short_position_share;			// total avail short position
	int m_available_today_long_position_share;
	int m_available_today_short_position_share;
	int m_available_overnight_long_position_share;
	int m_available_overnight_short_position_share;
	void update_avail_today_long_position_share(int volume, int updateType);
	void update_avail_today_short_position_share(int volume, int updateType);
	void update_avail_overnight_long_position_share(int volume, int updateType);
	void update_avail_overnight_short_position_share(int volume, int updateType);
	void update_available_long_position_share();
	void update_available_short_position_share();
	void save_close_ydpos_order(int volume, const OrderIdentifyKey& orderKey);
	
	// for positions' cost
	double m_long_position_cost;					// total long posi cost
	double m_short_position_cost;					// total short posi cost
	double m_today_long_position_cost;
	double m_today_short_position_cost;
	double m_overnight_long_position_cost;
	double m_overnight_short_position_cost;
	void update_today_long_position_cost(double price);
	void update_today_short_position_cost(double price);
	void update_long_position_cost();
	void update_short_position_cost();


	void save_long_postion_detail(int volume, double midPrice);
	void save_short_postion_detail(int volume, double midPrice);

	// for positions' profit
	double m_long_position_value;
	double m_short_position_value;

	double m_long_close_profit;
	double m_short_close_profit;
	double m_long_position_profit;
	double m_short_position_profit;
	double m_totalProfit;
	void update_long_position_profit(TThostFtdcPriceType midPrice);
	void update_short_position_profit(TThostFtdcPriceType midPrice);
	void update_short_close_profit(int volume, TThostFtdcPriceType price);
	void update_long_close_profit(int volume, TThostFtdcPriceType price);
	void update_total_profit();


	struct detailCost{
		double price;
		int volume;
	};
	queue<detailCost> m_costQueueLongPosition;
	queue<detailCost> m_costQueueShortPosition;
	deque<frozenYdPosition> m_frozenYdPos;				// 保存已发出的平仓单记录
	deque<frozenYdPosition2> m_frozenYdPos2;

	double		m_PriceTick;		// the price tick
	int			m_VolumeMultiple;	// the volume tick
	ThreadMutex m_posLock;			// lock while multi threads update positions

private:
	bool find_frozen_ydPosition(int& vol, const CThostFtdcOrderField* pOrder);

public:
	/// freeze available positions
	void freeze_available_long_position_share_today(int volume);
	void freeze_available_short_position_share_today(int volume);
	void freeze_available_long_position_share_overnight(int volume, const OrderIdentifyKey& orderKey);
	void freeze_available_short_position_share_overnight(int volume, const OrderIdentifyKey& orderKey);
	/// free available positions
	void free_available_long_position_share(const CThostFtdcOrderField* pOrder);
	void free_available_short_position_share(const CThostFtdcOrderField* pOrder);

	//void update_position_share(CThostFtdcTradeField* pTrade);
	//void update_position_profit(CThostFtdcTradeField* pTrade);
	void update_position(const CThostFtdcTradeField *pTrade, double midPrice);

public:
	void set_instrument_info_for_position(const CThostFtdcInstrumentField *pInstrument);

	/// get members relevant positions
	int get_long_position_share(){				return m_long_position_share; };
	int get_short_position_share(){				return m_short_position_share; };
	int get_today_long_position_share(){		return m_today_long_position_share; };
	int get_today_short_position_share(){		return m_today_short_position_share; };
	int get_overnight_long_position_share(){	return m_overnight_long_position_share; };
	int get_overnight_short_position_share(){	return m_overnight_short_position_share; };
	int get_available_long_position_share(){	return m_available_long_position_share; };
	int get_available_short_position_share(){	return m_available_short_position_share; };
	int get_available_long_position_share_today(){		return m_available_today_long_position_share; };
	int get_available_short_position_share_today(){		return m_available_today_short_position_share; };
	int get_available_short_position_share_overnight(){	return m_available_overnight_short_position_share; };
	int get_available_long_position_share_overnight(){	return m_available_overnight_long_position_share; }
	
	/// get members relevant profit
	double get_long_position_profit(){	return m_long_position_profit;  }
	double get_short_position_profit(){	return m_short_position_profit; }
	double get_position_profit(){		return m_long_position_profit + m_short_position_profit;}
	double get_long_close_profit(){		return m_long_close_profit;  }
	double get_short_close_profit(){	return m_short_close_profit; }
	double get_close_profit(){			return m_long_close_profit + m_short_close_profit; }
	double get_long_position_cost(){	return m_long_position_cost; }
	double get_short_position_cost(){	return m_short_position_cost;}

	/// get average position cost
    double get_avg_long_position_cost();
	double get_avg_overnight_long_position_cost();
	double get_avg_today_long_position_cost();
	double get_avg_short_position_cost();
	double get_avg_overnight_short_position_cost();
	double get_avg_today_short_position_cost();
};



