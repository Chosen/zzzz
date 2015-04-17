#include <iostream>


#include ".\ThostTraderApi\ThostFtdcTraderApi.h"
#include "TraderSpi.h"
#include "futureApi.h"
#include "Utilities.h"
#include "logger.h"
#include <fstream>
using namespace std;
#pragma warning(disable : 4996)
extern CThostFtdcTradingAccountField myAccount;
extern bool bRspQryAccount;
extern int rspQryCommision;
extern int rspQryMarginRate;
extern int rspQryInstrument;
extern int rspQryPosition;
extern bool bLogInTrader;
extern bool g_bExistPreOrder;
int icommission=0;
extern vector<CThostFtdcInstrumentCommissionRateField>	commisionRate;
extern vector<CThostFtdcInstrumentMarginRateField>		marginRate;
extern vector<CThostFtdcInstrumentField>				instrument;
extern vector<CThostFtdcInvestorPositionField>			insturmentPosition;


CTraderSpi::CTraderSpi(const config& logConfig, CThostFtdcTraderApi* pTraderApi)
{
	m_logConfig		= logConfig;
	m_iNextOrderRef	= 0;
	m_iRequestID	= 0;
	m_pTraderApi	= pTraderApi;
	fill_order_field();
	fill_parked_order_field();
}

void CTraderSpi::fill_order_field()
{
	memset(&m_orderInsertReq,0,sizeof(m_orderInsertReq));

	///���͹�˾����
	strcpy(m_orderInsertReq.BrokerID, m_logConfig.Broker_ID);
	///Ͷ���ߴ���
	strcpy(m_orderInsertReq.InvestorID, m_logConfig.InvesterID);

	///�û�����
	//	TThostFtdcUserIDType	UserID;
	///�����۸�����: �޼�
	m_orderInsertReq.OrderPriceType = THOST_FTDC_OPT_LimitPrice;

	///���Ͷ���ױ���־
	m_orderInsertReq.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;

	///��Ч������: ������Ч
	m_orderInsertReq.TimeCondition = THOST_FTDC_TC_GFD;
	///GTD����
	//	TThostFtdcDateType	GTDDate;
	///�ɽ�������: �κ�����
	m_orderInsertReq.VolumeCondition = THOST_FTDC_VC_AV;
	///��С�ɽ���: 1
	m_orderInsertReq.MinVolume = 1;
	///��������: ����
	m_orderInsertReq.ContingentCondition = THOST_FTDC_CC_Immediately;
	///ֹ���
	//	TThostFtdcPriceType	StopPrice;
	///ǿƽԭ��: ��ǿƽ
	m_orderInsertReq.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
	///�Զ������־: ��
	m_orderInsertReq.IsAutoSuspend = 0;
	///ҵ��Ԫ
	//	TThostFtdcBusinessUnitType	BusinessUnit;
	///������
	//	TThostFtdcRequestIDType	RequestID;
	///�û�ǿ����־: ��
	m_orderInsertReq.UserForceClose = 0;

	sprintf_s(m_orderInsertReq.BusinessUnit, 20, "%s_stgA", m_orderInsertReq.InstrumentID);
}

void CTraderSpi::fill_parked_order_field()
{
	memset(&m_orderParkReq,0,sizeof(m_orderParkReq));

	///���͹�˾����
	strcpy(m_orderParkReq.BrokerID, m_logConfig.Broker_ID);
	///Ͷ���ߴ���
	strcpy(m_orderParkReq.InvestorID, m_logConfig.InvesterID);

	///�û�����
	//	TThostFtdcUserIDType	UserID;
	///�����۸�����: �޼�
	m_orderParkReq.OrderPriceType = THOST_FTDC_OPT_LimitPrice;

	///���Ͷ���ױ���־
	m_orderParkReq.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;

	///��Ч������: ������Ч
	m_orderParkReq.TimeCondition = THOST_FTDC_TC_GFD;
	///GTD����
	//	TThostFtdcDateType	GTDDate;
	///�ɽ�������: �κ�����
	m_orderParkReq.VolumeCondition = THOST_FTDC_VC_AV;
	///��С�ɽ���: 1
	m_orderParkReq.MinVolume = 1;
	///��������: ����
	m_orderParkReq.ContingentCondition = THOST_FTDC_CC_Immediately;
	///ֹ���
	//	TThostFtdcPriceType	StopPrice;
	///ǿƽԭ��: ��ǿƽ
	m_orderParkReq.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
	///�Զ������־: ��
	m_orderParkReq.IsAutoSuspend = 0;
	///ҵ��Ԫ
	//	TThostFtdcBusinessUnitType	BusinessUnit;
	///������
	//	TThostFtdcRequestIDType	RequestID;
	///�û�ǿ����־: ��
	m_orderParkReq.UserForceClose = 0;
}


