/**
 *  pelecom.h - header file for simulation of service center for Pelecom LTD. 
 */

/*
 * Customer type: what does the customer wish to accomplish?
 */
#define TYPE_NEW      0		// new customer
#define TYPE_UPGRADE  1		// customer wishes to upgrade hardware or program
#define TYPE_REPAIR   2		// customer wisher to repair hardware
#define TYPE_QUIT     3		// special customer type, used to terminate the simulation

/*
 * Ratio between the populations: 
 */
#define POP_NEW     10		// 10% of incoming customers are new
#define POP_UPGRADE 35		// 40% of incoming customers wish to upgrade
#define POP_REPAIR  55		// half of all customers come in to fix broken phones


/*
 * Average service times and spreads for each customer type
 */
#define AVRG_NEW       150000	// Average processing time of new customer
#define SPRD_NEW        25000	// Standard deviation of new customer processing time
#define MIN_NEW         30000	// Minimum time for processing new customer

#define AVRG_UPGRADE    90000	// Same for upgrading customer
#define SPRD_UPGRADE    25000
#define MIN_UPGRADE     30000

#define AVRG_REPAIR    120000 	// Same for repair work
#define SPRD_REPAIR     20000
#define MIN_REPAIR      60000  

#define AVRG_ARRIVE     40000	// Same for customer arrival rate
#define SPRD_ARRIVE    100000
#define MIN_ARRIVE          0

#define AVRG_SORT       10000	// Same for the time it takes to decide what the 
#define SPRD_SORT       10000	// customer came to the center to do
#define MIN_SORT        5000


/*
 * Customer - structure of the messages on the queues
 */
typedef struct data
{
    int   type;
    int   process_time;
    long  enter_time;
    long  start_time;
    long  exit_time;
    long  elapse_time;
} data;


typedef struct 
{
    long  c_id;
    data  c_data;
} customer;

