#pragma  once
#include "futureApi.h"
#include "Utilities.h"
#include "logger.h"
#include <iostream>
using namespace std;
//bool futureApi::m_bReqQryAccount=false;
//tested


fstream data;
int futureApi::m_iInstrument=0;
extern int numInstrument;
bool g_bExistPreOrder = true;
futureApi::futureApi(TThostFtdcInstrumentIDType INSTRUMENT_ID, CMdSpi* pMdSpi, CTraderSpi* pTraderSpi,
	CThostFtdcTradingAccountField* pMyAccount,
	CThostFtdcInstrumentCommissionRateField* pCommssion,
	CThostFtdcInstrumentMarginRateField *pMarginRate,
	CThostFtdcInstrumentField *pInstrument)
{
//  	data.open("data.txt", fstream::out | ios_base::trunc);
//  	data.close();
 	data.open("data.txt",ios::app);
	SYSTEMTIME stTime;
	GetLocalTime(&stTime);
	data<<"#################################"<<endl;
	data<<"new session start at: "<<stTime.wHour<<"."<<stTime.wMinute<<endl;
	data<<"#################################"<<endl<<endl;

	m_INSTRUMENT_ID=INSTRUMENT_ID;

	pMdSpi->quoteBaseVec.push_back(this);
	m_pMdSpi=pMdSpi;

	pTraderSpi->tradeBaseVec.push_back(this);
	m_pTraderSpi=pTraderSpi;

	memset(&m_last_quote,0,sizeof(struct CThostFtdcDepthMarketDataField));
	memset(&m_new_quote,0,sizeof(struct CThostFtdcDepthMarketDataField));

	set_account(pMyAccount);
	set_commissionRate(pCommssion);
	set_marginRate(pMarginRate);
	set_instrument_info(pInstrument);

	m_instrumentStatus=THOST_FTDC_IS_BeforeTrading;

	m_bIsLastReqPos=false;
	m_pTraderSpi->ReqQryInvestorPosition(m_INSTRUMENT_ID);
	while(!m_bIsLastReqPos)
	{
		Sleep(1000);

	}
	m_bIsLastReqOrder=false;

	m_pTraderSpi->ReqQryOrder(INSTRUMENT_ID);

	while(!m_bIsLastReqOrder)
	{
		Sleep(1000);
		if(	g_bExistPreOrder==false)
			break;
	}

	g_bExistPreOrder=true;

	
	cout<<m_INSTRUMENT_ID<<":constructed"<<endl;
	m_bConstructed=true;
}


void futureApi::set_instrument_info(CThostFtdcInstrumentField *pInstrument)
{
	set_instrument_info_for_position(pInstrument);
	//cout<<this->m_INSTRUMENT_ID<<endl;
	m_MaxLimitOrderVolume=pInstrument->MaxLimitOrderVolume;
	m_MaxMarginSideAlgorithm=pInstrument->MaxMarginSideAlgorithm;
	if(m_MaxMarginSideAlgorithm!='0' && m_MaxMarginSideAlgorithm!='1')
		m_MaxMarginSideAlgorithm='1';
	AsyncLOG(INFO, "set_instrument_info: sec=%s,tick=%.3lf,multi=%d,maxLmtVol=%d", pInstrument->InstrumentID, pInstrument->PriceTick, pInstrument->VolumeMultiple, pInstrument->MaxLimitOrderVolume);
}

void futureApi::on_quote_change(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
	m_new_quote=*pDepthMarketData;
	if(numInstrument == m_iInstrument)
	{
		update_long_position_profit(m_new_quote.LastPrice);
		update_short_position_profit(m_new_quote.LastPrice);

		process_depth_market_Data(pDepthMarketData);
	}

	m_last_quote=*pDepthMarketData;
}