//tested
void CTraderSpi::OnFrontConnected()
{
	cerr << "--->>> " << "OnFrontConnected" << endl;
	SyncLOG(INFO, "Trader OnFrontConnected");
	///�û���¼����
	ReqUserLogin();
}
//tested
int CTraderSpi::ReqUserLogin()
{
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, m_logConfig.Broker_ID);
	strcpy(req.UserID, m_logConfig.InvesterID);
	strcpy(req.Password,m_logConfig.PASSWORD);
	int iResult =m_pTraderApi->ReqUserLogin(&req, GetNextReqID());
	cerr << "--->>> �����û���¼����: "<<iResult<<" "  << ((iResult == 0) ? "�ɹ�" : "ʧ��") << endl;
	return iResult;
}
//tested
void CTraderSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
		CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "--->>> " << "OnRspUserLogin" << endl;
	if (bIsLast && !IsErrorRspInfo(pRspInfo))
	{
		// ����Ự����
		m_FRONT_ID = pRspUserLogin->FrontID;
		m_SESSION_ID = pRspUserLogin->SessionID;
		m_iNextOrderRef= atoi(pRspUserLogin->MaxOrderRef);
		m_iNextOrderRef++;
		sprintf(m_ORDER_REF, "%d", m_iNextOrderRef);
		///��ȡ��ǰ������
		cerr << "--->>> ��ȡ��ǰ������ = " << m_pTraderApi->GetTradingDay() << endl;
		SyncLOG(INFO, "OnRspUserLogin: FrontID=%d, SessionID=%d, MaxOrderRef=%s", m_FRONT_ID, m_SESSION_ID, pRspUserLogin->MaxOrderRef);

		///Ͷ���߽�����ȷ��
		ReqSettlementInfoConfirm();
		bLogInTrader = true;
	}
}

//tested
int CTraderSpi::ReqSettlementInfoConfirm()
{
	CThostFtdcSettlementInfoConfirmField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, m_logConfig.Broker_ID);
	strcpy(req.InvestorID, m_logConfig.InvesterID);
	int iResult = m_pTraderApi->ReqSettlementInfoConfirm(&req, GetNextReqID());
	cerr << "--->>> Ͷ���߽�����ȷ��: "<<iResult<<" "  << ((iResult == 0) ? "�ɹ�" : "ʧ��") << endl;
	return iResult;
}

//tested
void CTraderSpi::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "--->>> " << "OnRspSettlementInfoConfirm" << endl;
	if (bIsLast && !IsErrorRspInfo(pRspInfo))
	{
		CThostFtdcQryBrokerTradingParamsField req;
		memset(&req, 0, sizeof(req));
		strcpy(req.BrokerID, m_logConfig.Broker_ID);
		strcpy(req.InvestorID, m_logConfig.InvesterID);
		int iResult = m_pTraderApi->ReqQryBrokerTradingParams(&req, GetNextReqID());
		//cerr << "--->>> Ͷ���߽�����ȷ��: "<<iResult<<" "  << ((iResult == 0) ? "�ɹ�" : "ʧ��") << endl;
		bLogInTrader=true;
	}
}

void CTraderSpi::OnRspQrySettlementInfo(CThostFtdcSettlementInfoField *pSettlementInfo, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
}

void CTraderSpi::OnRspQryBrokerTradingParams(CThostFtdcBrokerTradingParamsField *pBrokerTradingParams, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	//cout<<pBrokerTradingParams->MarginPriceType<<endl;
}

//tested
int CTraderSpi::ReqQryInstrument(TThostFtdcInstrumentIDType InstrumentID)
{
	CThostFtdcQryInstrumentField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.InstrumentID, InstrumentID);
	//strcpy(req.ExchangeID, "SHFE");
	int iResult= m_pTraderApi->ReqQryInstrument(&req, ++m_iRequestID);
	cerr << "--->>> �����ѯ��Լ: "<<iResult<<" "  << ((iResult == 0) ? "�ɹ�" : "ʧ��") << endl;
	return iResult;
}

//tested
void CTraderSpi::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspInfo != NULL && pRspInfo->ErrorID != 0)
	{
		SyncLOG(ERRO, "OnRspQryInstrument: reqId=%d -> error [%d][%s]", nRequestID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
		return;
	}
	if (pInstrument != NULL)
	{
		instrument.push_back(*pInstrument);
		++rspQryInstrument;
		SyncLOG(INFO, "OnRspQryInstrument[%d]: sec=%s,expDate=%s,prcTick=%.3lf", rspQryInstrument, pInstrument->InstrumentID, pInstrument->ExpireDate, pInstrument->PriceTick);
	}

	/////
// 	/*m_tradeBase*/
// 	if(pInstrument==NULL)return;
// 	vector<tradeBase*>::iterator it=find_tradeBase(pInstrument->InstrumentID);
// 	if(it==tradeBaseVec.end() || tradeBaseVec.size()==0)return;
// 	(*it)->on_rsp_qry_Instrument(pInstrument);
}

