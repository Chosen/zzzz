#include "order.h"
#include "Utilities.h"

#if 0
//tested
void order::create_new_order(CThostFtdcInputOrderField *orderInsertReq, TThostFtdcFrontIDType FrontID, TThostFtdcSessionIDType SessionID)
{
	CThostFtdcOrderField order;
	memset(&order,0,sizeof(order));
	////经济公司代码
	strcpy_s(order.BrokerID,orderInsertReq->BrokerID);
	////投资者代码
	strcpy_s(order.InvestorID,orderInsertReq->InvestorID);

	///单价格条件
	order.OrderPriceType=THOST_FTDC_OPT_LimitPrice;
	////组合投机套标志
	order.CombHedgeFlag[0]=THOST_FTDC_HF_Speculation;
	////有效期
	order.TimeCondition=THOST_FTDC_TC_GFD;
	////成交类型
	order.VolumeCondition=THOST_FTDC_VC_AV;
	////最小成交量
	order.MinVolume=1;
	////触发条件
	order.ContingentCondition=THOST_FTDC_CC_Immediately;
	/////强平原因
	order.ForceCloseReason=THOST_FTDC_FCC_NotForceClose;
	////强平标志
	order.UserForceClose=0;

	order.Direction=orderInsertReq->Direction;
	order.CombOffsetFlag[0]=orderInsertReq->CombOffsetFlag[0];
	order.LimitPrice=orderInsertReq->LimitPrice;
	order.VolumeTotalOriginal=orderInsertReq->VolumeTotalOriginal;
	strcpy_s(order.InstrumentID,orderInsertReq->InstrumentID);
	strcpy_s(order.OrderRef,orderInsertReq->OrderRef);
	order.SessionID=SessionID;
	order.FrontID=FrontID;

	MutexGuard lk(m_orderMtx);
	m_orderVec.push_back(order);
}  

void order::remove_order(OrderListIterator it)
{
	MutexGuard lk(m_orderMtx);
	m_orderVec.erase(it);    //可能有安全隐患
}

void order::update_order_map(CThostFtdcOrderField *pOrder, orderStatus status)
{
	OrderListIterator it = find_order(pOrder);
	if(it==m_orderVec.end())return;
	if(Accepted==status)
	{
		update_order_status(it,pOrder);//tested

	}else if(InsertRejected==status)
	{
		remove_order(it);

	}else if(AllFilled==status)
	{
		remove_order(it);//tested

	}else if(InsertSubmitted==status)
	{
		update_order_status(it,pOrder);//tested
	}else if(Canceled==status)
	{
		remove_order(it);
	}
}

//tested
OrderListIterator order::find_order(CThostFtdcOrderField *pOrder)
{
	OrderListIterator it;
	for(it=m_orderVec.begin();it!=m_orderVec.end();++it)
	{
		if(    pOrder->FrontID==it->FrontID
			&& pOrder->SessionID==it->SessionID
			&& strcmp(pOrder->OrderRef,it->OrderRef)==0)
		{
			break;
		}
	}
	if(it!=m_orderVec.end())return it;
	for(it=m_orderVec.begin();it!=m_orderVec.end();++it)
	{
		if(    strcmp(pOrder->ExchangeID,it->ExchangeID)==0
			&& strcmp(pOrder->OrderSysID,it->OrderSysID)==0)
		{
			break;
		}
	}
	if(it!=m_orderVec.end())return it;
	for(it=m_orderVec.begin();it!=m_orderVec.end();++it)
	{
		if(     strcmp(pOrder->ExchangeID,it->ExchangeID)==0
			&& strcmp(pOrder->TraderID,it->TraderID)==0
			&&	strcmp(pOrder->OrderLocalID,it->OrderLocalID)==0
		  )
		{
			break;
		}
	}
	return it;
}

OrderListIterator order::find_order(TThostFtdcOrderRefType orderRef, TThostFtdcFrontIDType FrontID, TThostFtdcSessionIDType SessionID)
{
	OrderListIterator it;
	for(it=m_orderVec.begin();it!=m_orderVec.end();++it)
	{
		if(    FrontID==it->FrontID
			&& SessionID==it->SessionID
			&& strcmp(orderRef,it->OrderRef)==0)
		{
			break;
		}
	}
	return it;
}

