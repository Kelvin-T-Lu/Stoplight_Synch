/* 
 * stoplight.c
 *
 * 31-1-2003 : GWA : Stub functions created for CS161 Asst1.
 *
 * NB: You can use any synchronization primitives available to solve
 * the stoplight problem in this file.
 */


/*
 * Includes
 */

#include <types.h>
#include <lib.h>
#include <test.h>
#include <thread.h>
#include <synch.h>

/*
 * Constants
 */

//Number of vehicles created.
#define NVEHICLES 20

//Creates an integer representation of each lane.
#define A 0
#define B 1 
#define C 2

//Declaring a turn direction with an integer.
#define RIGHT 0
#define LEFT 1

// Direction string corresponding to index
char stringDirection[2][6] = {"Right", "Left"};

// Lanes as strings corresponding to index
char charLane[] = {'A', 'B', 'C'};

#define NUMROUTES 3 //Number of Routes

//Integer representation of vehicle types.
#define CAR 0
#define TRUCK 1
// Vehicle strings corresponding to index
char type[2][6] = {"Car", "Truck"};


//Static Variables Declaration.

// Locks used for program.
static struct lock *AB;
static struct lock *BC; 
static struct lock *CA;
// Locks for printing.
static struct lock *printAB;
static struct lock *printBC;
static struct lock *printCA;
// Locks for requirements of deadlocks from left turns
static struct lock *left1;
static struct lock *left2;

char intersection[NUMROUTES][3] = {"AB", "BC", "CA"};

// Counts to keep track of certain variables
static int countVehicles; 
static int countLeft;
static int countRight;
static int countLeft1;
static int countLeft2;

//Number of cars in a lane
static int waitingCarsCountA = 0;
static int waitingCarsCountB = 0;
static int waitingCarsCountC = 0;

// Function Definitions
/*
 * Prints the  initial vehicle information when it arrives at an intersection. 
 */
static void printInfo(unsigned long vehicleDirection,
			unsigned long vehicleNumber,
			unsigned long vehicleType,
			unsigned long direction){
	//Calculates the final destination of the vehicle.
	int destination;

	if (direction == RIGHT){ //Turning Right
		destination = (vehicleDirection + 1) % NUMROUTES;
	} else { //Turning LEFT
		destination = (vehicleDirection + 2) % NUMROUTES;
	}

	kprintf("%s %lu waiting at Route %c wants to turn %s to Route %c.\n",
			type[vehicleType], vehicleNumber, charLane[vehicleDirection],
			stringDirection[direction], charLane[destination]
			);
}

/*
 * Handles whether a truck should be yielded and manages counts of cars 
 * sleeping/waiting in lanes.
 */
void handleVehicle(unsigned long vehicletype, unsigned long lane){
  // Get the waitingCarCount for the specified lane.
  int *waitingCarsCountLane;
  switch(lane){
    case A:
      waitingCarsCountLane = &waitingCarsCountA;
      break;
    case B:
      waitingCarsCountLane = &waitingCarsCountB;
      break;
    case C:
      waitingCarsCountLane = &waitingCarsCountC;
      break;
  }
  // If vehicle type is truck, yield to other car threads until cars have finished.
  switch(vehicletype){
    case TRUCK:
      while(*waitingCarsCountLane > 0){
        thread_yield();
      }
      break;
     case CAR:
      *waitingCarsCountLane += 1;
  }
}

/*
 * turnleft()
 *
 * Arguments:
 *      unsigned long vehicledirection: the direction from which the vehicle
 *              approaches the intersection.
 *      unsigned long vehiclenumber: the vehicle id number for printing purposes.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      This function should implement making a left turn through the 
 *      intersection from any direction.
 *      Write and comment this function.
 */