//tested
int CTraderSpi::ReqQryInstrumentCommissionRate(TThostFtdcInstrumentIDType InstrumentID)
{
	CThostFtdcQryInstrumentCommissionRateField pQryCommissionRate;
	memset(&pQryCommissionRate, 0, sizeof(pQryCommissionRate));
	strcpy_s(pQryCommissionRate.BrokerID,m_logConfig.Broker_ID);
	strcpy_s(pQryCommissionRate.InvestorID,m_logConfig.InvesterID);
	strcpy_s(pQryCommissionRate.InstrumentID, InstrumentID);
	int iResult= m_pTraderApi->ReqQryInstrumentCommissionRate(&pQryCommissionRate, GetNextReqID());

	cerr << "--->>> �����ѯ��Լ��������: "<<iResult<<" "  << ((iResult == 0) ? "�ɹ�" : "ʧ��") << endl;
	return iResult;
}

//tested  
void CTraderSpi::OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if(pInstrumentCommissionRate==NULL)
	{
		if (pRspInfo != NULL)
			SyncLOG(ERRO, "OnRspQryInstrumentCommissionRate: [%d][%s]", pRspInfo->ErrorID, pRspInfo->ErrorMsg);

		CThostFtdcInstrumentCommissionRateField com;
		memset(&com, 0, sizeof(com));
		commisionRate.push_back(com);
		++rspQryCommision;
		return;
	}

	commisionRate.push_back(*pInstrumentCommissionRate);
	++rspQryCommision;

	CThostFtdcInstrumentCommissionRateField* p2 = pInstrumentCommissionRate;
	SyncLOG(INFO, "OnRspQryInstrumentCommissionRate: sec=%s,closeByMny=%.3lf,closeByVol=%.3lf, openByMny=%.3lf, openByVol=%.3lf,isLast=%d", p2->InstrumentID, 
		p2->CloseRatioByMoney, p2->CloseRatioByVolume, p2->OpenRatioByMoney, p2->OpenRatioByVolume, bIsLast);

// 	cout<<"icommission:"<<icommission<<endl;
// 	if(bIsLast)
// 	bRspQryCommision=true;
// 
// 	if(pInstrumentCommissionRate==NULL)  //ģ�⻷���鲻�� ���ؿ�ָ��
// 	{
// 		CThostFtdcInstrumentCommissionRateField comRate;
// 		memset(&comRate, 0, sizeof(comRate));
// 		// 		vector<tradeBase*>::iterator it;
// 		// 		for(it=tradeBaseVec.begin();it!=tradeBaseVec.end();++it)
// 		// 		{
// 		// 			tradeBaseVec.at(1);
// 		// 			(*it)->on_rsp_qry_instrument_commission_rate(&comRate);
// 		// 		}
// 		if(icommission<(int)tradeBaseVec.size())
// 		{
// 			tradeBaseVec.at(icommission)->on_rsp_qry_instrument_commission_rate(&comRate);
// 			++icommission;
// 		}
// 		return;
// 	}

	//vector<tradeBase*>::iterator it=find_tradeBase(pInstrumentCommissionRate->InstrumentID);
	// 	vector<tradeBase*>::iterator it;
	// 	int len=strlen(pInstrumentCommissionRate->InstrumentID);
	// 	for(it=tradeBaseVec.begin();it!=tradeBaseVec.end();++it)
	// 	{
	// 		futureApi* pFuture=(futureApi*)(*it);
	// 
	// 		string insturmentString=pFuture->m_INSTRUMENT_ID;
	// 
	// 		if(strcmp(pInstrumentCommissionRate->InstrumentID,insturmentString.substr(0,len).c_str())==0)
	// 		{
	// 			 (*it)->on_rsp_qry_instrument_commission_rate(pInstrumentCommissionRate);
	// 		}
	// 	}

// 	if(icommission<(int)tradeBaseVec.size())
// 	{
// 		tradeBaseVec.at(icommission)->on_rsp_qry_instrument_commission_rate(pInstrumentCommissionRate);
// 		++icommission;
// 	}
// 	return;

	// 	if(tradeBaseVec.size()==0)
	// 	{
	// 		cout<<"�����Ѳ�ѯ���ɹ�"<<endl;
	// 		return;
	// 	}

}

