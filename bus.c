#include "SIMLIB/simlib.h"      /* Required for use of simlib.c. */

#define EVENT_PERSON_ARRIVAL    1 /* Event type for arrival of a person to a location */
#define EVENT_BUS_ARRIVAL       2 /* Event type for arrival of the bus to a location */
#define EVENT_UNLOAD_DONE       3 /* Event type for end of unloading one person from the bus */
#define EVENT_LOAD_DONE         4 /* Event type for end of loading one person onto the bus */
#define EVENT_MIN_STOP_END      5 /* Event type for end of the minimum stop time of the bus at a location */
#define EVENT_END_SIMULATION    6 /* Event type for end of simulation */

#define STREAM_INTERARRIVAL     0 /* Random-number stream for interarrivals 
                                    base: interarrival streams are 0 + loc = 1..3 
                                    OR, klo mau, bagi ke STREAM_INTERARRIVAL 1/2/3 trs di-switch2 gt tergantung lokasinya */
#define STREAM_UNLOADING        4 /* Random-number stream for unloading times */
#define STREAM_LOADING          5 /* Random-number stream for loading times */
#define STREAM_DESTINATION      6 /* Random-number stream for car-rental destinations */

#define NUM_LOCATIONS           3 /* Number of locations */
#define CAR_RENTAL              3 /* Location of Car Rental */

/* Declare non-simlib global variables. */

int main() {
    //* Add something about input and output file */
    
    /* Initialize to idle state? */
    
    /*  Initialize simlib */
    init_simlib();
    
    /* Set maxatr = max(maximum number of attributes per record, 4) */
    maxatr = 4;			/* NEVER SET maxatr TO BE SMALLER THAN 4. */
  
    /* Schedule the arrival of the first job. */
    // event_schedule()
    
    /* Schedule the end of the simulation */
    // event_schedule()

    /* Run the simulation until it terminates after an end-simulation event
       (type EVENT_END_SIMULATION) occurs. */


    /* Then close input and output file */
    
}