static
void
turnleft(unsigned long vehicledirection,
		unsigned long vehiclenumber,
		unsigned long vehicletype)
{
  /*
   *  Uses two locks to ensure only two vehicles can perform left turns inside
   *  the intersection. This is to avoid deadlocks of having 3 vehicles left turn.
   *  The locks will keep track of a queue, and a counter will be used that 
   *  the queue is evenly distributed.
   */
  if(countLeft1 <= countLeft2){
    countLeft1++;
    lock_acquire(left1);
  }
  else{
    countLeft2++;
    lock_acquire(left2);
  }
  /*
   * Vehicle will try to acquire intersection locks. Once acquired, it'll 
   * acquire the lock to have exclusive access to printing. The vechicle will
   * print on its current status.
   */
	switch(vehicledirection){ //Create 2 do, while loops for mutex locks.
		case A: //Check AB, then check BC.
      lock_acquire(AB);
      lock_acquire(printAB);
      if(vehicletype == CAR){
        waitingCarsCountA--;
      }
      kprintf("%-5s %-2lu is entering AB and waiting for BC.\t\t\t\t\tAB Closed\n", type[vehicletype], vehiclenumber);
      lock_acquire(BC);
      lock_acquire(printBC);
      lock_release(AB);
      kprintf("%-5s %-2lu is entering BC from AB.\t\t\t\t\tAB Open BC Closed\n", type[vehicletype], vehiclenumber);
      lock_release(printAB);
      lock_release(BC);
      kprintf("%-5s %-2lu is leaving BC and exited at Route C.\t\t\t\tBC Open\n", type[vehicletype], vehiclenumber);
      lock_release(printBC);
			break; 
		case B: //Check BC, then CA.
      lock_acquire(BC);
      lock_acquire(printBC);
      if(vehicletype == CAR){
        waitingCarsCountB--;
      }
      kprintf("%-5s %-2lu is entering BC and waiting for CA.\t\t\t\t\tBC Closed\n", type[vehicletype], vehiclenumber);
      lock_acquire(CA);
      lock_acquire(printCA);
      lock_release(BC);
      kprintf("%-5s %-2lu is entering CA from BC.\t\t\t\t\tBC Open CA Closed\n",type[vehicletype], vehiclenumber);
      lock_release(printBC);
      lock_release(CA);
      kprintf("%-5s %-2lu is leaving CA and exited at Route A.\t\t\t\tCA Open\n", type[vehicletype], vehiclenumber);
      lock_release(printCA);
			break; 
		case C: //Check CA, then AB.
      lock_acquire(CA);
      lock_acquire(printCA);
      if(vehicletype == CAR){
        waitingCarsCountC--;
      }
      kprintf("%-5s %-2lu is entering CA and waiting for AB.\t\t\t\t\tCA Closed\n", type[vehicletype], vehiclenumber);
      lock_acquire(AB);
      lock_acquire(printAB);
      lock_release(CA);
      kprintf("%-5s %-2lu is entering AB from CA.\t\t\t\t\tCA Open AB Closed\n", type[vehicletype], vehiclenumber);
      lock_release(printCA);
      lock_release(AB);
      kprintf("%-5s %-2lu is leaving AB and exited at Route B.\t\t\t\tAB Open\n", type[vehicletype], vehiclenumber);
      lock_release(printAB);
			break; 
	}
  if(lock_do_i_hold(left1) == 1){
    countLeft1--;
    lock_release(left1);
  }
  else{
    countLeft2--;
    lock_release(left2);
  }
}


/*
 * turnright()
 *
 * Arguments:
 *      unsigned long vehicledirection: the direction from which the vehicle
 *              approaches the intersection.
 *      unsigned long vehiclenumber: the vehicle id number for printing purposes.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      This function should implement making a right turn through the 
 *      intersection from any direction.
 *      Write and comment this function.
 */

static
void
turnright(unsigned long vehicledirection,
		unsigned long vehiclenumber,
		unsigned long vehicletype)
{
  /*
   * Vehicle will try to acquire intersection locks. Once acquired, it'll 
   * acquire the lock to have exclusive access to printing. The vechicle will
   * print on its current status.
   */
	switch(vehicledirection){
		case A:
			//Check AB, increment number of vehicles in intersection.
			lock_acquire(AB);
      lock_acquire(printAB);
      if(vehicletype == CAR){
        waitingCarsCountA--;
      }
			kprintf("%-5s %-2lu is entering AB.\t\t\t\t\t\t\tAB Closed\n", type[vehicletype], vehiclenumber);
			lock_release(AB);
			kprintf("%-5s %-2lu is leaving AB and exited at Route B.\t\t\t\tAB Open\n",
					type[vehicletype], vehiclenumber);
      lock_release(printAB);
			break; 
		case B: //Check BC.
			lock_acquire(BC);
      lock_acquire(printBC);
      if(vehicletype == CAR){
        waitingCarsCountB--;
      }
			kprintf("%-5s %-2lu is entering BC.\t\t\t\t\t\t\tBC Closed\n", type[vehicletype], vehiclenumber);
			lock_release(BC);
			kprintf("%-5s %-2lu is leaving BC and exited at Route C.\t\t\t\tBC Open\n",
					type[vehicletype], vehiclenumber);
      lock_release(printBC);
			break; 
		case C: //Check CA.
			lock_acquire(CA);
      lock_acquire(printCA);
      if(vehicletype == CAR){
        waitingCarsCountC--;
      }
			kprintf("%-5s %-2lu is entering CA.\t\t\t\t\t\t\tCA Closed\n", type[vehicletype], vehiclenumber);
			lock_release(CA);
			kprintf("%-5s %-2lu is leaving CA and exited at Route A.\t\t\t\tCA Open\n",
					type[vehicletype], vehiclenumber);
      lock_release(printCA);
			break;
	}

}