//tested
int CTraderSpi::ReqQryInstrumentMarginRate(TThostFtdcInstrumentIDType InstrumentID)
{
	CThostFtdcQryInstrumentMarginRateField pQryInstrumentMarginRate;
	memset(&pQryInstrumentMarginRate, 0, sizeof(pQryInstrumentMarginRate));
	strcpy_s(pQryInstrumentMarginRate.BrokerID,m_logConfig.Broker_ID);
	strcpy_s(pQryInstrumentMarginRate.InvestorID,m_logConfig.InvesterID);
	strcpy_s(pQryInstrumentMarginRate.InstrumentID,InstrumentID);
	pQryInstrumentMarginRate.HedgeFlag=THOST_FTDC_HF_Speculation;
	int iResult=m_pTraderApi->ReqQryInstrumentMarginRate(&pQryInstrumentMarginRate,++m_iRequestID);

	cerr << "--->>> �����ѯ��Լ��֤����: " <<iResult<<" " << ((iResult == 0) ? "�ɹ�" : "ʧ��") << endl;

	//ReqQryParkedOrder(InstrumentID);
	return iResult;
}


//tested
void CTraderSpi::OnRspQryInstrumentMarginRate(CThostFtdcInstrumentMarginRateField *pInstrumentMarginRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{

	if(pInstrumentMarginRate!=NULL)
	{
		marginRate.push_back(*pInstrumentMarginRate);
		++rspQryMarginRate;
	}else
	{
		cout<<endl<<"δ��ѯ����Լ�ı�֤���ʣ��ú�Լ���ܲ�����"<<endl;
		SyncLOG(ERRO, "δ��ѯ����Լ�ı�֤���ʣ��ú�Լ���ܲ�����: [%d][%s]", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
	}
	

// 	vector<tradeBase*>::iterator it=find_tradeBase(pInstrumentMarginRate->InstrumentID);
// 	if(it==tradeBaseVec.end() || tradeBaseVec.size()==0)return;
// 	(*it)->on_rsp_qry_instrument_margin_rate(pInstrumentMarginRate);
}


//tested
int CTraderSpi::ReqQryAccount()
{
	CThostFtdcQryTradingAccountField reqTradingAccount;
	memset(&reqTradingAccount, 0, sizeof(reqTradingAccount));
	strcpy(reqTradingAccount.BrokerID, m_logConfig.Broker_ID);
	strcpy(reqTradingAccount.InvestorID, m_logConfig.InvesterID);
	int iResult=m_pTraderApi->ReqQryTradingAccount(&reqTradingAccount,++m_iRequestID);

	cerr << "--->>> �����ѯ�˻��ʽ�: "<<iResult<<" "<< ((iResult == 0) ? "�ɹ�" : "ʧ��") << endl;

	return iResult;
}

//tested
void CTraderSpi::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	myAccount = *pTradingAccount;
	bRspQryAccount = true;
	if (pRspInfo != NULL && pRspInfo->ErrorID != 0)
	{
		SyncLOG(ERRO, "OnRspQryTradingAccount: error [%d][%s]", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
		return;
	}
	if (pTradingAccount != NULL)
	{
		CThostFtdcTradingAccountField* pta = pTradingAccount;
		SyncLOG(INFO, "OnRspQryTradingAccount: accntID=%s,avail=%.3lf,rPnl=%.3lf,posPnl=%.3lf,commi=%.3lf,mgn=%.3lf,frozCash=%.3lf,frozCommi=%.3lf,frozMgn=%.3lf", 
			pta->AccountID, pta->Available, pta->CloseProfit, pta->PositionProfit, pta->Commission, 
			pta->CurrMargin, pta->FrozenCash, pta->FrozenCommission, pta->FrozenMargin);
	}

	// 	if(tradeBaseVec.size()>0)  //�ʽ���ȫ�־�̬����
	// 	{
	// 		tradeBaseVec.front()->on_rsp_qry_instrument_trading_account(pTradingAccount);
	// 	}

	// 	vector<tradeBase*>::iterator it;
	// 	for(it=tradeBaseVec.begin();it!=tradeBaseVec.end();++it)
	// 	{
	// 		(*it)->on_rsp_qry_instrument_trading_account(pTradingAccount);
	// 	}


}


int CTraderSpi::ReqQryOrder(TThostFtdcInstrumentIDType InstrumentID)
{
	CThostFtdcQryOrderField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, m_logConfig.Broker_ID);
	strcpy(req.InvestorID, m_logConfig.InvesterID);
	strcpy(req.InstrumentID, InstrumentID);
	int iResult = m_pTraderApi->ReqQryOrder(&req,++m_iRequestID);
	cerr << "--->>> �����ѯί����Ϣ: "<<iResult<<" "<< ((iResult == 0) ? "�ɹ�" : "ʧ��") << endl;
	return iResult;
}

int CTraderSpi::ReqQryParkedOrder(TThostFtdcInstrumentIDType InstrumentID)
{
	CThostFtdcQryParkedOrderField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, m_logConfig.Broker_ID);
	strcpy(req.InvestorID, m_logConfig.InvesterID);
	strcpy(req.InstrumentID, InstrumentID);
	int iResult = m_pTraderApi->ReqQryParkedOrder(&req,++m_iRequestID);
	cerr << "--->>> �����ѯԤ��ί����Ϣ: "<<iResult<<" "<< ((iResult == 0) ? "�ɹ�" : "ʧ��") << endl;
	return iResult;
}



