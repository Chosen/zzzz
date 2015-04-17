#pragma once
#include "futureApi.h"
#include <iostream>
#include <fstream>
using namespace std;
class strategy:public futureApi
{
public:
	strategy::strategy(
		TThostFtdcInstrumentIDType INSTRUMENT_ID, 
		CMdSpi* pMdSPi,
		CTraderSpi* pTraderSPi,
		CThostFtdcTradingAccountField* pMyAccount,
		CThostFtdcInstrumentCommissionRateField* pCommssion,
		CThostFtdcInstrumentMarginRateField *pMarginRate,
		CThostFtdcInstrumentField *pInstrument
		);
	void update_time(TThostFtdcTimeType UpdateTime);
	void calculate_prob();
	void get_required_sell_vol(int &sellVol,int &buyVolAtBid);
	void get_required_buy_vol(int &buyVol,int &sellVolAtAsk);
	void get_orgsell_vol_above_traded_price(int& sellVolAbove,double tradedPrice);
	void get_orgbuy_vol_below_traded_price(int& buyVolBelow,double tradedPrice);
	void get_sell_vol_above_traded_price(int& sellVolAbove,double tradedPrice);
	void get_buy_vol_below_traded_price(int& buyVolBelow,double tradedPrice);

	CThostFtdcOrderField get_lowest_sell_order();
	CThostFtdcOrderField get_highest_buy_order();
	void cancel_sell_order_above_bid_price();
	void cancel_buy_order_below_ask_price();
	int get_insert_hour(TThostFtdcTimeType insertTime);
	void get_buy_vol_at_bid(int& buyVolAtBid);
	void get_sell_vol_at_ask(int& sellVolAtAsk);
	void get_buy_vol_below_bid(int& buyVolAtBid);
	void get_sell_vol_above_ask(int& sellVolAboveAsk);
protected:
	virtual void process_depth_market_Data(CThostFtdcDepthMarketDataField *pDepthMarketData);
	virtual void process_order_execution(CThostFtdcOrderField *pOrder,CThostFtdcTradeField *pTrade);
	virtual void process_order_accepted(CThostFtdcOrderField *pOrder);
	virtual void process_order_canceled(CThostFtdcOrderField *pOrder);
	virtual void process_order_rejected(CThostFtdcOrderField *pOrder);
private:
	fstream recordInfo;
	int m_netPos;
	double m_lastTotalProfit;
	string m_quoteTime;
	int m_HH;
	int m_MM;
	int m_SS;
	double m_probUp;
	double m_probDn;
	double m_rho;
	double m_mot;
	int m_minDepth;
	int m_iFOK;
	int m_iOrder;
	SYSTEMTIME m_startTime;
	SYSTEMTIME m_endTime;
	int m_totalOpen;
	int m_dist;
};