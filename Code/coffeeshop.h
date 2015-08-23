#ifndef COFFEESHOP_H

#define COFFEESHOP_H

#include <queue>
#include <math.h>
#include <vector>

using namespace std;

static double T_OPEN = 6.0;
static double T_CLOSE = 22.0;

static double STAY_MIN = 5.0;
static double STAY_MAX = 60.0;

static double REG_MIN = 0.5;
static double REG_MAX = 2.0;

static double DRINK_PREP_MIN = 0.3; //Filter coffee, easiest drink (pour into cup, put on lid, deliver)
static double DRINK_PREP_MAX = 3.0; //Single person, complicated drink
static double DRINK_PREP_MINMAX = 1.5; //Multiple people, complicated drink still takes at least 1.5 min


static double LINGER_TIME = 30.0; //Half hour before considering another drink

static double INTERARRIVAL_1 = 6.0; //approx 10 customers per hour
static double INTERARRIVAL_2 = 3.0; //approx 20 customers per hour
static double INTERARRIVAL_3 = 2.0; //approx 30 customers per hour
static double INTERARRIVAL_4 = 1.5; //approx 40 customers per hour
static double INTERARRIVAL_5 = 1.2; //approx 50 customers per hour

//static double INTERARRIVAL_1 = 5.0; 
//static double INTERARRIVAL_2 = 2.5; 
//static double INTERARRIVAL_3 = 1.666; 
//static double INTERARRIVAL_4 = 1.25; 
//static double INTERARRIVAL_5 = 1.0; 


static double MIN2HOUR = 1.0/60.0;
static double HOUR2MIN = 1.0/MIN2HOUR;

static int MINSDAY = 60*24;

static int BALK_LIMIT = 20;
static int LINGER_LIMIT = 10; //People in reg and drink lines, combined
static int REORDER_LENGTH = 3;

enum EVENT_TYPE {
	CUSTOMER_ARRIVE,
	TABLE_DEPART,
	REG_DEPART,
	DRINK_RECEIPT
};

enum TIME_OF_DAY {
	CLOSED,
	EARLY_MORNING, //6-7.5
	PEAK_MORNING, //7.5-9.5
	LATE_MORNING, //9.5-11.5
	MID_DAY, //11.5-1.5
	EARLY_AFTERNOON, //1.5-3
	LATE_AFTERNOON, //3-5
	EARLY_EVENING, //5-7
	EVENING, //7-9
	LATE_EVENING //9-10
};


class customer {

public:

	bool stay_in;

	bool reordered;

	bool reorder_candidate;

	double t_arrive;

	double t_start_reg;

	double t_drink_receipt;

	double t_exit;

	double latest_t_leave;

	double reg_depart_time;

	customer(double t);

	~customer() {};

};

class coffee_shop{

public:

	//Constants

	int num_cashiers;

	int num_baristas;

	int num_tables;

	//For generating DSOVs:

	int reorder_count;

	int customer_count;

	int balk_count;

	int peak_reg_line;

	int peak_drink_group;

	double t_wait_reg;

	double t_wait_drink;

	double t_cash_worked;

	//State variables

	int tables_occupied;

	int cashiers_occupied;

	TIME_OF_DAY tod;

	queue<customer*> reg_line;

	vector<customer*> table_group;

	vector<customer*> reg_group;

	vector<customer*> drink_group;

	coffee_shop(int nc, int nb, int nt);

	~coffee_shop();

	double getRegUseTime();

	double getArrivalTime();

	double getTableStayTime();

	double getDrinkMakeTime();

	double getDrinkLagTime(double clock);

	void update_tod(double t);

};


#endif
