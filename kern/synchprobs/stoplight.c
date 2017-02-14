/*
 * Copyright (c) 2001, 2002, 2009
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Driver code is in kern/tests/synchprobs.c We will replace that file. This
 * file is yours to modify as you see fit.
 *
 * You should implement your solution to the stoplight problem below. The
 * quadrant and direction mappings for reference: (although the problem is, of
 * course, stable under rotation)
 *
 *   |0 |
 * -     --
 *    01  1
 * 3  32
 * --    --
 *   | 2|
 *
 * As way to think about it, assuming cars drive on the right: a car entering
 * the intersection from direction X will enter intersection quadrant X first.
 * The semantics of the problem are that once a car enters any quadrant it has
 * to be somewhere in the intersection until it call leaveIntersection(),
 * which it should call while in the final quadrant.
 *
 * As an example, let's say a car approaches the intersection and needs to
 * pass through quadrants 0, 3 and 2. Once you call inQuadrant(0), the car is
 * considered in quadrant 0 until you call inQuadrant(3). After you call
 * inQuadrant(2), the car is considered in quadrant 2 until you call
 * leaveIntersection().
 *
 * You will probably want to write some helper functions to assist with the
 * mappings. Modular arithmetic can help, e.g. a car passing straight through
 * the intersection entering from direction X will leave to direction (X + 2)
 * % 4 and pass through quadrants X and (X + 3) % 4.  Boo-yah.
 *
 * Your solutions below should call the inQuadrant() and leaveIntersection()
 * functions in synchprobs.c to record their progress.
 */

#include <types.h>
#include <lib.h>
#include <thread.h>
#include <test.h>
#include <synch.h>

/*
 * Called by the driver during initialization.
 */
#define GO_STRAIGHT 0
#define TURN_LEFT 1
#define TURN_RIGHT 2
#define WAITING_POS -1
#define EXIT_POS -2

//struct lock *intersectionlock;
struct lock *zero_lock;
struct lock *one_lock;
struct lock *two_lock;
struct lock *three_lock;
/*struct cv *zero_cv;
struct cv *one_cv;
struct cv *two_cv;
struct cv *three_cv;*/
//volatile int current_orientation = 0;
//volatile int current_route = 0;
struct rwlock* intersection_lock;
bool isverticalstraightpresent = false;
bool ishorizontalstraightpresent = false;
//struct spinlock * stoplight_spinlock;


void
stoplight_init() {
	zero_lock = lock_create("zero_lock");
        one_lock = lock_create("one_lock");
        two_lock = lock_create("two_lock");
        three_lock = lock_create("three_lock");
	
/*	orientation_lock = lock_create("orientation_lock");
	detination_lock = lock_create("destination_lock");
	orientation_cv = cv_create("orientation_cv");
        direction_cv = cv_create("direction_cv");
*/	
	intersection_lock = rwlock_create("intersection_lock");
//	spinlock_init(stoplight_spinlock);
	return;
}

/*
 * Called by the driver during teardown.
 */

void stoplight_cleanup() {
	lock_destroy(zero_lock);
        lock_destroy(one_lock);       
        lock_destroy(two_lock);       
        lock_destroy(three_lock);       
/*        lock_destroy(orientation_lock);       
        lock_destroy(destination_lock);       
	
        cv_destroy(orientation_cv);
        cv_destroy(destination_cv);
	return;*/
	rwlock_destroy(intersection_lock);
//	spinlock_cleanup(stoplight_spinlock);
}


void
turnright(uint32_t direction, uint32_t index)
{
/* 	lock_acquire(intersectionlock);
        int next_pos = getnextquadrent(direction,WAITING_POS,TURN_RIGHT);
        while(next_pos!=EXIT_POS)
	{
	inQuadrant(next_pos,index);
	next_pos = getnextquadrent(direction,next_pos,TURN_RIGHT);
	}
	leaveIntersection(index);
        lock_release(intersectionlock);
*/
/*	struct lock* lock;
	 switch (direction) {
                case 0:
                	lock_acquire(zero_lock);
			lock = zero_lock;
			while(current_route!=0) 
	               		cv_wait(&zero_cv,&zero_lock);
		case 1:
			lock_acquire(one_lock);
			lock = one_lock;
			while(current_route!=2)
                                cv_wait(&one_cv,&one_lock);
                case 2:
			lock_acquire(two_lock);
			lock = two_lock;
			while(current_route!=0)
                                cv_wait(&two_cv,&two_lock);
                case 3:
			lock_acquire(three_lock);
			lock = three_lock;
			while(current_route!=2)
                                cv_wait(&three_cv,&three_lock);
		default:
			break;
	}
	int next_pos = getnextquadrent(direction,WAITING_POS,TURN_RIGHT);
	inQuadrant(next_pos,index);
	if(current_route==0)
		{
			if(zero_lock->waiting_thread_count==0&&two_lock->waiting_thread_count==0)
				{
				current_route=(current_route+1)%4;
				cv_signal(&zero_cv,&zero_lock);
				cv_signal(&two_cv,&two_lock);				
				}
		}		
	else if(current_route==2)
		{
		 	if(one_lock->waiting_thread_count==0&&three_lock->waiting_thread_count==0)
                                current_route=(current_route+1)%4;
		}
        lock_release(lock);
        leaveIntersection(index);
*/
	rwlock_acquire_read(intersection_lock);
	int next_pos = getnextquadrent(direction,WAITING_POS,GO_STRAIGHT);
	struct lock* cur_lock = getlockforquadrent(next_pos);
	lock_acquire(cur_lock);
	inQuadrant(next_pos,index);
	leaveIntersection(index);
        lock_release(cur_lock);
	rwlock_release_read(intersection_lock);
}	
	
