/*
 * Synchronization primitives.
 * See synch.h for specifications of the functions.
 */

#include <types.h>
#include <lib.h>
#include <synch.h>
#include <thread.h>
#include <curthread.h>
#include <machine/spl.h>

////////////////////////////////////////////////////////////
//
// Semaphore.

struct semaphore *
sem_create(const char *namearg, int initial_count)
{
	struct semaphore *sem;

	assert(initial_count >= 0);

	sem = kmalloc(sizeof(struct semaphore));
 	DEBUG(DB_KMALLOC, "KMALLOC Interrupt: %p\n", sem); 
	if (sem == NULL) {
		return NULL;
	}

	sem->name = kstrdup(namearg);
	if (sem->name == NULL) {
		kfree(sem);
		return NULL;
	}

	sem->count = initial_count;
	return sem;
}

void
sem_destroy(struct semaphore *sem)
{
	int spl;
	assert(sem != NULL);

	spl = splhigh();
	assert(thread_hassleepers(sem)==0);
	splx(spl);
	/*
	 * Note: while someone could theoretically start sleeping on
	 * the semaphore after the above test but before we free it,
	 * if they're going to do that, they can just as easily wait
	 * a bit and start sleeping on the semaphore after it's been
	 * freed. Consequently, there's not a whole lot of point in 
	 * including the kfrees in the splhigh block, so we don't.
	 */

	kfree(sem->name);
	kfree(sem);
}

void 
P(struct semaphore *sem)
{
	int spl;
	assert(sem != NULL);

	/*
	 * May not block in an interrupt handler.
	 *
	 * For robustness, always check, even if we can actually
	 * complete the P without blocking.
	 */
	assert(in_interrupt==0);

	spl = splhigh();
	while (sem->count==0) {
		thread_sleep(sem);
	}
	assert(sem->count>0);
	sem->count--;
	splx(spl);
}

void
V(struct semaphore *sem)
{
	int spl;
	assert(sem != NULL);
	spl = splhigh();
	sem->count++;
	assert(sem->count>0);
	thread_wakeup(sem);
	splx(spl);
}

////////////////////////////////////////////////////////////
//
// Lock.

struct lock *
lock_create(const char *name)
{
	struct lock *lock;

	lock = kmalloc(sizeof(struct lock));
 	DEBUG(DB_KMALLOC, "KMALLOC Interrupt: %p\n", lock); 
	if (lock == NULL) {
		return NULL;
	}

	lock->name = kstrdup(name);
	if (lock->name == NULL) {
		kfree(lock);
		return NULL;
	}
	
	// add stuff here as needed
  	lock->owner = NULL;	
  	lock->locked = UNLOCKED;
	return lock;
}

void
lock_destroy(struct lock *lock)
{
	assert(lock != NULL);

	// add stuff here as needed
	
	kfree(lock->name);
	kfree(lock);
}

void
lock_acquire(struct lock *lock)
{
	// Write this
  assert(lock != NULL);

  // Prevent context switch by setting priority level to high.
  int spl = splhigh();
  // While lock not acquired by current thread(empty or held by another)
  // sleep until lock is empty to set acquired to curthread. 
  while(lock->locked == LOCKED){
    thread_sleep(lock);
  }
  // Set lock to locked and the owner to current thread
  lock->locked = LOCKED;
  lock->owner = curthread;
  // Set priority level back
  splx(spl);
}

void
lock_release(struct lock *lock)
{
	// Write this
  assert(lock != NULL);
  // If lock held by current thread, release lock, set acquirer to NULL, then 
  // wakeup.
  assert(lock->locked == LOCKED);
  assert(lock_do_i_hold(lock) == 1);
  // Disable interupts to prevent context switch.
  int spl = splhigh();
  // Set lock to not acquired and the acquirer to NULL.
  lock->locked = UNLOCKED;
  lock->owner = NULL;
  // Wakeup thread waiting for the lock.
  thread_wakeup(lock);
  // Set priority level back.
  splx(spl);
}

int
lock_do_i_hold(struct lock *lock)
{
	// Write this
  assert(lock != NULL);
  // If acquiring thread is the current thread,return true (1).
  if(lock->owner == curthread){
    return 1;
  }
  else{
    return 0;
  }
}

////////////////////////////////////////////////////////////
//
// CV


struct cv *
cv_create(const char *name)
{
	struct cv *cv;

	cv = kmalloc(sizeof(struct cv));
	DEBUG(DB_KMALLOC, "cv pointer debug: %p\n", cv); 
	if (cv == NULL) {
		return NULL;
	}

	cv->name = kstrdup(name);
	if (cv->name==NULL) {
		kfree(cv);
		return NULL;
	}
	
	// add stuff here as needed
	
	return cv;
}

void
cv_destroy(struct cv *cv)
{
	assert(cv != NULL);

	// add stuff here as needed
	
	kfree(cv->name);
	kfree(cv);
}

void
cv_wait(struct cv *cv, struct lock *lock)
{
	// Write this
	(void)cv;    // suppress warning until code gets written
	(void)lock;  // suppress warning until code gets written
}

void
cv_signal(struct cv *cv, struct lock *lock)
{
	// Write this
	(void)cv;    // suppress warning until code gets written
	(void)lock;  // suppress warning until code gets written
}

void
cv_broadcast(struct cv *cv, struct lock *lock)
{
	// Write this
	(void)cv;    // suppress warning until code gets written
	(void)lock;  // suppress warning until code gets written
}
