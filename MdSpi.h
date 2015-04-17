#pragma once
#include ".\ThostTraderApi\ThostFtdcMdApi.h"
#include "TraderSpi.h"

class  quoteBase
{
public:
	//virtual ~quoteBase();
	virtual void on_quote_change(CThostFtdcDepthMarketDataField *pDepthMarketData)=0;

};
class CMdSpi : public CThostFtdcMdSpi
{
public:
	quoteBase* m_quoteBase;
	vector<quoteBase*> quoteBaseVec;
	CMdSpi(char* ppInstrumentID[],int numInsturmentID,config logConfig,CThostFtdcMdApi* pMdApi);
	///����Ӧ��
	virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo,
		int nRequestID, bool bIsLast);

	///���ͻ����뽻�׺�̨ͨ�����ӶϿ�ʱ���÷��������á���������������API���Զ��������ӣ��ͻ��˿ɲ�������
	///@param nReason ����ԭ��
	///        0x1001 �����ʧ��
	///        0x1002 ����дʧ��
	///        0x2001 ����������ʱ
	///        0x2002 ��������ʧ��
	///        0x2003 �յ�������
	virtual void OnFrontDisconnected(int nReason);
		
	///������ʱ���档����ʱ��δ�յ�����ʱ���÷��������á�
	///@param nTimeLapse �����ϴν��ձ��ĵ�ʱ��
	virtual void OnHeartBeatWarning(int nTimeLapse);

	///���ͻ����뽻�׺�̨������ͨ������ʱ����δ��¼ǰ�����÷��������á�
	virtual void OnFrontConnected();
	
	///��¼������Ӧ
	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///��������Ӧ��
	virtual void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///ȡ����������Ӧ��
	virtual void OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///�������֪ͨ
	virtual void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData);
	void SubscribeMarketData();
private:
	void ReqUserLogin();
	
	// 
	bool IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo);
	CThostFtdcMdApi* m_pMdApi;
	config m_logConifg;
	int m_iRequestID;
	char* m_INSTRUMENT_ID[1024];
	int m_numInsturmentID;
	vector<quoteBase*>::iterator find_quoteBase(char* isntrumentID);
};