void CTraderSpi::OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if(pOrder != NULL)
	{
		tradeBase* ptdBase = find_tradeBase2(pOrder->InstrumentID);
		if (ptdBase == NULL)
		{
			cout << "���󣺶�������δ�ҵ�" << endl;
			SyncLOG(WARN, "OnRspQryOrder: not find the order -> sec=%s,qt=%d,ref=%d\n", pOrder->InstrumentID, pOrder->VolumeTotalOriginal, pOrder->OrderRef);
			return;
		}

		CThostFtdcOrderField* po = pOrder;
		SyncLOG(INFO, "OnRspQryOrder: sec=%s,volOrig=%d,volTd=%d,volTotal=%d,prc=%.3lf,dir=%s,offset=%s", po->InstrumentID, 
			po->VolumeTotalOriginal, po->VolumeTraded, po->VolumeTotal, po->LimitPrice, side_str(po->Direction), offset_str(po->CombOffsetFlag[0]));
		ptdBase->on_rsp_qry_order(pOrder, bIsLast);
	}
	else
	{
		if (pRspInfo != NULL)
			SyncLOG(ERRO, "OnRspQryOrder: [%d][%s]", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
		g_bExistPreOrder = false;
	}
}

void CTraderSpi::OnRspQryParkedOrder(CThostFtdcParkedOrderField *pParkedOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	
// 	CThostFtdcRemoveParkedOrderField req;
// 	memset(&req, 0, sizeof(req));
// 	strcpy(req.BrokerID, m_logConfig.Broker_ID);
// 	strcpy(req.InvestorID, m_logConfig.InvesterID);
// 	strcpy(req.ParkedOrderID, pParkedOrder->ParkedOrderID);
// 	int iResult=m_pTraderApi->ReqRemoveParkedOrder(&req,++m_iRequestID);
}

void CTraderSpi::OnRspRemoveParkedOrder(CThostFtdcRemoveParkedOrderField *pRemoveParkedOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cout<<pRemoveParkedOrder->ParkedOrderID<<endl;
}


//tested
int CTraderSpi::ReqQryInvestorPosition(TThostFtdcInstrumentIDType	InstrumentID)
{
	CThostFtdcQryInvestorPositionField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, m_logConfig.Broker_ID);
	strcpy(req.InvestorID, m_logConfig.InvesterID);
	strcpy(req.InstrumentID, InstrumentID);
	int iResult = m_pTraderApi->ReqQryInvestorPosition(&req, ++m_iRequestID);
	cerr << "--->>> �����ѯͶ���ֲ߳�: "<<iResult<<" " << ((iResult == 0) ? "�ɹ�" : "ʧ��") << endl;
	return 0;
}

//tested ģ�⻷���ƺ��鲻��  �޲�λҲ�鲻��
void CTraderSpi::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
// 	cerr << "--->>> " << "OnRspQryInvestorPosition" << endl;
// 
// 
// 	if(pInvestorPosition==NULL)
// 	{
// 		CThostFtdcInvestorPositionField investorPosition;
// 		memset(&investorPosition, 0, sizeof(investorPosition));
// 		insturmentPosition.push_back(investorPosition);
// 		++rspQryPosition;
// 	}else
// 	{
// 
// 	}

	if(pInvestorPosition==NULL)  //ģ�⻷���鲻�� ���ؿ�ָ��
	{
		CThostFtdcInvestorPositionField investorPosition;
		memset(&investorPosition, 0, sizeof(investorPosition));
		vector<tradeBase*>::iterator it;
		for(it = tradeBaseVec.begin(); it != tradeBaseVec.end(); ++it)
		{
			futureApi* pFuture = (futureApi*)(*it);
			pFuture->m_bIsLastReqPos = bIsLast;
			(*it)->on_rsp_qry_position(&investorPosition);
		}
		return;
	}
	

	tradeBase* ptdBase = find_tradeBase2(pInvestorPosition->InstrumentID);
	if (ptdBase == NULL)
	{
		SyncLOG(ERRO, "OnRspQryInvestorPosition: not find the trade base for [%s]", pInvestorPosition->InstrumentID);
		return;
	}

	futureApi* pFuture = static_cast<futureApi*>(ptdBase);
	if (bIsLast && !IsErrorRspInfo(pRspInfo))
	{
		pFuture->m_bIsLastReqPos = bIsLast;
	}
	
	// update local positions here
	ptdBase->on_rsp_qry_position(pInvestorPosition);

}

