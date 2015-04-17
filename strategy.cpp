#include "strategy.h"



strategy *rb,*rb1510;
strategy::strategy(TThostFtdcInstrumentIDType INSTRUMENT_ID, 
	CMdSpi* pMdSPi, 
	CTraderSpi* pTraderSPi,
	CThostFtdcTradingAccountField* pMyAccount,
	CThostFtdcInstrumentCommissionRateField* pCommssion,
	CThostFtdcInstrumentMarginRateField *pMarginRate,
	CThostFtdcInstrumentField *pInstrument)
:futureApi(INSTRUMENT_ID,pMdSPi,pTraderSPi,pMyAccount,pCommssion,pMarginRate,pInstrument)
{
	
}


void strategy::process_depth_market_Data(CThostFtdcDepthMarketDataField *pQuote)
{
	
	
	
}


void strategy::process_order_accepted(CThostFtdcOrderField *pOrder)
{
	

}





void  strategy::process_order_execution(CThostFtdcOrderField *pOrder,CThostFtdcTradeField *pTrade)
{
		


}


void strategy::process_order_canceled(CThostFtdcOrderField *pOrder)
{
	
}


void strategy::process_order_rejected(CThostFtdcOrderField *pOrder)
{

}

