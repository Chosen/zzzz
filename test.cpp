#include "strategy.h"
#include "conio.h"
#include <stdio.h>
#include <iostream>
using namespace std;
#include "logger.h"

CThostFtdcTradingAccountField myAccount;
bool bLogInTrader=false;
bool bRspQryAccount;		// indicate where account querying was returned
int rspQryCommision;
int rspQryMarginRate;		// indicate how many instruments were queried done
int rspQryInstrument;
int rspQryPosition;
bool bLogMd=false;
vector<CThostFtdcInstrumentCommissionRateField> commisionRate;
vector<CThostFtdcInstrumentMarginRateField> marginRate;
vector<CThostFtdcInstrumentField> instrument;
vector<CThostFtdcInvestorPositionField> insturmentPosition;
//vector<>
char* ppInstrumentID[]={"rb1510"};
int numInstrument=sizeof(ppInstrumentID)/4;

void TradeTest(strategy* pApi);

void main()
{	
	COORD size = {201, 10001};

	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE); // 获取标准输出设备句柄
	SetConsoleScreenBufferSize(hOut,size);
	
	
	SMALL_RECT rc = {0,0, 200, 10000}; // 重置窗口位置和大小
	SetConsoleScreenBufferSize(hOut,size);
	SetConsoleWindowInfo(hOut,true ,&rc);
	CreateLOG("strategy_trade.log", Debugview);
	SyncLOG(INFO, "\n==========app begin running==========\n");
	AsyncLOG(INFO, "\n==========app begin running==========\n");
	
	config configMd,configTrade;

	char brokerID[32] = "0069";
	char investID[32] = "00037";
	char password[32] = "qwert";
#if 1
	strcpy_s(configTrade.Broker_ID, brokerID);
	strcpy_s(configTrade.FRONT_ADDR,"tcp://180.168.146.181:10000");
	strcpy_s(configTrade.InvesterID, investID);
	strcpy_s(configTrade.PASSWORD, password);
	/*strcpy_s(configTrade.Broker_ID,"0275");
	strcpy_s(configTrade.FRONT_ADDR,"tcp://180.168.146.181:10200");
	strcpy_s(configTrade.InvesterID,"00011");
	strcpy_s(configTrade.PASSWORD,"123456");*/
#else
	strcpy_s(configTrade.FRONT_ADDR, "tcp://203.187.171.250:10910");
	strcpy_s(configTrade.InvesterID, "000100000007");
	strcpy_s(configTrade.PASSWORD,"123");
#endif
	//strcpy_s(configTrade.InstrumentID,ppInstrumentID[0]);

#if 1
	strcpy_s(configMd.Broker_ID, brokerID);
	strcpy_s(configMd.FRONT_ADDR,"tcp://180.168.146.181:10100");
	strcpy_s(configMd.InvesterID, investID);
	strcpy_s(configMd.PASSWORD, password);
	/*strcpy_s(configMd.Broker_ID,"0275");
	strcpy_s(configMd.FRONT_ADDR,"tcp://180.168.146.181:10210");
	strcpy_s(configMd.InvesterID,"00011");
	strcpy_s(configMd.PASSWORD,"123456");*/
#else
	strcpy_s(configMd.FRONT_ADDR,"tcp://203.187.171.250:10915");
	strcpy_s(configMd.InvesterID,"000100000001");
	strcpy_s(configMd.PASSWORD,"123");
#endif

	CThostFtdcTraderApi* pTraderApi = CThostFtdcTraderApi::CreateFtdcTraderApi();			// 创建UserApi
	CTraderSpi* pTraderSpi = new CTraderSpi(configTrade,pTraderApi);
	pTraderApi->RegisterSpi((CThostFtdcTraderSpi*)pTraderSpi);			// 注册事件类
	pTraderApi->SubscribePublicTopic(THOST_TERT_QUICK);					// 注册公有流
	pTraderApi->SubscribePrivateTopic(THOST_TERT_QUICK);					// 注册私有流
	pTraderApi->RegisterFront(configTrade.FRONT_ADDR);							// connect
	pTraderApi->Init();
	while(bLogInTrader==false)
	{
		Sleep(300);
	}


	bRspQryAccount=false;
	pTraderSpi->ReqQryAccount();
	while(bRspQryAccount==false)
	{
		Sleep(300);
	}

	int k = 0;
	rspQryCommision=0;
	for(k = 0; k < numInstrument; ++k)
	{
		pTraderSpi->ReqQryInstrumentCommissionRate(ppInstrumentID[k]);
		Sleep(300);
	}
	
	while(rspQryCommision<numInstrument)
	{
		Sleep(300);
	}

	rspQryMarginRate=0;
	for(k = 0; k < numInstrument; ++k)
	{
		pTraderSpi->ReqQryInstrumentMarginRate(ppInstrumentID[k]);
	}

	while(rspQryMarginRate<numInstrument)
	{
		Sleep(100);
	}

	rspQryInstrument=0;
	for(k = 0; k < numInstrument; ++k)
	{
		pTraderSpi->ReqQryInstrument(ppInstrumentID[k]);
	}

	while(rspQryInstrument<numInstrument)
	{
		Sleep(100);
	}


	//strcpy_s(configMd.InstrumentID,ppInstrumentID[0]);

	CThostFtdcMdApi* pMdApi = CThostFtdcMdApi::CreateFtdcMdApi();			// 创建UserApi
	CThostFtdcMdSpi* pMdSpi = new CMdSpi(ppInstrumentID,numInstrument,configMd,pMdApi);
	pMdApi->RegisterSpi(pMdSpi);						// 注册事件类
	pMdApi->RegisterFront(configMd.FRONT_ADDR);					// connect
	pMdApi->Init();
	while(bLogMd==false)
	{
		Sleep(100);
	}
	
	vector<strategy*> futures(numInstrument);
