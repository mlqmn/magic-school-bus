#include "SIMLIB/simlib.h"      /* Required for use of simlib.c. */
#include <stdbool.h>            /* Boolean */

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

// Define SAMPST Variables so it's more readable because jesus christ there's lots of things to account for here
// Something to note: The numbers are not in order because they're gonna be added/subtracted so as to not overlap with each other
#define SAMPST_DELAY            0 /* Delay in queue at Stations 1, 2, 3 */
#define SAMPST_SYSTEM           3 /* Time a person is in system by arrival */
#define SAMPST_STOP             6 /* How long bus is stopped at Stations 1, 2, 3 */
#define SAMPST_LOOP             10/* Time for how long the bus loops */

#define LIST_QUEUE              0 /* List LIST_QUEUE + i is the queue at location i */
#define LIST_BUS                3 /* List LIST_BUS + i holds the people on the bus with destination i */

#define NUM_LOCATIONS           3 /* Number of locations */
#define CAR_RENTAL              3 /* Location of Car Rental */

/* Declare non-simlib global variables. */

int i, j, bus_capacity, bus_location, next_location[NUM_LOCATIONS + 1], num_on_bus;
double bus_arrival_time, last_departure_from_rental, travel_time[NUM_LOCATIONS + 1], load_min, load_max, unload_min, unload_max;
bool bus_busy, min_stop_passed;

void bus_depart() { /* Departure of bus from current location */
    int location = bus_location;

    // Count time the bus stopped at said location
    sampst(sim_time - bus_arrival_time, SAMPST_STOP + location);

    // Departure from car rental starts new loop
    if (location == CAR_RENTAL) {
        sampst(sim_time - last_departure_from_rental, SAMPST_LOOP);
        last_departure_from_rental = sim_time;
    }

    // Schedule the arrival of the bus to the next location.
    bus_location = 0;
    transfer[3] = next_location[location];
    event_schedule(sim_time + travel_time[location], EVENT_BUS_ARRIVAL);
}

void serve_next() { /* Start next unloading/loading at current location.
                       if nothing to do, let bus depart*/
    int location = bus_location;
    double arrival_time, origin, destination;

    // Check if anyone on the bus is getting off here
    if (list_size[LIST_BUS + location] > 0) {
        // Start unloading the first person for this location
        list_remove(FIRST, LIST_BUS + location);
        arrival_time = transfer[1];
        origin = transfer[2];
        bus_busy = true;
        transfer[3] = arrival_time;
        transfer[4] = origin;
        event_schedule(sim_time + uniform(unload_min, unload_max, STREAM_UNLOADING), EVENT_UNLOAD_DONE);
    }

    // Unload done. Check if there's people to be loaded
    else if (list_size[LIST_QUEUE + location] > 0 && num_on_bus < bus_capacity) {
        // Start loading the first person in the queue, tally delay in queue for this location
        list_remove(FIRST, LIST_QUEUE + location);
        sampst(sim_time - transfer[1], SAMPST_DELAY + location);
        arrival_time = transfer[1];
        origin = transfer[2];
        destination = transfer[3];
        bus_busy = true;
        transfer[3] = arrival_time;
        transfer[4] = origin;
        transfer[5] = destination;
        event_schedule(sim_time + uniform(load_min, load_max, STREAM_LOADING), EVENT_LOAD_DONE);
    } else {
        // Nothing to unload or load, bus leaves if min stop time passed, wait otherwise
        bus_busy = false;
        if (min_stop_passed) {
            bus_depart();
        }
    }
}

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