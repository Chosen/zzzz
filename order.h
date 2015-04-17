#pragma once
#include ".\ThostTraderApi\ThostFtdcTraderApi.h"
#include "lock.h"
#include <vector>
#include <iostream>
using namespace std;
/////////////////////////////////
///orderStatusType是一个订单的状态
/////////////////////////////////
///已经提交
#define InsertSubmitted '0'
///撤单已经提交
#define CancelSubmitted '1'
//、修改已经提交
#define  ModifySubmitted '2'
///已经接受
#define  Accepted   '3'
////保单已经被拒绝
#define InsertRejected '4'
///撤单被拒绝
#define CancelRejecte  '5'
///改单已经被拒绝
#define ModifyRejected  '6'
///部分成交
#define PartiallyFilled '7'
///全部成交 
#define  AllFilled '8'
///撤单成功
#define Canceled '9'
///成交发生
#define traded  '10'

typedef char orderStatus;

#if 0
typedef vector<CThostFtdcOrderField> OrderLists;
typedef vector<CThostFtdcOrderField>::iterator OrderListIterator;

class order
{
public:
	OrderLists	m_orderVec;		// keep all the pending orders
	ThreadMutex	m_orderMtx;		// mutex access m_orderVec

	void create_new_order(CThostFtdcInputOrderField *orderInsertReq, TThostFtdcFrontIDType FrontID, TThostFtdcSessionIDType SessionID);
	void remove_order(OrderListIterator it);
	void update_order_map(CThostFtdcOrderField *pOder, orderStatus status);
	void update_order_status(OrderListIterator it, CThostFtdcOrderField *pOrder);
	OrderListIterator find_order(CThostFtdcOrderField *pOrder);
	OrderListIterator find_order(TThostFtdcExchangeIDType ExchangeID, TThostFtdcOrderSysIDType orderSysID);
	OrderListIterator find_order(TThostFtdcOrderRefType orderRef,TThostFtdcFrontIDType FrontID,TThostFtdcSessionIDType SessionID);
};
#endif//0

//////////////////////////////////////////////////////////////////////////
// an order node of the order list
typedef struct OrderNode
{
	CThostFtdcOrderField order;		// the order info
	OrderNode* pre;					// the previous order
	OrderNode* next;				// the next order
}OrderNode;

/*
 * an order list which keep all the pending orders
 */
class Orders
{
	friend class strategy;
public:
	Orders();
	~Orders();

	/// save one order info
	CThostFtdcOrderField* save_order(CThostFtdcOrderField* pOrder);
	CThostFtdcOrderField* save_order(CThostFtdcInputOrderField* orderInsertReq, TThostFtdcFrontIDType FrontID, TThostFtdcSessionIDType SessionID);

	/// remove the order from the list
	int remove_order(OrderNode* pOrder);

	/// find the order
	OrderNode* find_order(TThostFtdcOrderRefType orderRef, TThostFtdcFrontIDType FrontID, TThostFtdcSessionIDType SessionID, bool isFromTail = false);
	OrderNode* find_order(TThostFtdcExchangeIDType ExchangeID, TThostFtdcOrderSysIDType orderSysID);
	OrderNode* find_order(CThostFtdcOrderField *pOrder);

	/// update order info
	void update_order_map(CThostFtdcOrderField *pOrder, orderStatus status);
	void update_order_status(OrderNode* pNode, CThostFtdcOrderField *pOrder);

	/// return the list count
	size_t size() { return m_orderCount; }

	/// for debug
	void print_order(CThostFtdcOrderField *pOrder);
	void print_orders();

	void get_volumes_by_price(double price, int& volumeTotal, int& totalVolumeOrignal, bool (*cmp)(double x, double y))
	{
		MutexGuard lk(m_ordLock);
		OrderNode* pNode = m_ordHead;
		while (pNode != NULL)
		{
			CThostFtdcOrderField& ord = pNode->order;
			if (cmp(ord.LimitPrice, price))
			{
				volumeTotal += ord.VolumeTotal;
				totalVolumeOrignal += ord.VolumeTotalOriginal;
			}
			pNode = pNode->next;
		}
	}

private:
	OrderNode*	m_ordHead;			// the order list header
	OrderNode*	m_ordTail;			// the order list tail
	ThreadMutex	m_ordLock;			// thread mutex lock for the list
	size_t		m_orderCount;		// the list count

	void append_order_node(OrderNode* pNode);
	void remove_order_node(OrderNode* pNode);
};













