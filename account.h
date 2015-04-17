#pragma once
#include ".\ThostTraderApi\ThostFtdcTraderApi.h"
#define ByMoney '0'
#define ByVolume '1'
#include "position.h"
typedef char  commissionType;
typedef pair<double,vector<CThostFtdcOrderField>::iterator>comPair;
typedef pair<double,vector<CThostFtdcOrderField>::iterator>margPair;


class account:public position
{
public:
	account();
	void set_account(CThostFtdcTradingAccountField *pTradingAccount);
	void set_commissionRate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate);
	void set_marginRate(CThostFtdcInstrumentMarginRateField *pMarginRate);
	void freeze_commission_today(TThostFtdcOffsetFlagType offsetFlag,double price,int vol,vector<CThostFtdcOrderField>::iterator pOrder);
	void freeze_commission_overnight(double price,int vol,vector<CThostFtdcOrderField>::iterator pOrder);
	
	
	void free_frozen_commission(int vol,vector<CThostFtdcOrderField>::iterator pOrder);
	
	vector<comPair> m_comVec;
	vector<margPair>m_margVec;
	
protected:	
	void add_commission(TThostFtdcOffsetFlagType offsetFlag,
		double price,int vol,double sentPrice,int ol,int os,TThostFtdcDirectionType direction);
	void update_currBalance();
	void update_available();
	
	void find_frozen_commission(double& price,vector<CThostFtdcOrderField>::iterator pOrder);
	void find_frozen_margin(double& margin,vector<CThostFtdcOrderField>::iterator pOrder);
	
	
	
	//void process_frozen_margin_change(TThostFtdcOffsetFlagType offsetFlag,TThostFtdcDirectionType direction);

	void freeze_margin(TThostFtdcDirectionType Direction,double& price,int& vol,vector<CThostFtdcOrderField>::iterator pOrder);
	void freeze_long_margin(double& price,int& vol,double& frozenMargin);
	void freeze_short_margin(double& price,int& vol,double& frozenMargin);
	void free_frozen_margin(TThostFtdcDirectionType Direction,int& vol,vector<CThostFtdcOrderField>::iterator pOrder);
	void free_frozen_long_margin(double margin);
	void free_frozen_short_margin(double margin);

	void process_curr_margin_change(TThostFtdcOffsetFlagType offsetFlag,TThostFtdcDirectionType direction);
	void update_curr_long_margin(double price);
	void update_curr_short_margin(double price);

	void update_long_margin();
	void update_short_margin();
	void update_margin();
	void update_frozen_margin();
	void update_curr_margin();
	void update_frozen_cash();
	double	m_OpenRatioByMoney;
	
	double	m_OpenRatioByVolume;

	double	m_CloseRatioByMoney;

	double	m_CloseRatioByVolume;

	double	m_CloseTodayRatioByMoney;

	double	m_CloseTodayRatioByVolume;



	///��ͷ��֤����
	TThostFtdcRatioType	m_LongMarginRatioByMoney;
	///��ͷ��֤���
	TThostFtdcMoneyType	m_LongMarginRatioByVolume;
	///��ͷ��֤����
	TThostFtdcRatioType	m_ShortMarginRatioByMoney;
	///��ͷ��֤���
	TThostFtdcMoneyType	m_ShortMarginRatioByVolume;

	double m_frozenMargin;

	double  m_frozenLongMargin;

	double  m_frozenShortMargin;

	double  m_currMargin;
	
	double  m_currLongMargin;

	double  m_currShortMargin;

	double	m_ydLongMargin;

	double	m_ydShortMargin;

    double  m_longMargin;

	double  m_shortMargin;

	double  m_margin;

	commissionType m_commissionType;
	TThostFtdcMaxMarginSideAlgorithmType	m_MaxMarginSideAlgorithm;
	static double m_accountCommission;
	static double m_preAccountBalance;
	static double m_currAccountBalance;
	static double m_accountAvailable;
	static double m_currAccountMargin;          //ռ�ñ�֤��
	static double m_frozenAccountMargin;       //���ᱣ֤��
	static double m_frozenAccountCommission;  //����������
	static double m_frozenAccountCash;        //�����ʽ�=���ᱣ֤��+����������
	static double m_marginAccount;
	static int m_numInstrument;
};