//
OrderListIterator order::find_order(TThostFtdcExchangeIDType ExchangeID, TThostFtdcOrderSysIDType OrderSysID)
{
	OrderListIterator it;
	for(it=m_orderVec.begin();it!=m_orderVec.end();++it)
	{
		if(strcmp(ExchangeID,it->ExchangeID)==0
			&& strcmp(OrderSysID,it->OrderSysID)==0)
		{
			break;
		}

	}
	return it;
}
//tested
 void order::update_order_status(OrderListIterator it, CThostFtdcOrderField *pOrder)
 {
	 // here, we could only update some fields, like orderStatus, volume
	 *it=*pOrder;
 }

 #endif//0
//////////////////////////////////////////////////////////////////////////
Orders::Orders() : m_ordHead(NULL), m_ordTail(NULL), m_orderCount(0) 
{}

Orders::~Orders()
{
	OrderNode* pcur = m_ordHead;
	while (pcur)
	{
		OrderNode* pnext = pcur->next;
		delete pcur;
		pcur = pnext;
	}
	m_ordHead = NULL;
	m_ordTail = NULL;
}

/// save one order info
CThostFtdcOrderField* Orders::save_order(CThostFtdcOrderField* pOrder)
{
	OrderNode* pNew = new OrderNode();
	if (pNew == NULL)
		return NULL;

	memcpy(&pNew->order, pOrder, sizeof(CThostFtdcOrderField));

	// append the order to the tail
	MutexGuard lk(m_ordLock);
	append_order_node(pNew);
	++m_orderCount;

	return &(pNew->order);
}

CThostFtdcOrderField* Orders::save_order(CThostFtdcInputOrderField* orderInsertReq, TThostFtdcFrontIDType FrontID, TThostFtdcSessionIDType SessionID)
{
	 OrderNode* pNew = new OrderNode();
	 if (pNew == NULL)
		 return NULL;

	 CThostFtdcOrderField& order = pNew->order;
	 memset(&order, 0, sizeof(order));
	 ////经济公司代码
	 strcpy_s(order.BrokerID, orderInsertReq->BrokerID);
	 ////投资者代码
	 strcpy_s(order.InvestorID, orderInsertReq->InvestorID);

	 ///单价格条件
	 order.OrderPriceType		= THOST_FTDC_OPT_LimitPrice;
	 ////组合投机套标志
	 order.CombHedgeFlag[0]		= THOST_FTDC_HF_Speculation;
	 ////有效期
	 order.TimeCondition		= THOST_FTDC_TC_GFD;
	 ////成交类型
	 order.VolumeCondition		= THOST_FTDC_VC_AV;
	 ////最小成交量
	 order.MinVolume			= 1;
	 ////触发条件
	 order.ContingentCondition	= THOST_FTDC_CC_Immediately;
	 /////强平原因
	 order.ForceCloseReason		= THOST_FTDC_FCC_NotForceClose;
	 ////强平标志
	 order.UserForceClose		= 0;

	 order.Direction			= orderInsertReq->Direction;
	 order.CombOffsetFlag[0]	= orderInsertReq->CombOffsetFlag[0];
	 order.LimitPrice			= orderInsertReq->LimitPrice;
	 order.VolumeTotalOriginal	= orderInsertReq->VolumeTotalOriginal;
	 order.SessionID			= SessionID;
	 order.FrontID				= FrontID;
	 strcpy_s(order.InstrumentID, orderInsertReq->InstrumentID);
	 strcpy_s(order.OrderRef, orderInsertReq->OrderRef);

	 // append the order to the tail
	 MutexGuard lk(m_ordLock);
	 append_order_node(pNew);
	 ++m_orderCount;

	 return &(pNew->order);
 }

 /// remove the order from the list
int Orders::remove_order(OrderNode* pOrder)
{
	 if (pOrder == NULL)
		 return -1;

	 CThostFtdcOrderField& ord = pOrder->order;
	 Log2DebugView("remove_order: sec=%s,ref=%s,qt=%d,%d,%d,dir=%c,sysId=%s\n", ord.InstrumentID, ord.OrderRef, 
		 ord.VolumeTotalOriginal, ord.VolumeTraded, ord.VolumeTotal, ord.Direction, ord.OrderSysID);
	 // remove the order node from order list
	 MutexGuard lk(m_ordLock);
	 remove_order_node(pOrder);
	 --m_orderCount;

	 // release the order
	 delete pOrder;
	 return 0;
 }

void Orders::update_order_map(CThostFtdcOrderField *pOrder, orderStatus status)
{
	OrderNode* pNode = find_order(pOrder);
	if (pNode == NULL)
		return;

	if(Accepted == status)
	{
		update_order_status(pNode, pOrder);//tested
	}
	else if(InsertRejected == status)
	{
		remove_order(pNode);
	}
	else if(AllFilled == status)
	{
		remove_order(pNode);//tested
	}
	else if(InsertSubmitted == status)
	{
		update_order_status(pNode, pOrder);//tested
	}
	else if(Canceled == status)
	{
		remove_order(pNode);
	}
}