/*
 * approachintersection()
 *
 * Arguments: 
 *      void * unusedpointer: currently unused.
 *      unsigned long vehiclenumber: holds vehicle id number.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Change this function as necessary to implement your solution. These
 *      threads are created by createvehicles().  Each one must choose a direction
 *      randomly, approach the intersection, choose a turn randomly, and then
 *      complete that turn.  The code to choose a direction randomly is
 *      provided, the rest is left to you to implement.  Making a turn
 *      or going straight should be done by calling one of the functions
 *      above.
 */

static
void
approachintersection(void * unusedpointer,
		unsigned long vehiclenumber) {
	int vehicledirection, turndirection, vehicletype;

	(void) unusedpointer;

	// Randomly sets vehicle variables.

	vehicledirection = random() % 3;
	turndirection = random() % 2;
	vehicletype = random() % 2;


  printInfo(vehicledirection, vehiclenumber, vehicletype, turndirection);
	// If vehicle is a truck, yield to cars. Else add to waitingCarsCount for lane.
  handleVehicle(vehicletype, vehicledirection);

	// Turns left or right depening on turndirection.
	switch(turndirection){
		case LEFT:
			turnleft(vehicledirection, vehiclenumber, vehicletype);
      countLeft++;
			break;
		case RIGHT: 
			turnright(vehicledirection, vehiclenumber, vehicletype);
      countRight++;
			break;
	}

  // Increments count of executed vehicles
	countVehicles++;
}



/*
 * createvehicles()
 *
 * Arguments:
 *      int nargs: unused.
 *      char ** args: unused.
 *
 * Returns:
 *      0 on success.
 *
 * Notes:
 *      Driver code to start up the approachintersection() threads.  You are
 *      free to modiy this code as necessary for your solution.
 */

int
createvehicles(int nargs,
		char ** args)
{
	int index, error;

	/*
	 * Avoid unused variable warnings.
	 */

	(void) nargs;
	(void) args;

	// Creates the lock for the intersection. 
	AB = lock_create("AB"); 
	BC = lock_create("BC"); 
	CA = lock_create("CA");
  // Prevent deadlocks from having 3 left turns
  left1 = lock_create("left1");
  left2 = lock_create("left2");
  printAB = lock_create("printAB");
  printBC = lock_create("printBC");
  printCA = lock_create("printCA");
  //print = lock_create("print");// Used to avoid accident prints

	//Initialize countVehicles, a counter to check if all the
	//Threads has been executed.
	countVehicles = 0;

  countLeft = 0;
  countRight = 0;
  countLeft1 = 0;
  countLeft2 = 0;
	//Ensures that a deadlock isn't present by maximizing
	// Num of current locks being used.
 	//lock_acquire(menu);
  /*
	 * Start NVEHICLES approachintersection() threads.
	 */

	for (index = 0; index < NVEHICLES; index++) {

		error = thread_fork("approachintersection thread",
				NULL,
				index,
				approachintersection,
				NULL
				);

		/*
		 * panic() on error.
		 */

		if (error) {

			panic("approachintersection: thread_fork failed: %s\n",
					strerror(error)
				 );
		}
	}

	//BUSY WAIT SOLUTION
	//Waits until all of the threads are executed.
	while(countVehicles < NVEHICLES){
    thread_yield();
	}
  kprintf("Right turns executed: %d \n", countRight);
  kprintf("Left turns executed: %d \n", countLeft);
  // Destroy locks
	lock_destroy(AB);
	lock_destroy(BC);
	lock_destroy(CA);
  lock_destroy(left1);
  lock_destroy(left2);
  lock_destroy(printAB);
  lock_destroy(printBC);
  lock_destroy(printCA);

	return 0;
}


