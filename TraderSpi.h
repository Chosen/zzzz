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

	///���ͻ����뽻�׺�̨������ͨ������ʱ����δ��¼ǰ�����÷��������á�
	virtual void OnFrontConnected();

	///��¼������Ӧ
	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///�����ѯͶ���߽�������Ӧ
	virtual void OnRspQrySettlementInfo(CThostFtdcSettlementInfoField *pSettlementInfo, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///Ͷ���߽�����ȷ����Ӧ
	virtual void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	
	///�����ѯ��Լ��Ӧ
	virtual void OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///�����ѯ�ʽ��˻���Ӧ
	virtual void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///�����ѯͶ���ֲ߳���Ӧ
	virtual void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///����¼��������Ӧ
	virtual void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnRspParkedOrderInsert(CThostFtdcParkedOrderField *pParkedOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///��������������Ӧ
	virtual void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///����Ӧ��
	virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	
	///���ͻ����뽻�׺�̨ͨ�����ӶϿ�ʱ���÷��������á���������������API���Զ��������ӣ��ͻ��˿ɲ�������
	virtual void OnFrontDisconnected(int nReason);
		
	///������ʱ���档����ʱ��δ�յ�����ʱ���÷��������á�
	virtual void OnHeartBeatWarning(int nTimeLapse);
	
	///����֪ͨ
	virtual void OnRtnOrder(CThostFtdcOrderField *pOrder);

	///�ɽ�֪ͨ
	virtual void OnRtnTrade(CThostFtdcTradeField *pTrade);
	
	virtual void OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnRspQryParkedOrder(CThostFtdcParkedOrderField *pParkedOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	
	virtual void OnRspQryInstrumentMarginRate(CThostFtdcInstrumentMarginRateField *pInstrumentMarginRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

	virtual void OnRspQryBrokerTradingParams(CThostFtdcBrokerTradingParamsField *pBrokerTradingParams, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;
	
	///��Լ����״̬֪ͨ
	virtual void OnRtnInstrumentStatus(CThostFtdcInstrumentStatusField *pInstrumentStatus);
	
	
	virtual void OnRspRemoveParkedOrder(CThostFtdcRemoveParkedOrderField *pRemoveParkedOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);


	
	///����¼������
	int ReqOrderInsert(CThostFtdcInputOrderField *pInputOrder, int nRequestID);
	///������������
	int ReqOrderAction(CThostFtdcOrderField *pOrder);


	int ReqParkedOrderInsert(CThostFtdcParkedOrderField *pParkedOrder, int nRequestID);

    int ReqParkedOrderAction(CThostFtdcParkedOrderActionField *pParkedOrderAction);

	

	 ///������������
	 int ReqQryParkedOrder(TThostFtdcInstrumentIDType InstrumentID);

	
	
	int ReqQryAccount();
	int ReqQryInstrument(TThostFtdcInstrumentIDType	InstrumentID);
	int ReqQryInstrumentCommissionRate(TThostFtdcInstrumentIDType	InstrumentID);
	int ReqQryInstrumentMarginRate(TThostFtdcInstrumentIDType	InstrumentID);
	int ReqQryInvestorPosition(TThostFtdcInstrumentIDType	InstrumentID) ;
	int ReqQryOrder(TThostFtdcInstrumentIDType InstrumentID);

	/// ��ȡ��һ����������
	unsigned int GetNextOrderRef();
	/// ��ȡ��һ������ID
	unsigned int GetNextReqID();
private:
	///�û���¼����
	int ReqUserLogin();
	///�����ѯͶ���ֲ߳�

	///�����ѯͶ���߽�����
	int ReqQrySettlementInfo();
	///Ͷ���߽�����ȷ��
	int ReqSettlementInfoConfirm();
	///�����ѯ��Լ
	
	///�����ѯ�ʽ��˻�
	



	// �Ƿ��յ��ɹ�����Ӧ
	bool IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo);
	// �Ƿ��ҵı����ر�
	bool IsMyOrder(CThostFtdcOrderField *pOrder);
	// �Ƿ����ڽ��׵ı���
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