void futureApi::on_order_execution(CThostFtdcTradeField *pTrade)
{
	// find the order info from m_orderVec
	/*vector<CThostFtdcOrderField>::iterator pOrder=find_order(pTrade->OrderRef,m_pTraderSpi->m_FRONT_ID,m_pTraderSpi->m_SESSION_ID);  //
	if(pOrder==m_orderVec.end())  //重新登录之后防止以前的订单无法查到
	{
		pOrder=find_order(pTrade->ExchangeID,pTrade->OrderSysID); 
	}
	if(pOrder==m_orderVec.end())return;*/

	// add new code
	OrderNode* pOrder = find_order(pTrade->OrderRef, m_pTraderSpi->m_FRONT_ID, m_pTraderSpi->m_SESSION_ID, 
		pTrade->ExchangeID, pTrade->OrderSysID);
	if (pOrder == NULL)
	{
		Log2DebugView("on_order_execution: not find sec=%s,ref=%s,sysId=%s,qt=%d\n", pTrade->InstrumentID, pTrade->OrderRef, pTrade->OrderSysID, pTrade->Volume);
		return;
	}

	// update volumes
	Log2DebugView("on_order_execution: sec=%s,ref=%s,oldTdQt=%d,oldTotal=%d,tradeQt=%d\n", pTrade->InstrumentID,
		pTrade->OrderRef, pOrder->order.VolumeTraded, pOrder->order.VolumeTotal, pTrade->Volume);
	pOrder->order.VolumeTraded	+= pTrade->Volume;
	pOrder->order.VolumeTotal	-= pTrade->Volume;
		
    CThostFtdcOrderField order = pOrder->order;
	if (pOrder->order.VolumeTotal < 0)
	{
		cout << "error volume" << endl;
	}
	else if (pOrder->order.VolumeTotal == 0)
		m_orders.remove_order(pOrder);

	//cout<<m_new_quote.LastPrice<<endl;
	update_position(pTrade, m_new_quote.LastPrice);	  
	

	process_order_execution(&order,pTrade);
	
	cout<<"exe:"<<endl;
	data<<"exe:"<<endl;
	m_orders.print_order(&order);
	m_orders.print_orders();
    print_position();
}
//tested
void futureApi::on_order_accepted(CThostFtdcOrderField *pOrder)
{
	/*vector<CThostFtdcOrderField>::iterator it=find_order(pOrder->OrderRef,m_pTraderSpi->m_FRONT_ID,m_pTraderSpi->m_SESSION_ID);  //
	if(it==m_orderVec.end())  //重新登录之后防止以前的订单无法查到
	{
		it=find_order(pOrder->ExchangeID,pOrder->OrderSysID); 
	}
	if(it==m_orderVec.end())return;*/

	OrderNode* pOrigOrd = find_order(pOrder->OrderRef, m_pTraderSpi->m_FRONT_ID, m_pTraderSpi->m_SESSION_ID, 
		pOrder->ExchangeID, pOrder->OrderSysID);
	if (pOrigOrd == NULL)
	{
		Log2DebugView("on_order_execution: not find sec=%s,ref=%s,sysId=%s,tdQt=%d,totalQt=%d\n", pOrder->InstrumentID, 
			pOrder->OrderRef, pOrder->OrderSysID, pOrder->VolumeTraded, pOrder->VolumeTotal);
		return;
	}
	CThostFtdcOrderField& ord = pOrigOrd->order;

	bool bAccepted=false;
	if(ord.OrderSubmitStatus == THOST_FTDC_OSS_Accepted && ord.OrderStatus == THOST_FTDC_OST_NoTradeQueueing)
		bAccepted=true;                                                                        
	
	m_orders.update_order_map(&pOrigOrd->order, Accepted);
	if(bAccepted==true)
		return;
	
	process_order_accepted(pOrder);
	cout<<"accept:"<<endl;
	data<<"accept:"<<endl;
	m_orders.print_order(pOrder);
	m_orders.print_orders();
	print_position();

}

void futureApi::on_order_rejected(CThostFtdcOrderField *pOrder)
{
	/*vector<CThostFtdcOrderField>::iterator it=find_order(pOrder->OrderRef,m_pTraderSpi->m_FRONT_ID,m_pTraderSpi->m_SESSION_ID);  //
	if(it==m_orderVec.end())  //重新登录之后防止以前的订单无法查到
	{
		it=find_order(pOrder->ExchangeID,pOrder->OrderSysID); 
	}*/
	
	OrderNode* pOrigOrd = find_order(pOrder->OrderRef, m_pTraderSpi->m_FRONT_ID, m_pTraderSpi->m_SESSION_ID, 
		pOrder->ExchangeID, pOrder->OrderSysID);
	if (pOrigOrd == NULL)
	{
		Log2DebugView("on_order_rejected: not find sec=%s,ref=%s,sysId=%s,tdQt=%d,totalQt=%d,dir=%c\n", 
			pOrder->InstrumentID, pOrder->OrderRef, pOrder->OrderSysID, pOrder->VolumeTraded, pOrder->VolumeTotal, pOrder->Direction);
		return;
	}
	CThostFtdcOrderField& ord = pOrigOrd->order;

	// 如果是平仓单，则重新计算本地的可平仓量
	if(ord.Direction == THOST_FTDC_D_Sell && is_offset_flag(ord.CombOffsetFlag[0]))
	{
		free_available_long_position_share(&pOrigOrd->order);
	}
	else if(pOrder->Direction == THOST_FTDC_D_Buy && is_offset_flag(ord.CombOffsetFlag[0]))
	{
		free_available_short_position_share(&pOrigOrd->order);
	}

	// remove this order
	m_orders.remove_order(pOrigOrd);

	process_order_rejected(pOrder);

	cout<<"rejected:"<<endl;
	data<<"rejected:"<<endl;
	m_orders.print_order(pOrder);
	m_orders.print_orders();
	print_position();
}

void futureApi::on_order_cancel_rejected(CThostFtdcOrderField *pOrder)
{
	Log2DebugView("on_order_cancel_rejected: do nothing for sec=%s,ref=%s,qt=%d,%d,dir=%s,offset=%s\n", 
		pOrder->InstrumentID, pOrder->OrderRef, pOrder->VolumeTotalOriginal, pOrder->VolumeTotal, 
		side_str(pOrder->Direction), offset_str(pOrder->CombOffsetFlag[0]));
}

//撤单有问题
void futureApi::on_order_cancel(CThostFtdcOrderField *pOrder)
{
	/*vector<CThostFtdcOrderField>::iterator it=find_order(pOrder->OrderRef,m_pTraderSpi->m_FRONT_ID,m_pTraderSpi->m_SESSION_ID);  //
	CThostFtdcOrderField order=*pOrder;
	if(it==m_orderVec.end())  //重新登录之后防止以前的订单无法查到
	{
		it=find_order(pOrder->ExchangeID,pOrder->OrderSysID); 
	}
	if(it==m_orderVec.end())return;*/
	Log2DebugView("on_order_cancel: sec=%s,ref=%s,sysId=%s,tdQt=%d,totalQt=%d,dir=%s\n", 
		pOrder->InstrumentID, pOrder->OrderRef, pOrder->OrderSysID, pOrder->VolumeTraded, pOrder->VolumeTotal, side_str(pOrder->Direction));
	
	OrderNode* pOrigOrd = find_order(pOrder->OrderRef, m_pTraderSpi->m_FRONT_ID, m_pTraderSpi->m_SESSION_ID, 
		pOrder->ExchangeID, pOrder->OrderSysID);
	if (pOrigOrd == NULL)
	{
		Log2DebugView("on_order_cancel: not find sec=%s,ref=%s\n", pOrder->InstrumentID, pOrder->OrderRef);
		return;
	}
	CThostFtdcOrderField& ord = pOrigOrd->order;

	CThostFtdcOrderField  tmpOrder = *pOrder;

	// 当是平仓单撤单的时候，更新本地仓位可平数量
	if(pOrder->Direction == THOST_FTDC_D_Sell && is_offset_flag(pOrder->CombOffsetFlag[0]))
	{
		free_available_long_position_share(&pOrigOrd->order);
	}
	else if(pOrder->Direction == THOST_FTDC_D_Buy && is_offset_flag(pOrder->CombOffsetFlag[0]))
	{
		free_available_short_position_share(&pOrigOrd->order);
	}

	// remove this order from order list
	m_orders.remove_order(pOrigOrd);

	process_order_canceled(&tmpOrder);


	cout<<"cancel:"<<endl;
	data<<"cancel:"<<endl;
	m_orders.print_order(pOrder);
	m_orders.print_orders();
	print_position();
}

