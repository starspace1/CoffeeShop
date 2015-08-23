#include <iostream>
#include "events.h"
#include <time.h>

void run_sim(double * aw, double * odt, double * pic, int * prl, int * pdg, int * bc, int * cc, int * rc);

int main(int argc, char *argv[])
{
	srand(time(0));

	FILE * fptr = fopen("output.txt","w");

	int num_runs = 7;

	double avg_wait = 0.0;
	double avg_odt = 0.0;
	double p_idle_cash = 0.0;
	int peak_reg_line = 0;
	int peak_drink_group = 0;
	int balk_count = 0;
	int customer_count = 0;
	int reorder_count = 0;

	for(int i = 0; i < num_runs; i++)
	{

		run_sim(&avg_wait,&avg_odt,&p_idle_cash,&peak_reg_line,&peak_drink_group,&balk_count,
			&customer_count,&reorder_count);

	}


	fprintf(fptr,"Average customer wait time: %2.1f minutes\n",avg_wait/double(num_runs));
	fprintf(fptr,"Average order to delivery time: %2.1f minutes\n",avg_odt/double(num_runs));
	fprintf(fptr,"Peak register line length: %d customers\n",peak_reg_line/num_runs);
	fprintf(fptr,"Peak drink group length: %d customers\n",peak_drink_group/num_runs);
	fprintf(fptr,"Number of lost customers: %d\n",balk_count/num_runs);
	fprintf(fptr,"Percent idle time, cashier(s): %2.1f%%\n",100.0*(p_idle_cash/double(num_runs)));
	fprintf(fptr,"Number of customers: %d\n",customer_count/num_runs);
	fprintf(fptr,"Number of reorders: %d\n",reorder_count/num_runs);

	fclose(fptr);

	system("pause");

	return 0;
}

void run_sim(double * aw, double * odt, double * pic, int * prl, int * pdg, int * bc, int * cc, int * rc)
{
	
	coffee_shop * test_shop = new coffee_shop(2,2,6);

	event_handler eh(test_shop);

	while(eh.fel.size() > 0)
	{
		eh.execute();
	}

	//Average register wait time
	double avg_wait = test_shop->t_wait_reg/double(test_shop->customer_count);

	//Average order to delivery time
	double avg_odt = test_shop->t_wait_drink/double(test_shop->customer_count);

	double mins_total = (T_CLOSE-T_OPEN)*HOUR2MIN*double(test_shop->num_cashiers);

	double p_idle_cash = (mins_total- test_shop->t_cash_worked)/mins_total;

	*aw+=avg_wait;
	*odt+=avg_odt;
	*pic+=p_idle_cash;
	*prl+=test_shop->peak_reg_line;
	*pdg+=test_shop->peak_drink_group;
	*bc+=test_shop->balk_count;
	*cc+=test_shop->customer_count;
	*rc+=test_shop->reorder_count;

	delete test_shop;

}