//tested
int CTraderSpi::ReqOrderInsert(CThostFtdcInputOrderField *pInputOrder, int nRequestID)
{
	int iResult = m_pTraderApi->ReqOrderInsert(pInputOrder, nRequestID);

	char buf[512] = "";
	sprintf(buf, "ReqOrderInsert[%d]: sec=%s,ref=%s,qt=%d,dir=%c,offset=%c\n", iResult, pInputOrder->InstrumentID, 
		pInputOrder->OrderRef, pInputOrder->VolumeTotalOriginal, pInputOrder->Direction, pInputOrder->CombOffsetFlag[0]);
	Log2DebugView(buf);
	SyncLOG((iResult == 0) ? INFO : ERRO, buf);
	return iResult;
}


//tested �ʽ��� �����ܾ�
void CTraderSpi::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "--->>> " << "OnRspOrderInsert" << endl;
	//

	if(IsErrorRspInfo(pRspInfo))
	{
		if (pRspInfo)
			SyncLOG(ERRO, "OnRspOrderInsert: reqId=%d -> [%d][%s]", nRequestID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);

		tradeBase* ptdBase = find_tradeBase2(pInputOrder->InstrumentID);
		if(ptdBase == NULL)
		{
			SyncLOG(ERRO, "OnRspOrderInsert: ptdBase is null sec=%s,ref=%s,vol=%d", pInputOrder->InstrumentID, pInputOrder->OrderRef, pInputOrder->VolumeTotalOriginal);
			return;
		}

		// ���ڱ������ܾ����ᾭ���������˴�����һ����ʱ��CThostFtdcOrderField order
		CThostFtdcOrderField rejectOrder;
		memset(&rejectOrder, 0, sizeof(rejectOrder));
		strcpy(rejectOrder.BrokerID,		pInputOrder->BrokerID);
		strcpy(rejectOrder.InvestorID,		pInputOrder->InvestorID);
		strcpy(rejectOrder.InstrumentID,	pInputOrder->InstrumentID);
		strcpy(rejectOrder.OrderRef,		pInputOrder->OrderRef);
		strcpy(rejectOrder.BusinessUnit,	pInputOrder->BusinessUnit);
		strcpy(rejectOrder.CombOffsetFlag,	pInputOrder->CombOffsetFlag);
		strcpy(rejectOrder.CombHedgeFlag,	pInputOrder->CombHedgeFlag);
		rejectOrder.VolumeTotalOriginal =	pInputOrder->VolumeTotalOriginal;
		rejectOrder.OrderPriceType		=	pInputOrder->OrderPriceType;
		rejectOrder.LimitPrice	= pInputOrder->LimitPrice;
		rejectOrder.Direction	= pInputOrder->Direction;
		rejectOrder.StopPrice	= pInputOrder->StopPrice;
		rejectOrder.FrontID		= m_FRONT_ID;
		rejectOrder.SessionID	= m_SESSION_ID;
		// TODO: filled other fields

		futureApi* pFuture = static_cast<futureApi*>(ptdBase);
		ptdBase->on_order_rejected(&rejectOrder);
	}
}


///����֪ͨ
void CTraderSpi::OnRtnOrder(CThostFtdcOrderField *pOrder)
{
	Log2DebugView("OnRtnOrder: sec=%s,ref=%s,qt=%d,td_qt=%d,status=%c,substat=%c,dir=%s,offset=%s,sysid=%s,sessId=%d,msg=%s\n", 
		pOrder->InstrumentID, pOrder->OrderRef, pOrder->VolumeTotal, pOrder->VolumeTraded, pOrder->OrderStatus, pOrder->OrderSubmitStatus, 
		side_str(pOrder->Direction), offset_str(pOrder->CombOffsetFlag[0]), pOrder->OrderSysID, pOrder->SessionID, pOrder->StatusMsg);

	tradeBase* ptdBase = find_tradeBase2(pOrder->InstrumentID);
	if(ptdBase == NULL)
		return;

	futureApi* pFuture = static_cast<futureApi*>(ptdBase);
	/*if(pFuture->find_order(pOrder)==pFuture->m_orderVec.end())
		return;
	*/

	// TODO: how to verify the order whether is from local
	if (!IsMyOrder(pOrder))
	{
		SyncLOG(WARN, "OnRtnOrder: not local order sec=%s,qt=%d,dir=%s,stat=%c,sessId=%d,time=%s", pOrder->InstrumentID,
			pOrder->VolumeTotalOriginal, side_str(pOrder->Direction), pOrder->OrderStatus, pOrder->SessionID, pOrder->UpdateTime);
		return;
	}

	if(pOrder->OrderSubmitStatus == THOST_FTDC_OSS_InsertSubmitted && pOrder->OrderStatus == THOST_FTDC_OST_Unknown)
	{
		ptdBase->on_order_submitted(pOrder);//tested
	}
	else if(pOrder->OrderSubmitStatus == THOST_FTDC_OSS_Accepted && pOrder->OrderStatus == THOST_FTDC_OST_NoTradeQueueing)
	{
		ptdBase->on_order_accepted(pOrder);//tested
	}
	else if(pOrder->OrderSubmitStatus == THOST_FTDC_OSS_InsertRejected)
	{
		ptdBase->on_order_rejected(pOrder);
	}
	else if(pOrder->OrderSubmitStatus == THOST_FTDC_OSS_CancelRejected)   ///�ӿڿ��ܲ����������
	{
		ptdBase->on_order_cancel_rejected(pOrder);
	}	

	if(pOrder->OrderStatus == THOST_FTDC_OST_Canceled)   ////�Ѳ���
	{
		ptdBase->on_order_cancel(pOrder);
	}
}

