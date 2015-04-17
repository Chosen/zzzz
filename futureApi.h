#pragma  once
#include "MdSpi.h"
#include "TraderSpi.h"
#include "order.h"
#include "position.h"
#include "account.h"
#include "lock.h"
#include <math.h>
#include <iostream>
#include <Windows.h>
#include <iomanip>
#include <fstream>
using namespace std;

class futureApi:public quoteBase,tradeBase,public account
{
public:
	futureApi(TThostFtdcInstrumentIDType INSTRUMENT_ID,
		CMdSpi* mdspi,
		CTraderSpi* traderSpi,
		CThostFtdcTradingAccountField* pMyAccount,
		CThostFtdcInstrumentCommissionRateField* pCommssion,
		CThostFtdcInstrumentMarginRateField *pMarginRate,
		CThostFtdcInstrumentField *pInstrument);
	char* m_INSTRUMENT_ID;
	
	// strategy will implement all the pure virtual funcs
	virtual void process_depth_market_Data(CThostFtdcDepthMarketDataField *pDepthMarketData)=0;
	virtual void process_order_execution(CThostFtdcOrderField *pOrder,CThostFtdcTradeField *pTrade)=0;
	virtual void process_order_accepted(CThostFtdcOrderField *pOrder)=0;
	virtual void process_order_canceled(CThostFtdcOrderField *pOrder)=0;
	virtual void process_order_rejected(CThostFtdcOrderField *pOrder)=0;

	// private order requests
	void fill_comm_fields(CThostFtdcInputOrderField* pInputOrder, double price, int qt, char direction, char offset);
	int send_open_buy_order(double price,	int qt);
	int send_open_sell_order(double price,	int qt);
	int send_close_buy_order(double price,	int qt);
	int send_close_sell_order(double price,	int qt);
	int send_open_buy_park(double price,int qt);

	// order requests interface
	int send_buy_order(double price,		int qt);
	int send_sell_order(double price,		int qt);
	int send_buy_fok(double price,			int qt);
	int send_sell_fok(double price,			int qt);

	int send_parked_new_order(double price,int qt);
	int enter_parked_one_order();

	void cancel_one_order(CThostFtdcOrderField *pOrder);

	CTraderSpi* get_traderSpi() { return m_pTraderSpi; }
	CMdSpi* get_mdSpi() { return m_pMdSpi; }
	void print_position();
	void print_order(CThostFtdcOrderField *pOrder);

protected:
	int send_new_order(CThostFtdcInputOrderField* pInputOrder, int qt);
	int enter_one_order(CThostFtdcInputOrderField* pInputOrder);

	virtual void on_quote_change(CThostFtdcDepthMarketDataField *pDepthMarketData);
	virtual void on_order_execution(CThostFtdcTradeField *pTrade);
	virtual void on_rsp_qry_Instrument(CThostFtdcInstrumentField *pInstrument);
	virtual void on_order_accepted(CThostFtdcOrderField *pOrder);
	virtual void on_order_cancel_rejected(CThostFtdcOrderField *pOrder);
	virtual void on_order_rejected(CThostFtdcOrderField *pOrder);	
	virtual void on_order_cancel(CThostFtdcOrderField *pOrder);
	virtual void on_order_submitted(CThostFtdcOrderField *pOrder);
	virtual void on_rsp_qry_instrument_commission_rate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate);
	virtual void on_rsp_qry_instrument_margin_rate(CThostFtdcInstrumentMarginRateField *pInstrumentMarginRate);
	virtual void on_rsp_qry_instrument_trading_account(CThostFtdcTradingAccountField *pTradingAccount);
	virtual void on_rsp_qry_position(CThostFtdcInvestorPositionField *pInvestorPosition);
	virtual void on_rsp_qry_order(CThostFtdcOrderField *pOrder,bool bIsLast);

	void set_instrument_info(CThostFtdcInstrumentField *pInstrument);
	OrderNode* find_order(TThostFtdcOrderRefType orderRef, TThostFtdcFrontIDType FrontID, TThostFtdcSessionIDType SessionID, TThostFtdcExchangeIDType ExchangeID, TThostFtdcOrderSysIDType orderSysID);

protected:
	CTraderSpi*	m_pTraderSpi;
	CMdSpi*		m_pMdSpi;

	config		m_logConfigMd;
	config		m_logConfigTrade;
	bool		m_bConstructed;
	CThostFtdcInstrumentField		m_pInstrument;
	
public:
	CThostFtdcDepthMarketDataField	m_new_quote;
	CThostFtdcDepthMarketDataField	m_last_quote;
	TThostFtdcInstrumentStatusType	m_instrumentStatus;
	int			m_MaxLimitOrderVolume;
	bool		m_bIsLastReqPos;
	bool		m_bIsLastReqOrder;
	static int	m_iInstrument;
	//static bool m_bReqQryAccount;
	Orders		m_orders;		// keep the pending orders
};