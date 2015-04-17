#include "account.h"
double account::m_accountCommission=0;
double account::m_preAccountBalance=0;
double account::m_currAccountBalance=0;
double account::m_accountAvailable=0;
double account::m_currAccountMargin=0;      //占用保证金
double account::m_frozenAccountMargin=0;    //冻结保证金
double account::m_frozenAccountCommission=0;    //冻结手续费
double account::m_frozenAccountCash=0;       //冻结资金=冻结保证金+冻结手续费
double account::m_marginAccount=0;
int	   account::m_numInstrument=0;
//tested
account::account()
{
	

	m_frozenMargin=0;
	m_frozenLongMargin=0;
	m_frozenShortMargin=0;
	m_currMargin=0;
	m_currLongMargin=0;
	m_currShortMargin=0;
	m_margin=0;
	m_longMargin=0;
	m_shortMargin=0;
	
	m_accountAvailable=0;
	m_frozenAccountMargin=0;
	m_frozenAccountCash=0;
	m_frozenAccountCommission=0;
	m_currAccountMargin=0;

	m_accountCommission=0;
	m_preAccountBalance=0;

	

	m_LongMarginRatioByMoney=0;

	m_LongMarginRatioByVolume=0;

	m_ShortMarginRatioByMoney=0;

	m_ShortMarginRatioByVolume=0;


	m_OpenRatioByMoney=0;

	m_OpenRatioByVolume=0;

	m_CloseRatioByMoney=0;

	m_CloseRatioByVolume=0;

	m_CloseTodayRatioByMoney=0;

	m_CloseTodayRatioByVolume=0;

	m_ydLongMargin=0;
	m_ydShortMargin=0;
}


void account::update_frozen_cash()
{
	m_frozenAccountCash=m_frozenAccountCommission+m_frozenAccountMargin;
}


//tested
void account::set_account(CThostFtdcTradingAccountField *pTradingAccount)
{
	m_preAccountBalance=pTradingAccount->PreBalance-pTradingAccount->Withdraw+pTradingAccount->Deposit;
	m_currAccountBalance=m_preAccountBalance;
	m_accountAvailable=pTradingAccount->Available;
	m_currAccountMargin=pTradingAccount->CurrMargin;
	m_marginAccount=pTradingAccount->CurrMargin+pTradingAccount->FrozenMargin;
	m_accountCommission=pTradingAccount->Commission;
	m_frozenAccountCommission=pTradingAccount->FrozenCommission;

// 	cout<<"accountCommission:"<<     m_accountCommission  <<endl               
// 		<<"preAccountBalance:"<<		 m_preAccountBalance  <<endl
// 		<<"currAccountBalance:"<<		 m_currAccountBalance<<endl
// 		<<"accountAvailable:"<<		 m_accountAvailable<<endl
// 		<<"currAccountMargin:"  <<     m_currAccountMargin<<endl    
// 		<<"frozenAccountMargin:" <<	 m_frozenAccountMargin<<endl  
// 		<<"frozenAccountCommission:"<< m_frozenAccountCommission<<endl
// 		<<"frozenAccountCash:"  <<  	 m_frozenAccountCash<<endl  
// 		<<"marginAccount:"<<			 m_marginAccount<<endl<<endl;

}

//tested
void account::set_commissionRate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate)
{
	m_OpenRatioByMoney=pInstrumentCommissionRate->OpenRatioByMoney;
	m_OpenRatioByVolume=pInstrumentCommissionRate->OpenRatioByVolume;
	m_CloseRatioByMoney=pInstrumentCommissionRate->CloseRatioByMoney;
	m_CloseRatioByVolume=pInstrumentCommissionRate->CloseRatioByVolume;
	m_CloseTodayRatioByMoney=pInstrumentCommissionRate->CloseTodayRatioByMoney;
	m_CloseTodayRatioByVolume=pInstrumentCommissionRate->CloseTodayRatioByVolume;

	if(m_OpenRatioByVolume==0)
		m_commissionType=ByMoney;
	else
		m_commissionType=ByVolume;
}


void account::set_marginRate(CThostFtdcInstrumentMarginRateField *pMarginRate)
{
	m_LongMarginRatioByVolume=pMarginRate->LongMarginRatioByVolume;
	m_LongMarginRatioByMoney=pMarginRate->LongMarginRatioByMoney;

	m_ShortMarginRatioByVolume=pMarginRate->ShortMarginRatioByVolume;
	m_ShortMarginRatioByMoney=pMarginRate->ShortMarginRatioByMoney;
}



