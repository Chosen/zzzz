#pragma once
#include ".\ThostTraderApi\ThostFtdcTraderApi.h"
#include <iostream>
#include <vector>
using namespace std;
struct config
{
	char FRONT_ADDR[1024];
	TThostFtdcBrokerIDType Broker_ID;
	TThostFtdcInvestorIDType InvesterID;
	TThostFtdcPasswordType PASSWORD;
	//TThostFtdcInstrumentIDType InstrumentID;
};

class tradeBase
{
public:
	virtual ~tradeBase(){}
	virtual void on_order_execution(CThostFtdcTradeField *pTrade)=0;
	virtual void on_rsp_qry_Instrument(CThostFtdcInstrumentField *pInstrument)=0;
	virtual void on_order_accepted(CThostFtdcOrderField *pOrder)=0;
	virtual void on_order_rejected(CThostFtdcOrderField *pOrder)=0;
	virtual void on_order_cancel_rejected(CThostFtdcOrderField *pOrder)=0;
	virtual void on_order_cancel(CThostFtdcOrderField *pOrder)=0;
	virtual void on_order_submitted(CThostFtdcOrderField *pOrder)=0;
	virtual void on_rsp_qry_instrument_margin_rate(CThostFtdcInstrumentMarginRateField *pInstrumentMarginRate)=0;
	virtual void on_rsp_qry_instrument_commission_rate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate)=0;
	virtual void on_rsp_qry_instrument_trading_account(CThostFtdcTradingAccountField *pTradingAccount)=0;
	virtual void on_rsp_qry_position(CThostFtdcInvestorPositionField *pInvestorPosition)=0;
	virtual void on_rsp_qry_order(CThostFtdcOrderField *pOrderr,bool bIsLast)=0;
};


class CTraderSpi : public CThostFtdcTraderSpi
{
public:
	CTraderSpi(const config& logConfig, CThostFtdcTraderApi* pTraderApi);
	CThostFtdcTraderApi* m_pTraderApi;
	config m_logConfig;
	int m_iRequestID;
	TThostFtdcOrderRefType m_MaxOrderRef;
	TThostFtdcFrontIDType m_FRONT_ID;
	TThostFtdcSessionIDType m_SESSION_ID;
	TThostFtdcOrderRefType m_ORDER_REF;
	CThostFtdcInputOrderField m_orderInsertReq;
	CThostFtdcParkedOrderField m_orderParkReq;
	unsigned int m_iNextOrderRef;
	tradeBase *m_tradeBase;
	vector<tradeBase*> tradeBaseVec;
	void fill_order_field();
	void fill_parked_order_field();

	///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
	virtual void OnFrontConnected();

