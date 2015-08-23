#include "coffeeshop.h"

coffee_shop::coffee_shop(int nc, int nb, int nt)
{

	tod = EARLY_MORNING;

	num_cashiers = nc;
	num_baristas = nb;
	num_tables = nt;

	cashiers_occupied = 0;
	tables_occupied = 0;
	peak_reg_line = 0;
	peak_drink_group = 0;
	reorder_count = 0;
	customer_count = 0;
	balk_count = 0;

	t_wait_reg = 0.0;
	t_wait_drink = 0.0;
	t_cash_worked = 0.0;


}

coffee_shop::~coffee_shop()
{


}

double coffee_shop::getArrivalTime()
{
	double rndm = rand()/(double)RAND_MAX;

	switch (tod)
	{
	case CLOSED :
		return -1.0;
	case EARLY_MORNING :
		return -INTERARRIVAL_3 * log(1.0-rndm);
	case PEAK_MORNING :
		return -INTERARRIVAL_5 * log(1.0-rndm);
	case LATE_MORNING :
		return -INTERARRIVAL_4 * log(1.0-rndm);
	case MID_DAY :
		return -INTERARRIVAL_3 * log(1.0-rndm);
	case EARLY_AFTERNOON :
		return -INTERARRIVAL_2 * log(1.0-rndm);
	case LATE_AFTERNOON :
		return -INTERARRIVAL_1 * log(1.0-rndm);
	case EARLY_EVENING :
		return -INTERARRIVAL_2 * log(1.0-rndm);
	case EVENING :
		return -INTERARRIVAL_3 * log(1.0-rndm);
	case LATE_EVENING :
		return -INTERARRIVAL_1 * log(1.0-rndm);
	default :
		return -1.0;
	}

};

double coffee_shop::getRegUseTime()
{
	double rndm = rand()/(double)RAND_MAX;

	return REG_MIN + rndm*(REG_MAX-REG_MIN);

};

double coffee_shop::getTableStayTime()
{
	double rndm = rand()/(double)RAND_MAX;

	return STAY_MIN + rndm*(STAY_MAX-STAY_MIN);

};

double coffee_shop::getDrinkMakeTime()
{
	double rndm = rand()/(double)RAND_MAX;

	//Max drink make time decreases with # of baristas, up to a certain point...
	double make_max = DRINK_PREP_MAX - double(num_baristas)*0.75;
	
	//...but with infinite baristas, min amount of time to make
	//a complicated drink is DRINK_PREP_MINMAX
	if(make_max < DRINK_PREP_MINMAX)
		make_max = DRINK_PREP_MINMAX;

	return DRINK_PREP_MIN + rndm*(make_max-DRINK_PREP_MIN);


};

double coffee_shop::getDrinkLagTime(double clock)
{
	//Look through the customers in the current drink_group and figure out the latest time that they'll be receiving their drink
	
	double t = clock;

	for(vector<customer*>::iterator it = drink_group.begin(); it!=drink_group.end(); ++it)
	{
		customer * cust = *it;

		if(cust->t_drink_receipt > t)
			t = cust->t_drink_receipt;

	}

	return t;

}

void coffee_shop::update_tod(double t)
{
	//Time is in minutes
	//1440 minutes in a day
	//Closed from 12am-6am and 10pm-12am

	//t in minutes since sim start
	int day = int(t) / MINSDAY;

	double t_today = (t - day*MINSDAY)*MIN2HOUR;

	if (t_today < T_OPEN)
	{
		tod = CLOSED;
		return;
	}
	if((t_today >= T_OPEN) & (t_today < 7.5))
	{
		tod = EARLY_MORNING;
		return;
	}
	if((t_today >= 7.5) & (t_today < 9.5))
	{
		tod = PEAK_MORNING;
		return;
	}
	if((t_today >= 9.5) & (t_today < 11.5))
	{
		tod = LATE_MORNING;
		return;
	}
	if((t_today >= 11.5) & (t_today < 13.5))
	{
		tod = MID_DAY;
		return;
	}
	if((t_today >= 13.5) & (t_today < 15.0))
	{
		tod = EARLY_AFTERNOON;
		return;
	}
	if((t_today >= 15.0) & (t_today < 17.0))
	{
		tod = LATE_AFTERNOON;
		return;
	}
	if((t_today >= 17.0) & (t_today < 19.0))
	{
		tod = EARLY_EVENING;
		return;
	}
	if((t_today >= 19.0) & (t_today < 21.0))
	{
		tod = EVENING;
		return;
	}
	if((t_today >= 21.0) & (t_today < T_CLOSE))
	{
		tod = LATE_EVENING;
		return;
	}
	if(t_today > T_CLOSE)
	{
		tod = CLOSED;
		return;
	}

}

customer::customer(double t)
{
	t_arrive = t;
	stay_in = rand() > RAND_MAX / 2;
	reordered = false;
	reorder_candidate = rand() > RAND_MAX / 2;
	t_start_reg = 0.0;
	reg_depart_time = 0.0;
	t_drink_receipt = 0.0;
	latest_t_leave = 0.0;
	t_exit = 0.0;
}


