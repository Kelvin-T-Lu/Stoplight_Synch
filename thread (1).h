#ifndef _THREAD_H_
#define _THREAD_H_

/*
 * Definition of a thread.
 */

/* Get machine-dependent stuff */
#include <machine/pcb.h>
#include <synch.h>
#define TABLESIZE 128

struct addrspace;

struct thread {
	/**********************************************************/
	/* Private thread members - internal to the thread system */
	/**********************************************************/
	
	struct pcb t_pcb;
	char *t_name;
	const void *t_sleepaddr;
	char *t_stack;
	
	/**********************************************************/
	/* Public thread members - can be used by other code      */
	/**********************************************************/
	
	/*
	 * This is public because it isn't part of the thread system,
	 * and will need to be manipulated by the userprog and/or vm
	 * code.
	 */
	struct addrspace *t_vmspace;

	/*
	 * This is public because it isn't part of the thread system,
	 * and is manipulated by the virtual filesystem (VFS) code.
	 */
	struct vnode *t_cwd;

  	int pid;
  	int ppid;
  	int has_exit;
	int exit_code;
	int children[128];
	int num_children;
	struct semaphore *sem;
};

/**
 * A supplementary data structure to represent a thread
 * inside the process table.
 *    Allows threads to exit, while retaining the pid to be unique.
 *    Process table can't access thread's contents.
 */
struct thread_supp {

	int pid;
	int ppid;
	int has_exit;
	int exit_code;
	int children[128];
	int num_children;
	struct semaphore *sem;
};
// Process Table array to store thread
struct thread_supp **process_table;

/* Call once during startup to allocate data structures. */
struct thread *thread_bootstrap(void);

/* Call during panic to stop other threads in their tracks */
void thread_panic(void);

/* Call during shutdown to clean up (must be called by initial thread) */
void thread_shutdown(void);

/*
 * Make a new thread, which will start executing at "func".  The
 * "data" arguments (one pointer, one integer) are passed to the
 * function.  The current thread is used as a prototype for creating
 * the new one. If "ret" is non-null, the thread structure for the new
 * thread is handed back. (Note that using said thread structure from
 * the parent thread should be done only with caution, because in
 * general the child thread might exit at any time.) Returns an error
 * code.
 */
int thread_fork(const char *name, 
		void *data1, unsigned long data2, 
		void (*func)(void *, unsigned long),
		struct thread **ret);

/*
 * Cause the current thread to exit.
 * Interrupts need not be disabled.
 */
void thread_exit(void);

/*
 * Cause the current thread to yield to the next runnable thread, but
 * itself stay runnable.
 * Interrupts need not be disabled.
 */
void thread_yield(void);

/*
 * Cause the current thread to yield to the next runnable thread, and
 * go to sleep until wakeup() is called on the same address. The
 * address is treated as a key and is not interpreted or dereferenced.
 * Interrupts must be disabled.
 */
void thread_sleep(const void *addr);

/*
 * Cause all threads sleeping on the specified address to wake up.
 * Interrupts must be disabled.
 */
void thread_wakeup(const void *addr);

/*
 * Return nonzero if there are any threads sleeping on the specified
 * address. Meant only for diagnostic purposes.
 */
int thread_hassleepers(const void *addr);


/*
 * Private thread functions.
 */

/* Machine independent entry point for new threads. */
void mi_threadstart(void *data1, unsigned long data2, 
		    void (*func)(void *, unsigned long));

/* Machine dependent context switch. */
void md_switch(struct pcb *old, struct pcb *nu);

/* CODE FOR THE PROCESS TABLE */
/* Initialize a process table. */
struct thread_supp **table_init(int size);

/* Adds an element into the table.
 * Returns the index of the element.
 * Returns -1 on error.
 * */
int table_add(struct thread_supp **table, struct thread_supp *element);

/* Find the element's index.
 * Returns the indx.
 * Returns -1 on error.
 * */
int table_findIndex(struct thread_supp **table, struct thread_supp *element);

/* Find a process in the process table by element.
 * Returns the index of the thread.
 * Return -1 on error.
 * */
struct thread_supp *table_findProcess(struct thread_supp **table, int index);

/*
 * Remove the element from the process table.
 */
void table_remove(struct thread_supp **table, struct thread_supp *element);

/*
 * Remove the element from the process table based on index.
 * Return 0 on success, 1 otherwise.
 */
void table_index_remove(struct thread_supp **table, int index);

/*
 * Destroy and frees the table.
 */
void table_destroy(struct thread_supp **table);

/*
 * Mark the element to exit.
 */
void table_exit(struct thread_supp **table, int index);

#endif /* _THREAD_H_ */
