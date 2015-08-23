#ifndef EVENTS_H

#define EVENTS_H

#include "coffeeshop.h"

class event {

public:

	double time_stamp;

	EVENT_TYPE type;

	event(double ts, EVENT_TYPE et)
	{
		time_stamp = ts;

		type = et;

	}

};

//Class to evaluate priority of an event
//Earlier time stamp = higher priority
class compare_events {

public:

	bool operator()(event * e1, event * e2)
	{
		if (e1->time_stamp > e2->time_stamp)
			return true;
		else
			return false;
	}
};

class event_handler {

public:	

	coffee_shop * shop;

	double clock;

	FILE * fptr_log;

	void schedule(event * evt);

	void execute();

	void customer_arrive_event();

	void register_depart_event();

	void drink_receipt_event();

	void check_preconditions_leave();

	void check_preconditions_reorder();

	priority_queue<event*, vector<event*>, compare_events> fel;

	event_handler(coffee_shop * cs);

	~event_handler()
	{
		fclose(fptr_log);
	};

};

#endif
