#include "events.h"

event_handler::event_handler(coffee_shop * cs)
{
	shop = cs; 

	fptr_log = fopen("log.txt","w");

	//Schedule the first arrival

	double firstArrivalTime = T_OPEN*HOUR2MIN + shop->getArrivalTime();

	event * evt = new event(firstArrivalTime, CUSTOMER_ARRIVE);

	schedule(evt);

};

void event_handler::schedule(event * evt)
{

	fel.push(evt);

}

void event_handler::execute()
{
	//Peek at fel

	event * evt = fel.top();

	clock = evt->time_stamp;

	//Check preconditions

	check_preconditions_reorder();

	check_preconditions_leave();

	//Re-check fel in case any new events were scheduled above

	evt = fel.top();

	clock = evt->time_stamp;

	shop->update_tod(clock);

	//Execute this event
	switch ( evt->type )
	{

	case CUSTOMER_ARRIVE :

		customer_arrive_event();			

		break;

	case DRINK_RECEIPT:

		drink_receipt_event();

		break;

	case REG_DEPART:

		register_depart_event();

		break;

	}

	delete evt;

	//Remove this event from the FEL
	fel.pop();


	fprintf(fptr_log,"%2.1f %d %d %d %d\n",clock,(int)shop->reg_line.size(),
		(int)shop->drink_group.size(),shop->tables_occupied,(int)shop->reg_group.size());

	//Update max register line size
	if((int)shop->reg_line.size() > shop->peak_reg_line)
		shop->peak_reg_line = (int)shop->reg_line.size();

	//Update max drink group size
	if((int)shop->drink_group.size() > shop->peak_drink_group)
		shop->peak_drink_group = (int)shop->drink_group.size();

}

void event_handler::customer_arrive_event()
{
	//If the shop isn't closed, schedule the next arrival time
	if(shop->tod != CLOSED)
	{
		event * evt = new event(clock + shop->getArrivalTime(), CUSTOMER_ARRIVE);

		schedule(evt);
	}

	//If the register line is too long, customer balks
	if(int(shop->reg_line.size()) >= BALK_LIMIT)
	{
		shop->balk_count++;

		return;

	}

	shop->customer_count++;

	customer * cust = new customer(clock);

	//If there's a line, or all cashiers are busy, join the line
	if((shop->reg_line.size() > 0) | (shop->cashiers_occupied == shop->num_cashiers))
	{
		shop->reg_line.push(cust);
	}
	else //they go straight to a cashier
	{

		shop->cashiers_occupied++;

		cust->t_start_reg = clock;

		cust->reg_depart_time = clock + shop->getRegUseTime();

		event * evt = new event(cust->reg_depart_time, REG_DEPART);
		
		schedule(evt);

		shop->reg_group.push_back(cust);

	}




}

void event_handler::register_depart_event()
{

	for(vector<customer*>::iterator it = shop->reg_group.begin(); it!=shop->reg_group.end();)
	{
		customer * cust = *it;

		if(clock >= cust->reg_depart_time)
		{

			shop->t_wait_reg += (cust->t_start_reg - cust->t_arrive);

			shop->t_cash_worked += clock - cust->t_start_reg;

			cust->t_drink_receipt = shop->getDrinkLagTime(clock) + shop->getDrinkMakeTime();

			event * evt = new event(cust->t_drink_receipt, DRINK_RECEIPT);
			
			schedule(evt);

			shop->drink_group.push_back(cust);

			it = shop->reg_group.erase(it);

		}
		else
		{
			it++;

		}

	}

	//is there anyone waiting in the register line now?
	if(shop->reg_line.size() > 0)
	{
		//Get the customer at the front of the line
		customer * next_cust = shop->reg_line.front();

		//Remove this customer from the line
		shop->reg_line.pop();

		//Set its start reg time
		next_cust->t_start_reg = clock;

		//Set its register depart time
		next_cust->reg_depart_time = clock + shop->getRegUseTime();

		event * evt = new event(next_cust->reg_depart_time, REG_DEPART);

		schedule(evt);

		shop->reg_group.push_back(next_cust);

	}
	else
	{
		shop->cashiers_occupied--;

	}

}

void event_handler::drink_receipt_event()
{

	for(vector<customer*>::iterator it = shop->drink_group.begin(); it!=shop->drink_group.end();)
	{
		customer * cust = *it;

		if(clock >= cust->t_drink_receipt)
		{
			shop->t_wait_drink += (cust->t_drink_receipt - cust->reg_depart_time);

			//If the customer wants to sit
			if((cust->stay_in == true) & (shop->tables_occupied < shop->num_tables))
			{
				shop->tables_occupied++;

				cust->latest_t_leave = clock + shop->getTableStayTime();

				//move this customer to the table group
				shop->table_group.push_back(cust);

			}
			else
			{
				//leave now
				cust->t_exit = clock;
				cust->stay_in = false;
				delete cust;


			}

			it = shop->drink_group.erase(it);


		}
		else
		{
			it++;

		}

	}

}

void event_handler::check_preconditions_leave()
{
	//Loop through all customers in table group
	for(vector<customer*>::iterator it = shop->table_group.begin(); it!=shop->table_group.end();)
	{
		customer * cust = *it;

		double time_at_table = clock - cust->t_drink_receipt;

		bool check1 = int(shop->drink_group.size() + shop->reg_line.size()) > LINGER_LIMIT; 
		bool check2 = time_at_table >= LINGER_TIME; //been there long enough?
		bool check3 = clock >= cust->latest_t_leave;

		if((check1 & check2) | check3)
		{
			shop->tables_occupied--;

			cust->t_exit = clock;

			delete cust;

			it = shop->table_group.erase(it);

		}
		else
		{
			it++;

		}

	}

}

void event_handler::check_preconditions_reorder()
{

	//Loop through all customers in table group
	for(vector<customer*>::iterator it = shop->table_group.begin(); it!=shop->table_group.end();)
	{
		customer *cust = *it;

		double time_at_table = clock - cust->t_drink_receipt;

		bool check1 = time_at_table >= LINGER_TIME; //been there long enough?
		bool check2 = cust->reordered == false; //can only reorder once
		bool check3 = int(shop->reg_line.size()) <= REORDER_LENGTH; //is it calm enough?
		bool check4 = shop->tod != CLOSED; //can't reorder if it's closing
		bool check5 = cust->reorder_candidate; 

		if(check1 & check2 & check3 & check4 & check5)
		{
			//This customer gets up to go reorder
			shop->tables_occupied--;

			shop->customer_count++;

			shop->reorder_count++;

			cust->reordered = true;

			cust->t_arrive = clock;

			//If there's a line, or all cashiers are busy, join the line
			if((shop->reg_line.size() > 0) | (shop->cashiers_occupied == shop->num_cashiers))
			{
				shop->reg_line.push(cust);
			}
			else
			{
				shop->cashiers_occupied++;

				cust->t_start_reg = clock;

				cust->reg_depart_time = clock + shop->getRegUseTime();

				event * evt = new event(cust->reg_depart_time, REG_DEPART);

				schedule(evt);

				shop->reg_group.push_back(cust);

			}

			//Remove this guy from the table group
			it = shop->table_group.erase(it);

		}
		else
		{
			it++;

		}
	}

}