//tested
void futureApi::on_order_submitted(CThostFtdcOrderField *pOrder)
{
	Log2DebugView("on_order_submitted: ref=%s,qt=%d\n", pOrder->OrderRef, pOrder->VolumeTotalOriginal);
	m_orders.update_order_map(pOrder, InsertSubmitted);
	cout<<"submitted:"<<endl;
	data<<"submitted:"<<endl;

	m_orders.print_order(pOrder);
	m_orders.print_orders();
	print_position();	
}

//////////////////////////////////////////////////////////////////////////
void futureApi::fill_comm_fields(CThostFtdcInputOrderField* pInputOrder, double price, int qt, char direction, char offset)
{
	pInputOrder->Direction			= direction;
	pInputOrder->CombOffsetFlag[0]	= offset;
	pInputOrder->LimitPrice			= price;
	pInputOrder->VolumeTotalOriginal= qt;
	strcpy_s(pInputOrder->InstrumentID, m_INSTRUMENT_ID);
}

//tested
int futureApi::send_open_buy_order(double limitPrice, int qt)
{
	CThostFtdcInputOrderField& theOrder = m_pTraderSpi->m_orderInsertReq;
	fill_comm_fields(&theOrder, limitPrice, qt, THOST_FTDC_D_Buy, THOST_FTDC_OF_Open);

	int iResult = send_new_order(&theOrder, qt);
	if( iResult != 0)
	{
		cout<<"发单不成功"<<endl;
	}
	return iResult;
}

//tested
int futureApi::send_open_sell_order(double limitPrice, int qt)
{
	CThostFtdcInputOrderField& theOrder = m_pTraderSpi->m_orderInsertReq;
	fill_comm_fields(&theOrder, limitPrice, qt, THOST_FTDC_D_Sell, THOST_FTDC_OF_Open);

	int iResult = send_new_order(&theOrder, qt);
	if( iResult != 0)
	{
		cout<<"网络问题发单不成功"<<endl;
	}
	return iResult;
}

//tested
int futureApi::send_close_buy_order(double limitPrice, int qt)
{
	CThostFtdcInputOrderField& theOrder = m_pTraderSpi->m_orderInsertReq;
	fill_comm_fields(&theOrder, limitPrice, qt, THOST_FTDC_D_Buy, THOST_FTDC_OF_Close);

	if(qt > get_available_short_position_share())
	{
		cout<<"可用仓位不足"<<endl;
		Log2DebugView("close_buy: qt=%d,avail=%d\n", qt, get_available_short_position_share());
		return -100;
	}

	int iResult;
	int yd = get_available_short_position_share_overnight();
	if(yd <= 0)       //若无昨仓
	{
		theOrder.CombOffsetFlag[0] = THOST_FTDC_OF_CloseToday;
		iResult = send_new_order(&theOrder, qt);
		if(iResult == 0)
		{
			// 更新本地的可平仓位量
			freeze_available_short_position_share_today(qt);
		}
	}else
	{
		OrderIdentifyKey orderKey;
		memset(&orderKey, 0, sizeof(orderKey));
		strcpy_s(orderKey.OrderRef, theOrder.OrderRef);
		orderKey.FrontID	= m_pTraderSpi->m_FRONT_ID;
		orderKey.SessionID	= m_pTraderSpi->m_SESSION_ID;

		if(qt <= yd)
		{
			theOrder.CombOffsetFlag[0] = THOST_FTDC_OF_Close;
			iResult = send_new_order(&theOrder, qt);
			if(iResult == 0)
			{
				double price = m_new_quote.LastPrice;
				freeze_available_short_position_share_overnight(qt, orderKey);
			}
		}
		else
		{
			theOrder.CombOffsetFlag[0] = THOST_FTDC_OF_Close;
			iResult = send_new_order(&theOrder, yd);
			if(iResult == 0)
			{
				double price=m_new_quote.LastPrice;
				freeze_available_short_position_share_overnight(yd, orderKey);
			}

			theOrder.CombOffsetFlag[0] = THOST_FTDC_OF_CloseToday;
			iResult = send_new_order(&theOrder, qt-yd);
			if(iResult == 0)
			{
				double price = m_new_quote.LastPrice;
				freeze_available_short_position_share_today(qt - yd);
			}
		}
	}
	return iResult;
}