// 
	for(int i = 0; i < numInstrument; ++i)
	{
		futures[i]=new strategy(ppInstrumentID[i],
			(CMdSpi*)pMdSpi,pTraderSpi,
			&myAccount,
			&commisionRate[i],
			&marginRate[i],
			&instrument[i]);
	}

	futures[0]->get_mdSpi()->SubscribeMarketData();
	Sleep(1000);

	TradeTest(futures[0]);

	pMdApi->Join();
	//while (_kbhit()==0); 
	
}

void TradeTest(strategy* pApi)
{
	int cmd = 0;

	CThostFtdcOrderField cancel;
	memset(&cancel, 0, sizeof(cancel));
	strcpy_s(cancel.BrokerID, "");
	strcpy_s(cancel.InvestorID, "");
	strcpy_s(cancel.InstrumentID, ppInstrumentID[0]);

	TThostFtdcDirectionType dir;		// 0-buy, 1-sell
	TThostFtdcCombOffsetFlagType kpp;	// 0-open,1-close
	TThostFtdcPriceType price;
	TThostFtdcVolumeType vol;
	//TThostFtdcSequenceNoType orderSeq;
	TThostFtdcOrderRefType orderref;
	int isCancelOrder = 0;

	do 
	{
		printf("#print cmd: ");
		std::cin >> cmd;
		if (cmd == 999)
		{
			printf("while break!\n");
			break;
		}

		cerr<<" 是否撤单 > "; std::cin >> isCancelOrder;
		if (isCancelOrder == 1)
		{
			cerr<<" OrderRef  > "; 
			std::cin >> orderref;
			strcpy_s(cancel.OrderRef, orderref);
			pApi->get_traderSpi()->ReqOrderAction(&cancel);
			continue;
		}

		cerr<<" 方向 > "; std::cin >> dir; 
		cerr<<" 开平 > "; std::cin >> kpp;
		cerr<<" 价格 > "; std::cin >> price;
		cerr<<" 数量 > "; std::cin >> vol;

		// 买开
		if (dir == THOST_FTDC_D_Buy && kpp[0] == THOST_FTDC_OF_Open)
		{
			pApi->send_open_buy_order(price, vol);  //1
			continue;
		}

		// 卖开
		if (dir == THOST_FTDC_D_Sell && kpp[0] == THOST_FTDC_OF_Open)
		{
			pApi->send_open_sell_order(price, vol);  //1
			continue;
		}

		// 买平
		if (dir == THOST_FTDC_D_Buy && kpp[0] != THOST_FTDC_OF_Open)
		{
			pApi->send_close_buy_order(price, vol);  //1
			continue;
		}

		// 卖平
		if (dir == THOST_FTDC_D_Sell && kpp[0] != THOST_FTDC_OF_Open)
		{
			pApi->send_close_sell_order(price, vol);  //1
			continue;
		}

	}while (1);

}

// strcpy_s(configMd.Broker_ID,"4200");
// strcpy_s(configMd.FRONT_ADDR,"tcp://180.166.11.33:41213");
// strcpy_s(configMd.InvesterID,"00000056");
//strcpy_s(configMd.PASSWORD,"123456");
//strcpy_s(configTrade.Broker_ID,"1023");
//strcpy_s(configTrade.FRONT_ADDR,"tcp://116.236.213.178:21205");
//strcpy_s(configTrade.InvesterID,"00000056");
//strcpy_s(configTrade.PASSWORD,"123456");