#include "SIMLIB/simlib.h"      /* Required for use of simlib.c. */
#include <stdbool.h>            /* Boolean */

#define EVENT_PERSON_ARRIVAL    1 /* Event type for arrival of a person to a location */
#define EVENT_BUS_ARRIVAL       2 /* Event type for arrival of the bus to a location */
#define EVENT_UNLOAD_DONE       3 /* Event type for end of unloading one person from the bus */
#define EVENT_LOAD_DONE         4 /* Event type for end of loading one person onto the bus */
#define EVENT_MIN_STOP_END      5 /* Event type for end of the minimum stop time of the bus at a location */
#define EVENT_END_SIMULATION    6 /* Event type for end of simulation */

#define TIMEST_BUS              1 /* Timest variable for number on the bus */
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
#define TERMINAL_1              1 /* Location of Terminal 1 */
#define TERMINAL_2              2 /* Location of Terminal 2 */
#define CAR_RENTAL              3 /* Location of Car Rental */

/* Declare non-simlib global variables. */

int i, j, bus_capacity, bus_location, next_location[NUM_LOCATIONS + 1], num_on_bus, length_simulation;
double bus_arrival_time, last_departure_from_rental, travel_time[NUM_LOCATIONS + 1], load_min, load_max, unload_min, unload_max, mean_interarrival[NUM_LOCATIONS + 1], prob_distrib_dest[26], min_stop_time;
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
        list_file(LAST, LIST_BUS + destination);
        num_on_bus++;
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

void person_arrive() { /* When person arrives to a location */
    int destination;
    int location = transfer[3];

    // Schedule next arrival to this location
    transfer[3] = location;
    event_schedule(sim_time + expon(mean_interarrival[location], STREAM_INTERARRIVAL + location), EVENT_PERSON_ARRIVAL);

    if (location == CAR_RENTAL) {
        destination = random_integer(prob_distrib_dest, STREAM_DESTINATION);
    } else { // Destination != CAR_RENTAL
        destination = CAR_RENTAL;
    }

    // Place arriving person at end of queue for this location
    transfer[1] = sim_time;
    transfer[2] = location;
    transfer[3] = destination;
    list_file(LAST, LIST_QUEUE + location);

    // If bus is idle, start loading
    if (bus_location == location && !bus_busy) {
        serve_next();
    }
}

void bus_arrive() { /* When bus arrives to location */
    bus_location = transfer[3];
    bus_arrival_time = sim_time;
    min_stop_passed = 0;

    // Schedule end of the minimum stop time then start unloading
    event_schedule(sim_time + min_stop_time, EVENT_MIN_STOP_END);
    serve_next();
}

void unload_done() { /* When unloading one person is done */
    int origin = transfer[4];

    // Tally time in system for this person's arrival location
    sampst(sim_time - transfer[3], SAMPST_SYSTEM + origin);

    num_on_bus--;
    timest((double)num_on_bus, TIMEST_BUS);

    serve_next();
}

void load_done() { /* When loading one person is done */
    int destination = transfer[5];

    transfer[1] = transfer[3];
    transfer[2] = transfer[4];
    transfer[3] = destination;
    list_file(LAST, LIST_BUS + destination);

    ++num_on_bus;
    timest((double)num_on_bus, TIMEST_BUS);

    serve_next();
}

void min_stop_end() { /* When minimum stop time is done */
    min_stop_passed = true;

    // If no unloading or loading is in process, bus leaves immediately
    if (!bus_busy) {
        bus_depart();
    }
}

void init_simulation() {
    next_location[CAR_RENTAL] = TERMINAL_1;
    next_location[TERMINAL_1] = TERMINAL_2;
    next_location[TERMINAL_2] = CAR_RENTAL;
    bus_location = CAR_RENTAL;
    event_schedule(sim_time + travel_time[TERMINAL_1], EVENT_BUS_ARRIVAL);
    transfer[3] = TERMINAL_1;
    event_schedule(sim_time + expon(mean_interarrival[TERMINAL_1], STREAM_INTERARRIVAL + TERMINAL_1), EVENT_PERSON_ARRIVAL);
    transfer[3] = TERMINAL_2;
    event_schedule(sim_time + expon(mean_interarrival[TERMINAL_2], STREAM_INTERARRIVAL + TERMINAL_2), EVENT_PERSON_ARRIVAL);
    transfer[3] = CAR_RENTAL;
    event_schedule(sim_time + expon(mean_interarrival[CAR_RENTAL], STREAM_INTERARRIVAL + CAR_RENTAL), EVENT_PERSON_ARRIVAL);
    event_schedule(sim_time + length_simulation, EVENT_END_SIMULATION);
}