//partially tested under real env
void account::freeze_commission_today(TThostFtdcOffsetFlagType offsetFlag,double price,int vol,vector<CThostFtdcOrderField>::iterator pOrder)
{
	double commission=0;

	if(offsetFlag==THOST_FTDC_OF_Open)  //tested
	{
		if(m_commissionType==ByVolume)
			commission=vol*m_OpenRatioByVolume;
		else
			commission=vol*price*m_OpenRatioByMoney*m_VolumeMultiple;	//tested
	}else
	if(offsetFlag==THOST_FTDC_OF_CloseToday)
	{
		if(m_commissionType==ByVolume)
			commission=vol*m_CloseTodayRatioByVolume;
		else
			commission=vol*price*m_CloseTodayRatioByMoney*m_VolumeMultiple;	

	}

	m_frozenAccountCommission+=commission;
	comPair com(commission,pOrder);
	m_comVec.push_back(com);
	update_frozen_cash();
	update_available();
}


void account::freeze_commission_overnight(double price,int vol,vector<CThostFtdcOrderField>::iterator pOrder)
{
	double commission=0;
	
	if(m_commissionType==ByVolume)
		commission+=vol*m_CloseRatioByVolume;
	else
		commission+=vol*price*m_CloseRatioByMoney;

	m_frozenAccountCommission+=commission;
	comPair com(commission,pOrder);
	m_comVec.push_back(com);
	update_frozen_cash();
	update_available();
}

void account::find_frozen_commission(double& commission,vector<CThostFtdcOrderField>::iterator pOrder)
{
	vector<comPair>::iterator it;
	for(it=m_comVec.begin();it<m_comVec.end();++it)
	{
		if((*it).second==pOrder)
		{
			commission=(*it).first;
			return;
		}
	}
}



void account::free_frozen_commission(int vol,vector<CThostFtdcOrderField>::iterator pOrder)
{
	double commission=0;
	find_frozen_commission(commission,pOrder);
	m_frozenAccountCommission-=(double)vol/pOrder->VolumeTotalOriginal*commission;
	update_frozen_cash();
	update_available();
}


void account::add_commission(TThostFtdcOffsetFlagType offsetFlag,double price,int vol,double sentPrice,int ol,int os,TThostFtdcDirectionType direction)
{
	if(offsetFlag==THOST_FTDC_OF_Open)
	{
		if(m_commissionType==ByVolume)
			m_accountCommission+=vol*m_OpenRatioByVolume;
		else
			m_accountCommission+=vol*price*m_OpenRatioByMoney*m_VolumeMultiple;	

	}else if(direction==THOST_FTDC_D_Buy && (offsetFlag==THOST_FTDC_OF_CloseToday ||
		      offsetFlag==THOST_FTDC_OF_Close     ||
			  offsetFlag==THOST_FTDC_OF_CloseYesterday))
	{
		if(vol>os)
		{
			if(m_commissionType==ByVolume)
			{
				m_accountCommission+=os*m_CloseRatioByVolume;
				m_accountCommission+=(vol-os)*m_CloseTodayRatioByVolume*m_VolumeMultiple;
			}
			else
			{
				m_accountCommission+=os*price*m_CloseRatioByMoney;
				m_accountCommission+=(vol-os)*price*m_CloseTodayRatioByMoney*m_VolumeMultiple;
			}
		}else
		{
			if(m_commissionType==ByVolume)
			{
				m_accountCommission+=vol*m_CloseRatioByVolume;
			}
			else
			{
				m_accountCommission+=vol*price*m_CloseRatioByMoney*m_VolumeMultiple;
			}
		}
	}else if(direction==THOST_FTDC_D_Sell && (offsetFlag==THOST_FTDC_OF_CloseToday ||
		offsetFlag==THOST_FTDC_OF_Close     ||
		offsetFlag==THOST_FTDC_OF_CloseYesterday))
	{
		if(vol>ol)
		{
			if(m_commissionType==ByVolume)
			{
				m_accountCommission+=ol*m_CloseRatioByVolume;
				m_accountCommission+=(vol-ol)*m_CloseTodayRatioByVolume*m_VolumeMultiple;
			}
			else
			{
				m_accountCommission+=ol*price*m_CloseRatioByMoney;
				m_accountCommission+=(vol-ol)*price*m_CloseTodayRatioByMoney*m_VolumeMultiple;
			}
		}else
		{
			if(m_commissionType==ByVolume)
			{
				m_accountCommission+=vol*m_CloseRatioByVolume;
			}
			else
			{
				m_accountCommission+=vol*price*m_CloseRatioByMoney*m_VolumeMultiple;
			}
		}
	}
	update_available();
}



//void account::process_frozen_margin_change(TThostFtdcOffsetFlagType offsetFlag,TThostFtdcDirectionType direction)
// {
// 	
// }





//pt under real env
void account::freeze_margin(TThostFtdcDirectionType Direction,double& price,int& vol,vector<CThostFtdcOrderField>::iterator pOrder)
{
	double frozenMargin=0;
	if(Direction==THOST_FTDC_D_Buy)  //tested
	{
		freeze_long_margin(price,vol,frozenMargin);  
	}
	else if(Direction==THOST_FTDC_D_Sell)  //tested
	{
		freeze_short_margin(price,vol,frozenMargin);
	}
	margPair marg(frozenMargin,pOrder);
	m_margVec.push_back(marg);
	update_frozen_margin();
}
//tested
void account::freeze_long_margin(double& price,int& vol,double& frozenMargin)
{
	frozenMargin=price*vol*m_LongMarginRatioByMoney*m_VolumeMultiple;
	m_frozenLongMargin+=frozenMargin;   //price*vol*m_LongMarginRatioByMoney*m_VolumeMultiple;//初始化有问题： m_frozenLongMargin 可能不为零
	update_long_margin();
}