	///登录请求响应
	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///请求查询投资者结算结果响应
	virtual void OnRspQrySettlementInfo(CThostFtdcSettlementInfoField *pSettlementInfo, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///投资者结算结果确认响应
	virtual void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	
	///请求查询合约响应
	virtual void OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///请求查询资金账户响应
	virtual void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///请求查询投资者持仓响应
	virtual void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///报单录入请求响应
	virtual void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnRspParkedOrderInsert(CThostFtdcParkedOrderField *pParkedOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///报单操作请求响应
	virtual void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///错误应答
	virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	
	///当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
	virtual void OnFrontDisconnected(int nReason);
		
	///心跳超时警告。当长时间未收到报文时，该方法被调用。
	virtual void OnHeartBeatWarning(int nTimeLapse);
	
	///报单通知
	virtual void OnRtnOrder(CThostFtdcOrderField *pOrder);

	///成交通知
	virtual void OnRtnTrade(CThostFtdcTradeField *pTrade);
	
	virtual void OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnRspQryParkedOrder(CThostFtdcParkedOrderField *pParkedOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	
	virtual void OnRspQryInstrumentMarginRate(CThostFtdcInstrumentMarginRateField *pInstrumentMarginRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

	virtual void OnRspQryBrokerTradingParams(CThostFtdcBrokerTradingParamsField *pBrokerTradingParams, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;
	
	///合约交易状态通知
	virtual void OnRtnInstrumentStatus(CThostFtdcInstrumentStatusField *pInstrumentStatus);
	
	
	virtual void OnRspRemoveParkedOrder(CThostFtdcRemoveParkedOrderField *pRemoveParkedOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);


	
	///报单录入请求
	int ReqOrderInsert(CThostFtdcInputOrderField *pInputOrder, int nRequestID);
	///报单操作请求
	int ReqOrderAction(CThostFtdcOrderField *pOrder);


	int ReqParkedOrderInsert(CThostFtdcParkedOrderField *pParkedOrder, int nRequestID);

    int ReqParkedOrderAction(CThostFtdcParkedOrderActionField *pParkedOrderAction);

	

	 ///报单操作请求
	 int ReqQryParkedOrder(TThostFtdcInstrumentIDType InstrumentID);

	
	
	int ReqQryAccount();
	int ReqQryInstrument(TThostFtdcInstrumentIDType	InstrumentID);
	int ReqQryInstrumentCommissionRate(TThostFtdcInstrumentIDType	InstrumentID);
	int ReqQryInstrumentMarginRate(TThostFtdcInstrumentIDType	InstrumentID);
	int ReqQryInvestorPosition(TThostFtdcInstrumentIDType	InstrumentID) ;
	int ReqQryOrder(TThostFtdcInstrumentIDType InstrumentID);

	/// 获取下一个报单引用
	unsigned int GetNextOrderRef();
	/// 获取下一个请求ID
	unsigned int GetNextReqID();
private:
	///用户登录请求
	int ReqUserLogin();
	///请求查询投资者持仓

	///请求查询投资者结算结果
	int ReqQrySettlementInfo();
	///投资者结算结果确认
	int ReqSettlementInfoConfirm();
	///请求查询合约
	
	///请求查询资金账户
	



	// 是否收到成功的响应
	bool IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo);
	// 是否我的报单回报
	bool IsMyOrder(CThostFtdcOrderField *pOrder);
	// 是否正在交易的报单
	bool IsTradingOrder(CThostFtdcOrderField *pOrder);

	vector<tradeBase*>::iterator find_tradeBase(TThostFtdcInstrumentIDType InstrumentID);
	tradeBase* find_tradeBase2(TThostFtdcInstrumentIDType InstrumentID);
};
//////////////////////////////////////////////////////////////////////////
inline bool is_offset_flag(char offsetFlag)
{
	if (offsetFlag == THOST_FTDC_OF_Close || 
		offsetFlag == THOST_FTDC_OF_CloseToday ||
		offsetFlag == THOST_FTDC_OF_CloseYesterday)
		return true;
	return false;
}

inline const char* side_str(char direction)
{
	if (direction == THOST_FTDC_D_Buy || direction == THOST_FTDC_PD_Long)
		return "buy";
	else
		return "sell";
}

inline const char* offset_str(char offsetflag)
{
	switch (offsetflag)
	{
	case THOST_FTDC_OF_Open:		return "open";
	case THOST_FTDC_OF_Close:		return "close";
	case THOST_FTDC_OF_CloseToday:	return "close_td";
	case THOST_FTDC_OF_CloseYesterday:	return "close_yd";
	}
	return "";
}

inline const char* ydpos_str(char histpos)
{
	if (histpos == THOST_FTDC_PSD_History)
		return "hist";
	else
		return "today";
}

inline const char* instru_stat(char instruStatus)
{
	switch (instruStatus)
	{
	case THOST_FTDC_IS_BeforeTrading:	return "BeforeTrading";
	case THOST_FTDC_IS_NoTrading:		return "NoTrading";
	case THOST_FTDC_IS_Continous:		return "Continous";
	case THOST_FTDC_IS_AuctionOrdering:	return "AuctionOrdering";
	case THOST_FTDC_IS_AuctionBalance:	return "AuctionBalance";
	case THOST_FTDC_IS_AuctionMatch:	return "AuctionMatch";
	case THOST_FTDC_IS_Closed:			return "Closed";
	default:	return "unknown";
	}
}

//////////////////////////////////////////////////////////////////////////