void Orders::update_order_status(OrderNode* pNode, CThostFtdcOrderField *pOrder)
{
	pNode->order = *pOrder;
}

 /// find the order
OrderNode* Orders::find_order(TThostFtdcOrderRefType orderRef, TThostFtdcFrontIDType FrontID, TThostFtdcSessionIDType SessionID, bool isFromTail)
{
	 MutexGuard lk(m_ordLock);
	 OrderNode* pCur = NULL;
	 if (isFromTail)
		 pCur = m_ordTail;
	 else
		 pCur = m_ordHead;

	 while (pCur)
	 {
		 CThostFtdcOrderField& ord = pCur->order;
		 if ( strcmp(orderRef, ord.OrderRef) == 0 && 
			  FrontID == ord.FrontID && 
			  SessionID == ord.SessionID)
			return pCur;

		 if (isFromTail)
			 pCur = pCur->pre;
		 else
			 pCur = pCur->next;
	 }
	 return NULL;
 }

OrderNode* Orders::find_order(TThostFtdcExchangeIDType ExchangeID, TThostFtdcOrderSysIDType orderSysID)
{
	 MutexGuard lk(m_ordLock);
	 OrderNode* pCur = m_ordHead;
	 while (pCur)
	 {
		 CThostFtdcOrderField& ord = pCur->order;
		 if ( strcmp(ExchangeID, ord.ExchangeID) == 0 &&
			  strcmp(orderSysID, ord.OrderSysID) == 0 )
			 return pCur;
		 pCur = pCur->next;
	 }
	 return NULL;
 }

OrderNode* Orders::find_order(CThostFtdcOrderField *pOrder)
{
	OrderNode* pNode = NULL;
	pNode = find_order(pOrder->OrderRef, pOrder->FrontID, pOrder->SessionID);
	if (pNode != NULL)
		return pNode;

	pNode = find_order(pOrder->ExchangeID, pOrder->OrderSysID);
	if (pNode != NULL)
		return pNode;

	pNode = m_ordHead;
	while (pNode)
	{
		CThostFtdcOrderField& ord = pNode->order;
		if( strcmp(pOrder->ExchangeID, ord.ExchangeID) == 0 && 
			strcmp(pOrder->TraderID, ord.TraderID) == 0 &&
			strcmp(pOrder->OrderLocalID, ord.OrderLocalID) == 0)
			break;
		pNode = pNode->next;
	}
	return pNode;
}

void Orders::append_order_node(OrderNode* pNode)
{
	if (m_ordHead == NULL)
	{
		m_ordHead = pNode;
		m_ordTail = pNode;
		pNode->pre= pNode->next = NULL;
	}
	else
	{
		m_ordTail->next = pNode;
		pNode->pre	= m_ordTail;
		pNode->next	= NULL;
		m_ordTail	= pNode;
	}
}

void Orders::remove_order_node(OrderNode* pOrder)
{
	if (m_orderCount == 1)
	{
		m_ordHead = NULL;
		m_ordTail = NULL;
	}
	else
	{
		if (pOrder == m_ordHead)
		{
			m_ordHead = m_ordHead->next;
			m_ordHead->pre = NULL;
		}
		else if (pOrder == m_ordTail)
		{
			m_ordTail = m_ordTail->pre;
			m_ordTail->next = NULL;
		}
		else
		{
			pOrder->pre->next = pOrder->next;
			pOrder->next->pre = pOrder->pre;
		}
	}
}

void Orders::print_order(CThostFtdcOrderField *pOrder)
{
	Log2DebugView("OrderInfo: sec=%s,dir=%c,offset=%c,qtOri=%d,qtLeft=%d,qtTd=%d,sysid=%s\n",
		pOrder->InstrumentID, pOrder->Direction, pOrder->CombOffsetFlag[0],
		pOrder->VolumeTotalOriginal, pOrder->VolumeTotal, pOrder->VolumeTraded, pOrder->OrderSysID);
}

void Orders::print_orders()
{
	Log2DebugView("---------------------------------------\n");
#if 0
	MutexGuard lk(m_ordLock);
	OrderNode* pOrderNode = m_ordHead;
	int idx = 1;
	while (pOrderNode)
	{
		Log2DebugView("Orders[%d/%d]  ", idx++, size());
		print_order(&pOrderNode->order);
		pOrderNode = pOrderNode->next;
	}
#endif
	Log2DebugView("---------------------------------------\n");
}
 // TODO: implement other member functions