//tested
int futureApi::send_close_sell_order(double limitPrice,int qt)  //是否需要加锁
{
	CThostFtdcInputOrderField& theOrder = m_pTraderSpi->m_orderInsertReq;
	fill_comm_fields(&theOrder, limitPrice, qt, THOST_FTDC_D_Sell, THOST_FTDC_OF_Close);

	if(qt > get_available_long_position_share())
	{
		cout<<"可用仓位不足"<<endl;
		Log2DebugView("send_close_sell_order: fail qt=%d,avail=%d\n", qt, get_available_long_position_share());
		return -100;
	}

	int iResult;
	int yd = get_available_long_position_share_overnight();
	if(yd <= 0)       //若无昨仓  not tested
	{
		theOrder.CombOffsetFlag[0] = THOST_FTDC_OF_CloseToday;
		iResult = send_new_order(&theOrder, qt);
		if(iResult == 0)
		{
			double price=m_new_quote.LastPrice;
			freeze_available_long_position_share_today(qt);
		}
	}else
	{
		OrderIdentifyKey orderKey;
		memset(&orderKey, 0, sizeof(orderKey));
		strcpy_s(orderKey.OrderRef, theOrder.OrderRef);
		orderKey.FrontID	= m_pTraderSpi->m_FRONT_ID;
		orderKey.SessionID	= m_pTraderSpi->m_SESSION_ID;

		if(qt <= yd)
		{
			theOrder.CombOffsetFlag[0] = THOST_FTDC_OF_Close;
			iResult = send_new_order(&theOrder, qt);
			if(iResult == 0)
			{
				double price=m_new_quote.LastPrice;
				freeze_available_long_position_share_overnight(qt, orderKey);
			}
		}else
		{
			theOrder.CombOffsetFlag[0] = THOST_FTDC_OF_Close;
			iResult = send_new_order(&theOrder, yd);
			if(iResult == 0)
			{
				double price=m_new_quote.LastPrice;
				freeze_available_long_position_share_overnight(yd, orderKey);
			}

			theOrder.CombOffsetFlag[0] = THOST_FTDC_OF_CloseToday;
			iResult = send_new_order(&theOrder, qt-yd);
			if(iResult == 0)
			{
				double price=m_new_quote.LastPrice;
				freeze_available_short_position_share_today(qt-yd);
			}
		}
	}

	return iResult;
}

//tested
int futureApi::send_new_order(CThostFtdcInputOrderField* pInputOrder, int qt)
{
	//if(pInputOrder->LimitPrice < m_new_quote.LowerLimitPrice-m_PriceTick/2 ) //tested低于跌停价  || price>m_new_quote.UpperLimitPrice
	if(pInputOrder->LimitPrice < m_new_quote.LowerLimitPrice - 0.0001 )
	{
		cout<<"发单价格低于跌停价"<<endl;
		Log2DebugView("send_new_order: failed insertprc=%.3lf,lowerlmtprc=%.3lf", pInputOrder->LimitPrice, m_new_quote.LowerLimitPrice);
		return -100;
	}

	//if(pInputOrder->LimitPrice > m_new_quote.UpperLimitPrice+m_PriceTick/2 ) //tested高于涨停价  || price>m_new_quote.UpperLimitPrice
	if(pInputOrder->LimitPrice > m_new_quote.UpperLimitPrice + 0.0001)
	{
		cout<<"发单价格高于涨停价"<<endl;
		Log2DebugView("send_new_order: failed insertprc=%.3lf, upperlmtprc=%.3lf", pInputOrder->LimitPrice, m_new_quote.UpperLimitPrice);
		return -101;
	}

	int iResult;
	int M = qt / m_MaxLimitOrderVolume;			// normally, M is 0
	int residual = qt % m_MaxLimitOrderVolume;	// mormally, residual is equal to qt

	for(int i = 0; i < M; ++i)                  // tested
	{	//tested
		//m_pTraderSpi->m_orderInsertReq.VolumeTotalOriginal=m_MaxLimitOrderVolume;
		pInputOrder->VolumeTotalOriginal = m_MaxLimitOrderVolume;
		iResult = enter_one_order(pInputOrder);
	}

	//m_pTraderSpi->m_orderInsertReq.VolumeTotalOriginal=residual;
	pInputOrder->VolumeTotalOriginal = residual;
	iResult = enter_one_order(pInputOrder);
	if(iResult != 0)
	{
		cout<<"发单不成功"<<endl;
		Log2DebugView("enter_one_order: fail sec=%s,qt=%d,ref=%s,dir=%s,offset=%s\n", pInputOrder->InstrumentID,
			pInputOrder->VolumeTotalOriginal, pInputOrder->OrderRef, side_str(pInputOrder->Direction), offset_str(pInputOrder->CombOffsetFlag[0]));
	}
	return iResult;
}

//tested
int futureApi::enter_one_order(CThostFtdcInputOrderField* pInputOrder)
{
	sprintf(pInputOrder->OrderRef, "%d", m_pTraderSpi->GetNextOrderRef());
	int iResult = m_pTraderSpi->ReqOrderInsert(pInputOrder, m_pTraderSpi->GetNextReqID());
	if( iResult == 0)
	{
		m_orders.save_order(pInputOrder, m_pTraderSpi->m_FRONT_ID, m_pTraderSpi->m_SESSION_ID);
	}

	return iResult;
}

//////////////////////////////////////////////////////////////////////////

//tested under real env
void futureApi::on_rsp_qry_Instrument(CThostFtdcInstrumentField *pInstrument)
{
}

//tested under real env
void futureApi::on_rsp_qry_instrument_commission_rate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate)
{
}

//tested
void futureApi::on_rsp_qry_instrument_margin_rate(CThostFtdcInstrumentMarginRateField *pMarginRate)
{
}