void read_input(FILE *infile, FILE *outfile) {
    fscanf(infile, "%i", bus_capacity);

    fscanf(infile, "%lg", &mean_interarrival[TERMINAL_1]);
    fscanf(infile, "%lg", &mean_interarrival[TERMINAL_2]);
    fscanf(infile, "%lg", &mean_interarrival[CAR_RENTAL]);

    fscanf(infile, "%lg", &travel_time[TERMINAL_1]);
    fscanf(infile, "%lg", &travel_time[TERMINAL_2]);
    fscanf(infile, "%lg", &travel_time[CAR_RENTAL]);

    fscanf(infile, "%lg", &load_min);
    fscanf(infile, "%lg", &load_max);

    fscanf(infile, "%lg", &unload_min);
    fscanf(infile, "%lg", &unload_max);

    fscanf(infile, "%lg", &min_stop_time);

    fscanf(infile, "%lg", &prob_distrib_dest[TERMINAL_1]);
    fscanf(infile, "%lg", &prob_distrib_dest[TERMINAL_2]);
    fscanf(infile, "%lg", &length_simulation);

    fprintf(outfile, "car rental system simulation\n");

    fprintf(outfile, "bus capacity                  : %i  \n", bus_capacity);

    fprintf(outfile, "mean interarrival terminal 1  : %lgs \n", &mean_interarrival[TERMINAL_1]);
    fprintf(outfile, "mean interarrival terminal 2  : %lgs \n", &mean_interarrival[TERMINAL_2]);
    fprintf(outfile, "mean interarrival car rental  : %lgs \n", &mean_interarrival[CAR_RENTAL]);

    fprintf(outfile, "travel time terminal 1        : %lgs \n", &travel_time[TERMINAL_1]);
    fprintf(outfile, "travel time terminal 2        : %lgs \n", &travel_time[TERMINAL_2]);
    fprintf(outfile, "travel time car rental        : %lgs \n", &travel_time[CAR_RENTAL]);

    fprintf(outfile, "min load time                 : %lgs \n", &load_min);
    fprintf(outfile, "max load time                 : %lgs \n", &load_max);

    fprintf(outfile, "min unload time               : %lgs \n", &unload_min);
    fprintf(outfile, "max unload time               : %lgs \n", &unload_max);

    fprintf(outfile, "min stop time                 : %lgs \n", &min_stop_time);

    fprintf(outfile, "probability go to terminal 1  : %lgs \n", &prob_distrib_dest[TERMINAL_1]);
    fprintf(outfile, "probability go to terminal 2  : %lgs \n", &prob_distrib_dest[TERMINAL_2]);
    fprintf(outfile, "length of simulation          : %lgs \n", &length_simulation);

}

void run_simulation() {
    do {
        timing();
        switch (next_event_type) {
            case EVENT_PERSON_ARRIVAL:
            person_arrive();
            break;
            case EVENT_BUS_ARRIVAL:
            bus_arrive();
            break;
            case EVENT_UNLOAD_DONE:
            serve_next();
            break;
            case EVENT_LOAD_DONE:
            serve_next();
            break;
            case EVENT_MIN_STOP_END:
            bus_depart();
            break;
            case EVENT_END_SIMULATION:
            report();
            break;
        }
    } while (next_event_type != EVENT_END_SIMULATION);

}

int main(void) {
    /* Read input files */
    FILE *infile = fopen("bus.in", "r");
    FILE *outfile = fopen("bus.out", "w");
    read_input(infile, outfile);
    
    /* Initialize to idle state? */
    
    /*  Initialize simlib */
    init_simlib();
    
    /* Set maxatr = max(maximum number of attributes per record, 4) */
    maxatr = 5;			/* NEVER SET maxatr TO BE SMALLER THAN 4. */
  
    /* Schedule the arrival of bus, passengers, and end simulation. */
    init_simulation();

    /* Run the simulation until it terminates after an end-simulation event
       (type EVENT_END_SIMULATION) occurs. */
    run_simulation();

    /* Then close input and output file */
    
}