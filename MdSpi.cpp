#include "MdSpi.h"
#include "futureApi.h"
#include "Utilities.h"
#include "logger.h"
#include <iostream>
using namespace std;

#pragma warning(disable : 4996)
extern bool bLogMd;


CMdSpi::CMdSpi(char* ppInstrumentID[],int numInsturmentID,config logConfig,CThostFtdcMdApi* pMdApi)
{
	for(int i=0;i<numInsturmentID;++i)
	{
		m_INSTRUMENT_ID[i]=ppInstrumentID[i];
	}
	m_numInsturmentID=numInsturmentID;
	m_pMdApi=pMdApi;
	m_logConifg=logConfig;
}

void CMdSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo,
		int nRequestID, bool bIsLast)
{
	cerr << "--->>> "<< __FUNCTION__ << endl;
	IsErrorRspInfo(pRspInfo);
}

void CMdSpi::OnFrontDisconnected(int nReason)
{
	cerr << "--->>> " << __FUNCTION__ << endl;
	cerr << "--->>> Reason = " << nReason << endl;
}
		
void CMdSpi::OnHeartBeatWarning(int nTimeLapse)
{
	cerr << "--->>> " << __FUNCTION__ << endl;
	cerr << "--->>> nTimerLapse = " << nTimeLapse << endl;
}

void CMdSpi::OnFrontConnected()
{
	cerr << "--->>> " << __FUNCTION__ << endl;
	///用户登录请求
	ReqUserLogin();
}

void CMdSpi::ReqUserLogin()
{
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, m_logConifg.Broker_ID);
	strcpy(req.UserID, m_logConifg.InvesterID);
	strcpy(req.Password, m_logConifg.PASSWORD);
	int iResult = m_pMdApi->ReqUserLogin(&req, ++m_iRequestID);
	cerr << "--->>> 发送用户登录请求: " << ((iResult == 0) ? "成功" : "失败") << endl;
}

void CMdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
		CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "--->>> " << __FUNCTION__ << endl;
	if (bIsLast && !IsErrorRspInfo(pRspInfo))
	{
		///获取当前交易日
		//cerr << "--->>> 获取当前交易日 = " << m_pMdApi->GetTradingDay() << endl;
		bLogMd=true;
		// 请求订阅行情
		//SubscribeMarketData();	
	}
}

void CMdSpi::SubscribeMarketData()
{
	int iResult = m_pMdApi->SubscribeMarketData(m_INSTRUMENT_ID, m_numInsturmentID);
	cerr << "--->>> 发送行情订阅请求: " << ((iResult == 0) ? "成功" : "失败") << endl;
}

void CMdSpi::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << __FUNCTION__ << endl;
}

void CMdSpi::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << __FUNCTION__ << endl;
}

void CMdSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
	CThostFtdcDepthMarketDataField *p = pDepthMarketData;
	vector<quoteBase*>::iterator it=find_quoteBase(pDepthMarketData->InstrumentID);
	if(it == quoteBaseVec.end() || quoteBaseVec.size() == 0)
	{
		return;
	}
	(*it)->on_quote_change(pDepthMarketData);
	
	//Log2DebugView("OnMktData: sec=%s,prc=%.3f,bid=%d,ask=%d\n", p->InstrumentID, p->LastPrice, p->BidVolume1, p->AskVolume1);
	//cout<<"quote:"<<pDepthMarketData->InstrumentID<<endl;
	SyncLOG(DEBUG, "OnMktData: sec=%s,vol=%d, bid=%.3lf,%d, ask=%.3lf,%d\n", p->InstrumentID, p->Volume, p->BidPrice1, p->BidVolume1, p->AskPrice1, p->AskVolume1);
}

vector<quoteBase*>::iterator CMdSpi::find_quoteBase(char* instrumentID)
{
	vector<quoteBase*>::iterator it;
	for(it=quoteBaseVec.begin();it!=quoteBaseVec.end();++it)
	{
		futureApi* pFuture=(futureApi*)(*it);
		if(strcmp(instrumentID,pFuture->m_INSTRUMENT_ID)==0)
			break;
	}
	return it;
}

bool CMdSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{
	// 如果ErrorID != 0, 说明收到了错误的响应
	bool bResult = ((pRspInfo) && (pRspInfo->ErrorID != 0));
	if (bResult)
		cerr << "--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << endl;
	return bResult;
}