//tested
void futureApi::on_rsp_qry_position(CThostFtdcInvestorPositionField *pInvestorPosition)
{
	CThostFtdcInvestorPositionField* ppos = pInvestorPosition;
	Log2DebugView("OnRspQryPostion: sec=%s,dir=%s,pos=%d,tdPos=%d,ydPos=%d,openVol=%d,closeVol=%d,hist=%s,isLast=%d\n", ppos->InstrumentID,
		side_str(ppos->PosiDirection), ppos->Position, ppos->TodayPosition, ppos->YdPosition, ppos->OpenVolume, ppos->CloseVolume, ydpos_str(ppos->PositionDate), m_bIsLastReqPos);
	
	if(pInvestorPosition->PosiDirection == THOST_FTDC_PD_Long && pInvestorPosition->PositionDate == THOST_FTDC_PSD_History)
	{
		// 有问题，当昨仓被平掉后，是能查到昨仓是有多少手，但是实际可平量为0
		//m_overnight_long_position_share = pInvestorPosition->YdPosition;
		m_overnight_long_position_share = pInvestorPosition->Position;
		m_overnight_long_position_cost	= pInvestorPosition->OpenCost;
		m_long_close_profit				+=pInvestorPosition->CloseProfit;
		update_long_position_cost();

		m_ydLongMargin	= pInvestorPosition->UseMargin;
		m_longMargin	= m_currLongMargin + m_frozenLongMargin + m_ydLongMargin;
		AsyncLOG(INFO, "OnRspQryPostion: long sec=%s,ydpos=%d,ydpos_cost=%.3lf,rPnl=%.3lf\n", ppos->InstrumentID, 
			m_overnight_long_position_share, m_overnight_long_position_cost, m_long_close_profit);
	}
	else if(pInvestorPosition->PosiDirection == THOST_FTDC_PD_Long && pInvestorPosition->PositionDate==THOST_FTDC_PSD_Today)
	{
		m_currLongMargin	= pInvestorPosition->UseMargin;
		m_frozenLongMargin	= pInvestorPosition->FrozenMargin;
		m_longMargin		= m_currLongMargin + m_frozenLongMargin + m_ydLongMargin;

		m_today_long_position_share = pInvestorPosition->TodayPosition;    //可能有问题
		m_today_long_position_cost	= pInvestorPosition->OpenCost;
		m_long_close_profit			+=pInvestorPosition->CloseProfit;
		update_long_position_cost();

		//update_long_position_profit(m_new_quote.LastPrice);
		AsyncLOG(INFO, "OnRspQryPostion: long sec=%s,pos=%d,pos_cost=%.3lf,rPnl=%.3lf\n", ppos->InstrumentID, 
			m_today_long_position_share, m_long_position_cost, m_long_close_profit);
	}
	else if(pInvestorPosition->PosiDirection == THOST_FTDC_PD_Short && pInvestorPosition->PositionDate == THOST_FTDC_PSD_History)
	{
		m_ydShortMargin	= pInvestorPosition->UseMargin;
		m_shortMargin	= m_currShortMargin + m_frozenShortMargin + m_ydShortMargin;
		m_new_quote.PreSettlementPrice = pInvestorPosition->PreSettlementPrice;

		//m_overnight_short_position_share= pInvestorPosition->YdPosition;		
		m_overnight_short_position_share= pInvestorPosition->Position;		
		m_overnight_short_position_cost	= pInvestorPosition->OpenCost;
		m_short_close_profit			+=pInvestorPosition->CloseProfit;
		update_short_position_cost();

		//update_long_position_profit(m_new_quote.LastPrice);
		AsyncLOG(INFO, "OnRspQryPostion: short sec=%s,pos=%d,ydpos_cost=%.3lf,rPnl=%.3lf\n", ppos->InstrumentID, 
			m_overnight_long_position_share, m_overnight_short_position_cost, m_short_close_profit);
	}
	else if(pInvestorPosition->PosiDirection == THOST_FTDC_PD_Short && pInvestorPosition->PositionDate == THOST_FTDC_PSD_Today)
	{
		m_currShortMargin	= pInvestorPosition->UseMargin;
		m_frozenShortMargin	= pInvestorPosition->FrozenMargin;
		m_shortMargin		= m_currShortMargin + m_frozenShortMargin + m_ydShortMargin;

		m_today_short_position_share= pInvestorPosition->TodayPosition;
		m_today_short_position_cost	= pInvestorPosition->OpenCost;
		m_short_close_profit		+=pInvestorPosition->CloseProfit;
		update_short_position_cost();
		AsyncLOG(INFO, "OnRspQryPostion: short sec=%s,pos=%d,pos_cost=%.3lf,rPnl=%.3lf\n", ppos->InstrumentID, 
			m_today_short_position_share, m_short_position_cost, m_short_close_profit);
	}
	//tested
	if(m_bIsLastReqPos)
	{
		update_total_profit();

		m_long_position_share	= m_today_long_position_share  + m_overnight_long_position_share;
		m_short_position_share	= m_today_short_position_share + m_overnight_short_position_share;

		// it will be updated when query order info
		m_available_today_long_position_share		= m_today_long_position_share;
		m_available_today_short_position_share		= m_today_short_position_share;
		m_available_overnight_long_position_share	= m_overnight_long_position_share;
		m_available_overnight_short_position_share	= m_overnight_short_position_share;

		//m_available_long_position_share=m_available_today_long_position_share+m_available_overnight_long_position_share;
		//m_available_short_position_share=m_available_today_short_position_share+m_available_overnight_short_position_share;
		update_available_long_position_share();
		update_available_short_position_share();

		if(m_long_position_share > 0)
		{
			detailCost costLong;
			costLong.volume	= m_long_position_share;
			costLong.price	= m_long_position_cost/(m_long_position_share+1e-20)/m_VolumeMultiple;
			m_costQueueLongPosition.push(costLong);
		}

		if(m_short_position_share > 0)
		{
			detailCost costShort;
			costShort.volume = m_short_position_share;
			costShort.price  = m_short_position_cost/(m_short_position_share+1e-20)/m_VolumeMultiple;;
			m_costQueueShortPosition.push(costShort);
		}
		Log2DebugView("OnRspQryPostion[last]: sec=%s,long_cost=%.3lf,short_cost=%.3lf, longPnl=%.3lf,shortPnl=%.3lf,total_profit=%.3lf\n",
			ppos->InstrumentID, m_long_position_cost, m_short_position_cost, m_long_close_profit, m_short_close_profit, m_totalProfit);


// 		if(m_MaxMarginSideAlgorithm==THOST_FTDC_MMSA_NO)
// 		{		
// 			m_margin=m_longMargin+m_shortMargin;	
// 			m_currMargin=m_currLongMargin+m_currShortMargin;
// 			m_frozenMargin=m_frozenLongMargin+m_frozenShortMargin;
// 		}else
// 		{
// 			m_margin=(m_longMargin>m_shortMargin)?m_longMargin:m_shortMargin;   //tested
// 			m_currMargin=(m_currLongMargin>m_currShortMargin)?m_currLongMargin:m_currShortMargin;
// 			m_frozenMargin=(m_frozenLongMargin>m_frozenShortMargin)?m_frozenLongMargin:m_frozenShortMargin;
// 		}
// 	
		//cout<<this->m_INSTRUMENT_ID<<":init"<<endl;
		
	}
	

}