void account::freeze_short_margin(double& price,int& vol,double& frozenMargin)
{
	frozenMargin=price*vol*m_ShortMarginRatioByMoney*m_VolumeMultiple;
	m_frozenShortMargin+=frozenMargin;  //price*vol*m_ShortMarginRatioByMoney*m_VolumeMultiple;
	update_short_margin();
}


void account::update_frozen_margin()
{
	double preFrozenMargin=m_frozenMargin;
	if(m_MaxMarginSideAlgorithm==THOST_FTDC_MMSA_NO)
	{		
		m_frozenMargin=m_frozenLongMargin+m_frozenShortMargin;	

	}else
	{
		m_frozenMargin=(m_frozenLongMargin>m_frozenShortMargin)?m_frozenLongMargin:m_frozenShortMargin;   //tested
	}
	m_frozenAccountMargin+=m_frozenMargin-preFrozenMargin;
	update_frozen_cash();
}



void account::free_frozen_margin(TThostFtdcDirectionType Direction,int& vol,vector<CThostFtdcOrderField>::iterator pOrder)
{
	double frozenMargin=0;
	find_frozen_margin(frozenMargin,pOrder);
	double margin=(double)vol/pOrder->VolumeTotalOriginal*frozenMargin;
	
	if(Direction==THOST_FTDC_D_Buy)
	{
		free_frozen_long_margin(margin);//tested
	}
	else if(Direction==THOST_FTDC_D_Sell)  //tested
	{
		free_frozen_short_margin(margin);
	}
	update_frozen_margin();
}

void::account::find_frozen_margin(double& margin,vector<CThostFtdcOrderField>::iterator pOrder)
{
	vector<margPair>::iterator it;
	for(it=m_margVec.begin();it<m_margVec.end();++it)
	{
		if((*it).second==pOrder)
		{
			margin=(*it).first;
			break;
		}
	}
}


void account::free_frozen_long_margin(double margin)
{
	m_frozenLongMargin-=margin;
	update_long_margin();
}


void account::free_frozen_short_margin(double margin)
{
	m_frozenShortMargin-=margin;
	update_short_margin();
}


void account::process_curr_margin_change(TThostFtdcOffsetFlagType offsetFlag,TThostFtdcDirectionType direction)
{
	if((offsetFlag==THOST_FTDC_OF_Open && direction==THOST_FTDC_D_Buy) ||    //开多
		((offsetFlag==THOST_FTDC_OF_Close || offsetFlag==THOST_FTDC_OF_CloseToday || offsetFlag==THOST_FTDC_OF_CloseYesterday) && (direction==THOST_FTDC_D_Sell)))  //平多
	{
		update_long_margin();
	}else
	{
		update_short_margin();
	}
}


void account::update_curr_long_margin(double price)
{
	m_currLongMargin=m_today_long_position_share*price*m_LongMarginRatioByMoney;         //m_long_position_share*m_LongMarginRatioByMoney;
	update_curr_margin();
	update_long_margin();
}

void account::update_curr_short_margin(double price)
{
	m_currShortMargin=m_today_short_position_share*m_ShortMarginRatioByMoney;
	update_curr_margin();
	update_short_margin();
}

void account::update_curr_margin()
{
	double preCurrMargin=m_currMargin;
	if(m_MaxMarginSideAlgorithm==THOST_FTDC_MMSA_NO)
	{		
		m_currMargin=m_currLongMargin+m_currShortMargin;	

	}else
	{
		m_currMargin=(m_currLongMargin>m_currShortMargin)?m_currLongMargin:m_currShortMargin;   //tested
	}
	m_currAccountMargin+=m_currMargin-preCurrMargin;

}


//tested
void account::update_long_margin()
{		
	m_longMargin=m_currLongMargin+m_frozenLongMargin+m_ydLongMargin;
	update_margin();
}
//tested
void account::update_short_margin()
{
	m_shortMargin=m_currShortMargin+m_frozenShortMargin+m_ydShortMargin;
	update_margin();
}





//tested
void account::update_margin()
{

	double preMargin=m_margin;
	if(m_MaxMarginSideAlgorithm==THOST_FTDC_MMSA_NO)
	{		
		m_margin=m_longMargin+m_shortMargin;	

	}else
	{
		m_margin=(m_longMargin>m_shortMargin)?m_longMargin:m_shortMargin;   //tested
	}
	m_marginAccount+=m_margin-preMargin;
	update_available();
}

void account::update_available()
{
	m_accountAvailable=m_preAccountBalance+get_position_profit()+get_close_profit()-m_frozenAccountCommission-m_accountCommission-m_marginAccount;
}