///�ɽ�֪ͨ
void CTraderSpi::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
	Log2DebugView("OnRtnTrade: sec=%s,ref=%s,qt=%d,sysid=%s\n", pTrade->InstrumentID, pTrade->OrderRef, pTrade->Volume, pTrade->OrderSysID);

	//cerr << "--->>> " << "OnRtnTrade"  << endl;
	tradeBase* ptdBase = find_tradeBase2(pTrade->InstrumentID);
	if(ptdBase == NULL)
		return;
	ptdBase->on_order_execution(pTrade);
}

//tested
vector<tradeBase*>::iterator CTraderSpi::find_tradeBase(TThostFtdcInstrumentIDType InstrumentID)
{
	vector<tradeBase*>::iterator it;
	for(it=tradeBaseVec.begin();it!=tradeBaseVec.end();++it)
	{
		futureApi* pFuture=(futureApi*)(*it);
		if(strcmp(InstrumentID,pFuture->m_INSTRUMENT_ID)==0)
		{
			return it;
		}
	}

// 	int len=strlen(InstrumentID);
// 	for(it=tradeBaseVec.begin();it!=tradeBaseVec.end();++it)
// 	{
// 		futureApi* pFuture=(futureApi*)(*it);
// 		
// 		string insturmentString=pFuture->m_INSTRUMENT_ID;
// 
// 		if(strcmp(InstrumentID,insturmentString.substr(0,len).c_str())==0)
// 		{
// 			return it;
// 		}
// 	}

	return it;
}

//tested
tradeBase* CTraderSpi::find_tradeBase2(TThostFtdcInstrumentIDType InstrumentID)
{
	tradeBase* pBase = NULL;

	for(vector<tradeBase*>::iterator it = tradeBaseVec.begin(); it!=tradeBaseVec.end(); ++it)
	{
		futureApi* pFuture = static_cast<futureApi*>(*it);
		if(strcmp(InstrumentID, pFuture->m_INSTRUMENT_ID)==0)
		{
			pBase = *it;
			break;
		}
	}

	return pBase;
}

bool CTraderSpi::IsMyOrder(CThostFtdcOrderField *pOrder)
{
	return ((pOrder->FrontID == m_FRONT_ID) && (pOrder->SessionID == m_SESSION_ID));
}

// bool CTraderSpi::IsMyInputOrder(CThostFtdcInputOrderField *pOrder)
// {
// 	return ((pOrder->FrontID ==m_FRONT_ID) &&
// 		(pOrder->SessionID ==m_SESSION_ID));
// }


bool CTraderSpi::IsTradingOrder(CThostFtdcOrderField *pOrder)
{
	return ((pOrder->OrderStatus != THOST_FTDC_OST_PartTradedNotQueueing) &&
			(pOrder->OrderStatus != THOST_FTDC_OST_Canceled) &&
			(pOrder->OrderStatus != THOST_FTDC_OST_AllTraded));
}

unsigned int CTraderSpi::GetNextOrderRef()
{
	return AtomicAdd(&m_iNextOrderRef, 1);
}

unsigned int CTraderSpi::GetNextReqID()
{
	return AtomicAdd(&m_iRequestID, 1);
}

int CTraderSpi::ReqParkedOrderInsert(CThostFtdcParkedOrderField *pParkedOrder, int nRequestID)
{
	return m_pTraderApi->ReqParkedOrderInsert(pParkedOrder, nRequestID);
}


void CTraderSpi::OnRspParkedOrderInsert(CThostFtdcParkedOrderField *pParkedOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << endl;
	cout<<pParkedOrder->LimitPrice<<endl;
}