//tested
void futureApi::on_rsp_qry_instrument_trading_account(CThostFtdcTradingAccountField *pTradingAccount) //只在初始化时使用 
{

	
}

void futureApi::on_rsp_qry_order(CThostFtdcOrderField *pOrder,bool bIsLast)
{
	// 如果该单还有挂单量，且如果是平仓单，则更新本地可平仓位量
	if(pOrder->VolumeTraded < pOrder->VolumeTotalOriginal 
		&& pOrder->OrderStatus != THOST_FTDC_OST_Canceled
		&& pOrder->OrderStatus != THOST_FTDC_OST_Unknown)
	{
		Log2DebugView("on_rsp_qry_order: sec=%s,dir=%s,offset=%s,volOri=%d,volTd=%d", pOrder->InstrumentID, side_str(pOrder->Direction),
			offset_str(pOrder->CombOffsetFlag[0]),pOrder->VolumeTotalOriginal, pOrder->VolumeTraded);
		m_orders.save_order(pOrder);
		if(pOrder->Direction == THOST_FTDC_D_Sell)
		{
			/*if(pOrder->VolumeTotal<*/  //大商所 大商所如何处理
			if(pOrder->CombOffsetFlag[0] == THOST_FTDC_OF_CloseToday)
			{
				//m_available_today_long_position_share-=pOrder->VolumeTotal;
				update_avail_today_long_position_share(pOrder->VolumeTotal, position::POS_OP_DEC);
			}
			else if(pOrder->CombOffsetFlag[0] == THOST_FTDC_OF_Close)
			{
				/*if(m_available_overnight_long_position_share>pOrder->VolumeTotal)
				{
					m_available_overnight_long_position_share-=pOrder->VolumeTotal;
				}else
				{
					m_available_today_long_position_share-=(pOrder->VolumeTotal-m_available_overnight_long_position_share);
					m_available_overnight_long_position_share=0;
				}*/
				if (get_available_long_position_share_overnight() > pOrder->VolumeTotal)
				{
					update_avail_overnight_long_position_share(pOrder->VolumeTotal, position::POS_OP_DEC);
				}
				else
				{
					int vol = pOrder->VolumeTotal - get_available_long_position_share_overnight();
					update_avail_today_long_position_share(vol, position::POS_OP_DEC);
					update_avail_overnight_long_position_share(0, position::POS_OP_ASSIGN);
				}
			}
		}//if Direction	sell
		else
		{
			if(pOrder->CombOffsetFlag[0] == THOST_FTDC_OF_CloseToday)
			{
				//m_available_today_short_position_share-=pOrder->VolumeTotal;
				update_avail_today_short_position_share(pOrder->VolumeTotal, position::POS_OP_DEC);
			}
			else if(pOrder->CombOffsetFlag[0] == THOST_FTDC_OF_Close)
			{
				/*if(m_available_overnight_short_position_share>pOrder->VolumeTotal)
				{
					m_available_overnight_short_position_share-=pOrder->VolumeTotal;
				}else
				{
					m_available_today_short_position_share-=(pOrder->VolumeTotal-m_available_overnight_short_position_share);
					m_available_overnight_short_position_share=0;
				}*/
				if (get_available_short_position_share_overnight() > pOrder->VolumeTotal)
				{
					update_avail_overnight_short_position_share(pOrder->VolumeTotal, position::POS_OP_DEC);
				}
				else
				{
					int vol = pOrder->VolumeTotal - get_available_short_position_share_overnight();
					update_avail_today_short_position_share(vol, position::POS_OP_DEC);
					update_avail_overnight_short_position_share(0, position::POS_OP_ASSIGN);
				}
			}
		}//else Direction buy
	}//if volume
	
	m_bIsLastReqOrder=bIsLast;
	if(m_bIsLastReqOrder)
	{
		//m_available_long_position_share=m_available_today_long_position_share+m_available_overnight_long_position_share;
		//m_available_short_position_share=m_available_today_short_position_share+m_available_overnight_short_position_share;
		update_available_long_position_share();
		update_available_short_position_share();
		Log2DebugView("on_rsp_qry_order: avail_long_pos=%d, avail_short_pos=%d\n", m_available_long_position_share, m_available_short_position_share);
		
		m_orders.print_orders();
		print_position();
	}
}