void
gostraight(uint32_t direction, uint32_t index)
{
 /*	lock_acquire(intersectionlock);
        int next_pos = getnextquadrent(direction,WAITING_POS,GO_STRAIGHT);
        while(next_pos!=EXIT_POS)
	{
	inQuadrant(next_pos,index);
	next_pos = getnextquadrent(direction,next_pos,GO_STRAIGHT);
	}
	leaveIntersection(index);
        lock_release(intersectionlock);
*/
	rwlock_acquire_read(intersection_lock);
	struct lock * cur_lock;
	int next_pos = getnextquadrent(direction,WAITING_POS,GO_STRAIGHT);
	while(true)
        {
        cur_lock = getlockforquadrent(next_pos);
	lock_acquire(cur_lock);
        inQuadrant(next_pos,index);
        next_pos = getnextquadrent(direction,next_pos,GO_STRAIGHT);
	if(next_pos== EXIT_POS)
        {       
                leaveIntersection(index);
                lock_release(cur_lock);
                break;
        }
        lock_release(cur_lock);        
	}
        rwlock_release_read(intersection_lock);
}

void
turnleft(uint32_t direction, uint32_t index)
{
 /*	lock_acquire(intersectionlock);
        int next_pos = getnextquadrent(direction,WAITING_POS,TURN_LEFT);
        while(next_pos!=EXIT_POS)
	{
	inQuadrant(next_pos,index);
	next_pos = getnextquadrent(direction,next_pos,TURN_LEFT);
	}
	leaveIntersection(index);
        lock_release(intersectionlock);
*/
	struct lock * cur_lock;
	rwlock_acquire_write(intersection_lock);
	int next_pos = getnextquadrent(direction,WAITING_POS,TURN_LEFT);
        while(true)
        {
        cur_lock = getlockforquadrent(next_pos);
        lock_acquire(cur_lock);
        inQuadrant(next_pos,index);
        next_pos = getnextquadrent(direction,next_pos,TURN_LEFT);
        if(next_pos== EXIT_POS)
	{
                leaveIntersection(index);
		lock_release(cur_lock);
        	break;
	}
	lock_release(cur_lock);
        }
        rwlock_release_write(intersection_lock);

}


int getnextquadrent(int direction,int currentquadrent,int turn)
{
	switch (turn) {
                case GO_STRAIGHT:
			if(currentquadrent==WAITING_POS)
				return direction;
			else if(currentquadrent == direction)
				return (currentquadrent+3)%4;
			else
				return EXIT_POS;		
                case TURN_LEFT:
			if(currentquadrent==WAITING_POS)
                                return direction;
                        else if(currentquadrent == direction)
                                return (currentquadrent+3)%4;
                        else if(currentquadrent == (direction+3)%4)
                                return (currentquadrent+3)%4;
			else
                                return EXIT_POS;
                case TURN_RIGHT:
                	if(currentquadrent==WAITING_POS)
                                return direction;
                        else
                                return EXIT_POS;
		default:
			return WAITING_POS;        
}

}

struct lock* getlockforquadrent(int quadrent)
{
	 switch (quadrent) {
                case 0:
                        return zero_lock;
                case 1:
                        return one_lock;
                case 2:
                        return  two_lock;
                case 3:
                        return three_lock;
                default:
                        return zero_lock;
        }

}
/*
bool is_reader_or_writer(int direction,int destination)
{
	
}
*/
/*int getOrientation(int direction)
{
	if(direction%2==0)
		return VERTICAL;
	else
		return HORIZONTAL;
}
*/