int CTraderSpi::ReqOrderAction(CThostFtdcOrderField *pOrder)
{
	CThostFtdcInputOrderActionField req;
	memset(&req, 0, sizeof(req));
	///���͹�˾����
	strcpy(req.BrokerID, pOrder->BrokerID);
	///Ͷ���ߴ���
	strcpy(req.InvestorID, pOrder->InvestorID);
	///������������
	//	TThostFtdcOrderActionRefType	OrderActionRef;
	///��������
	strcpy(req.OrderRef, pOrder->OrderRef);
	///������
	//	TThostFtdcRequestIDType	RequestID;
	///ǰ�ñ��
	req.FrontID = m_FRONT_ID;
	///�Ự���
	req.SessionID = m_SESSION_ID;
	///����������
	//	TThostFtdcExchangeIDType	ExchangeID;
	///�������
	//	TThostFtdcOrderSysIDType	OrderSysID;
	///������־
	req.ActionFlag = THOST_FTDC_AF_Delete;
	///�۸�
	//	TThostFtdcPriceType	LimitPrice;
	///�����仯
	//	TThostFtdcVolumeType	VolumeChange;
	///�û�����
	//	TThostFtdcUserIDType	UserID;
	///��Լ����
	strcpy(req.InstrumentID, pOrder->InstrumentID);

	int iResult = m_pTraderApi->ReqOrderAction(&req, ++m_iRequestID);
	cerr << "--->>> ������������: " << ((iResult == 0) ? "�ɹ�" : "ʧ��") << endl;
	Log2DebugView("ReqOrderAction: ref=%s\n", req.OrderRef);
	
	return iResult;
}


int CTraderSpi::ReqParkedOrderAction(CThostFtdcParkedOrderActionField *pOrder)
{
	static bool ORDER_ACTION_SENT = false;		//�Ƿ����˱���
	if (ORDER_ACTION_SENT)
		return -1;

	CThostFtdcParkedOrderActionField req;
	memset(&req, 0, sizeof(req));
	///���͹�˾����
	strcpy(req.BrokerID, pOrder->BrokerID);
	///Ͷ���ߴ���
	strcpy(req.InvestorID, pOrder->InvestorID);
	///������������
	//	TThostFtdcOrderActionRefType	OrderActionRef;
	///��������
	strcpy(req.OrderRef, pOrder->OrderRef);
	///������
	//	TThostFtdcRequestIDType	RequestID;
	///ǰ�ñ��
	req.FrontID = m_FRONT_ID;
	///�Ự���
	req.SessionID = m_SESSION_ID;
	///����������
	//	TThostFtdcExchangeIDType	ExchangeID;
	///�������
	//	TThostFtdcOrderSysIDType	OrderSysID;
	///������־
	req.ActionFlag = THOST_FTDC_AF_Delete;
	///�۸�
	//	TThostFtdcPriceType	LimitPrice;
	///�����仯
	//	TThostFtdcVolumeType	VolumeChange;
	///�û�����
	//	TThostFtdcUserIDType	UserID;
	///��Լ����
	strcpy(req.InstrumentID, pOrder->InstrumentID);

	int iResult=m_pTraderApi->ReqParkedOrderAction(&req, ++m_iRequestID);
	cerr << "--->>> ������������: " << ((iResult == 0) ? "�ɹ�" : "ʧ��") << endl;
	ORDER_ACTION_SENT = true;
	return iResult;
}


void CTraderSpi::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "--->>> " << "OnRspOrderAction" << endl;
	IsErrorRspInfo(pRspInfo);
}


void CTraderSpi:: OnFrontDisconnected(int nReason)
{
	cerr << "--->>> " << "OnFrontDisconnected" << endl;
	cerr << "--->>> Reason = " << nReason << endl;
	SyncLOG(ERRO, "Trader OnFrontDisconnected [%d]", nReason);
}

void CTraderSpi::OnHeartBeatWarning(int nTimeLapse)
{
	cerr << "--->>> " << "OnHeartBeatWarning" << endl;
	cerr << "--->>> nTimerLapse = " << nTimeLapse << endl;
}

void CTraderSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "--->>> " << "OnRspError" << endl;
	IsErrorRspInfo(pRspInfo);
}

bool CTraderSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{
	// ���ErrorID != 0, ˵���յ��˴������Ӧ
	bool bResult = ((pRspInfo) && (pRspInfo->ErrorID != 0));
	if (bResult)
	{
		cerr << "--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << endl;
		AsyncLOG(ERRO, "IsErrorRspInfo: [%d][%s]", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
	}
	return bResult;
}

void CTraderSpi::OnRtnInstrumentStatus(CThostFtdcInstrumentStatusField *pInstrumentStatus)
{
	CThostFtdcInstrumentStatusField* pi = pInstrumentStatus;
	AsyncLOG(ERRO, "OnRtnInstrumentStatus: [%s,%s,%s]", pi->InstrumentID, instru_stat(pi->InstrumentStatus), pi->EnterTime);
}