//////////////////////////////////////////////////////////////////////////
void futureApi::cancel_one_order(CThostFtdcOrderField *pOrder)
{
	m_pTraderSpi->ReqOrderAction(pOrder);
}

int futureApi::send_buy_order(double price, int qt)
{
	MutexGuard lk(m_posLock);
	int iResult=-100;
	int availQt = get_available_short_position_share();
	if(availQt > 0)
	{
		if(qt > availQt)
		{
			iResult = send_close_buy_order(price, availQt);
			iResult = send_open_buy_order(price, qt - availQt);
		}else
		{
			iResult = send_close_buy_order(price, qt);
		}
	}
	else
	{
		iResult = send_open_buy_order(price, qt);
	}

	return iResult;
}

int futureApi::send_sell_order(double price,int qt)
{
	MutexGuard lk(m_posLock);
	int iResult = -100;
	int availQt = get_available_long_position_share();
	if(availQt  > 0)
	{
		if(qt > availQt)
		{
			iResult = send_close_sell_order(price, availQt);
			iResult = send_open_sell_order(price, qt - availQt);

		}else
		{
			iResult = send_close_sell_order(price, qt);
		}
	}else
	{
		iResult = send_open_sell_order(price, qt);
	}

	return iResult;
}

int futureApi::send_buy_fok(double price, int qt)
{
	MutexGuard lk(m_posLock);
	
 	m_pTraderSpi->m_orderInsertReq.TimeCondition	= THOST_FTDC_TC_IOC;
 	m_pTraderSpi->m_orderInsertReq.VolumeCondition	= THOST_FTDC_VC_CV;
	int iResult= send_buy_order(price, qt);

	// re-modify back the order condition
	m_pTraderSpi->m_orderInsertReq.TimeCondition	= THOST_FTDC_TC_GFD;
	m_pTraderSpi->m_orderInsertReq.VolumeCondition	= THOST_FTDC_VC_AV;
	return iResult;
}

int futureApi::send_sell_fok(double price, int qt)
{
	MutexGuard lk(m_posLock);
	m_pTraderSpi->m_orderInsertReq.TimeCondition=THOST_FTDC_TC_IOC;
	m_pTraderSpi->m_orderInsertReq.VolumeCondition=THOST_FTDC_VC_CV;

	int iResult= send_sell_order(price, qt);

	m_pTraderSpi->m_orderInsertReq.TimeCondition=THOST_FTDC_TC_GFD;
	m_pTraderSpi->m_orderInsertReq.VolumeCondition=THOST_FTDC_VC_AV;
	return iResult;
}

int futureApi::send_open_buy_park(double price,int qt)
{
	m_pTraderSpi->m_orderParkReq.Direction			= THOST_FTDC_D_Buy;
	m_pTraderSpi->m_orderParkReq.CombOffsetFlag[0]	= THOST_FTDC_OF_Open;
	m_pTraderSpi->m_orderParkReq.LimitPrice			= price;
	m_pTraderSpi->m_orderParkReq.VolumeTotalOriginal= qt;
	strcpy_s(m_pTraderSpi->m_orderParkReq.InstrumentID, m_INSTRUMENT_ID);

	int iResult = send_parked_new_order(price,qt);
	if(iResult == 0)
	{
		//
	}
	return iResult;
}

// 
int futureApi::send_parked_new_order(double price,int qt)
{
	if(price < m_new_quote.LowerLimitPrice - 0.0001) //超过涨跌停价  || price>m_new_quote.UpperLimitPrice
	{
		return -100;
	}

	int iResult;
	int M = qt / m_MaxLimitOrderVolume;                     
	int residual = qt % m_MaxLimitOrderVolume;
	for(int i = 0;i < M; ++i)                    //大单分撤小单
	{
		m_pTraderSpi->m_orderParkReq.VolumeTotalOriginal = m_MaxLimitOrderVolume;
		iResult=enter_parked_one_order();
	}
	m_pTraderSpi->m_orderParkReq.VolumeTotalOriginal = residual;
	iResult = enter_parked_one_order();
	return iResult;
}

int futureApi::enter_parked_one_order()
{
	sprintf_s(m_pTraderSpi->m_orderParkReq.OrderRef, "%d", m_pTraderSpi->GetNextOrderRef());
	int iResult = m_pTraderSpi->ReqParkedOrderInsert(&m_pTraderSpi->m_orderParkReq, m_pTraderSpi->GetNextReqID());
	if(iResult == 0)
	{
		//TODO: save the parked order
		//create_new_order(&m_pTraderSpi->m_orderParkReq,m_pTraderSpi->m_FRONT_ID,m_pTraderSpi->m_SESSION_ID);
	}
	
	return iResult;
}

//////////////////////////////////////////////////////////////////////////
void futureApi::print_position()
{
	// just log today positions
	int longPos		= get_long_position_share();
	int todayPos	= get_today_long_position_share();
	int availPos	= get_available_long_position_share();
	int availTdPos	= get_available_long_position_share_today();
	Log2DebugView("postion_long: pos=%d,todayPos=%d,availPos=%d,availTdPos=%d\n", longPos, todayPos, availPos, availTdPos);
	int shortPos	= get_short_position_share();
	int tdShortPos	= get_today_short_position_share();
	int avShortPos	= get_available_short_position_share();
	int avTdShortPos= get_available_short_position_share_today();
	Log2DebugView("postion_short: pos=%d,todayPos=%d,availPos=%d,availTdPos=%d\n", shortPos, tdShortPos, avShortPos, avTdShortPos);

	cout<<endl<<"long position share today:"<<get_today_long_position_share()<<endl
		<<"available long position share today:"<<get_available_long_position_share_today()<<endl
		<<"long position share overnight:"<<get_overnight_long_position_share()<<endl
		<<"available long position share overnight:"<<get_available_long_position_share_overnight()<<endl
		<<"long position cost:"<<setw(20)<<get_long_position_cost() <<endl
		<<"long position close profit:"<<get_long_close_profit()<<endl
		<<"long position profit:"<<get_long_position_profit()<<endl<<endl


		<<"short position share today:"<<get_today_short_position_share()<<endl
		<<"available short position share today:"<<get_available_short_position_share_today()<<endl
		<<"short position share overnight:"<<get_overnight_short_position_share()<<endl
		<<"available short position share overnight:"<<get_available_short_position_share_overnight()<<endl
		<<"short position cost:"<<setw(20)<<get_short_position_cost() <<endl
		<<"short position close profit:"<<get_short_close_profit()<<endl
		<<"short position profit:"<<get_short_position_profit()<<endl
		<<endl<<endl<<endl;


	data<<endl<<"long position share today:"<<get_today_long_position_share()<<endl
		<<"available long position share today:"<<get_available_long_position_share_today()<<endl
		<<"long position share overnight:"<<get_overnight_long_position_share()<<endl
		<<"available long position share overnight:"<<get_available_long_position_share_overnight()<<endl
		<<"long position cost:"<<get_long_position_cost() <<endl
		<<"long position close profit:"<<get_long_close_profit()<<endl
		<<"long position profit:"<<get_long_position_profit()<<endl<<endl


		<<"short position share today:"<<get_today_short_position_share()<<endl
		<<"available short position share today:"<<get_available_short_position_share_today()<<endl
		<<"short position share overnight:"<<get_overnight_short_position_share()<<endl
		<<"available short position share overnight:"<<get_available_short_position_share_overnight()<<endl
		<<"short position cost:"<<get_short_position_cost() <<endl
		<<"short position close profit:"<<get_short_close_profit()<<endl
		<<"short position profit:"<<get_short_position_profit()<<endl
		<<endl<<endl<<endl;
}

void futureApi::print_order(CThostFtdcOrderField *pOrder)
{
	m_orders.print_order(pOrder);
	cout<<setw(9)<<setiosflags(ios::left)<<pOrder->InsertTime<<" "
		<<setw(9)<<setiosflags(ios::left)<<pOrder->ExchangeInstID<<" "
		<<setw(9)<<setiosflags(ios::left)<<pOrder->FrontID<<" "
		<<setw(9)<<setiosflags(ios::left)<<pOrder->SessionID<<" "
		<<setw(13)<<setiosflags(ios::left)<<pOrder->OrderRef<<" "
		<<setw(9)<<setiosflags(ios::left)<<pOrder->ExchangeID<<" "
		<<setw(9)<<setiosflags(ios::left)<<pOrder->TraderID<<" "
		<<setw(21)<<setiosflags(ios::left)<<pOrder->OrderSysID<<" "
		<<setw(9)<<setiosflags(ios::left)<<pOrder->OrderLocalID<<" ";
	if(pOrder->CombOffsetFlag[0]==THOST_FTDC_OF_Open)
	{
		cout<<"open  ";
	}else
	{
		cout<<"close ";
	}


	if(pOrder->Direction==THOST_FTDC_D_Buy)
	{
		cout<<"buy  ";
	}else
	{
		cout<<"sell ";
	}

	cout<<setw(6)<<pOrder->LimitPrice<<" "
		<<setw(6)<<pOrder->VolumeTotalOriginal<<" "
		<<setw(6)<<pOrder->VolumeTotal<<endl;

	data<<setw(9)<<setiosflags(ios::left)<<pOrder->InsertTime<<" "
		<<setw(9)<<setiosflags(ios::left)<<pOrder->ExchangeInstID<<" "
		<<setw(9)<<setiosflags(ios::left)<<pOrder->FrontID<<" "
		<<setw(9)<<setiosflags(ios::left)<<pOrder->SessionID<<" "
		<<setw(13)<<setiosflags(ios::left)<<pOrder->OrderRef<<" "
		<<setw(9)<<setiosflags(ios::left)<<pOrder->ExchangeID<<" "
		<<setw(9)<<setiosflags(ios::left)<<pOrder->TraderID<<" "
		<<setw(21)<<setiosflags(ios::left)<<pOrder->OrderSysID<<" "
		<<setw(9)<<setiosflags(ios::left)<<pOrder->OrderLocalID<<" ";
	if(pOrder->CombOffsetFlag[0]==THOST_FTDC_OF_Open)
	{
		data<<"open  ";
	}else
	{
		data<<"close ";
	}


	if(pOrder->Direction==THOST_FTDC_D_Buy)
	{
		data<<"buy  ";
	}else
	{
		data<<"sell ";
	}

	data<<setw(6)<<pOrder->LimitPrice<<" "
		<<setw(6)<<pOrder->VolumeTotalOriginal<<" "
		<<setw(6)<<pOrder->VolumeTotal<<endl;
}

OrderNode* futureApi::find_order(TThostFtdcOrderRefType orderRef, TThostFtdcFrontIDType FrontID, TThostFtdcSessionIDType SessionID, 
	TThostFtdcExchangeIDType ExchangeID, TThostFtdcOrderSysIDType orderSysID)
{
	OrderNode* pOrder = NULL;
	if ((pOrder = m_orders.find_order(orderRef, FrontID, SessionID)) == NULL)
	{
		pOrder = m_orders.find_order(ExchangeID, orderSysID);
	}
	return pOrder;
}
