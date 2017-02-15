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
 * Driver code is in kern/tests/synchprobs.c We will
 * replace that file. This file is yours to modify as you see fit.
 *
 * You should implement your solution to the whalemating problem below.
 */

#include <types.h>
#include <lib.h>
#include <thread.h>
#include <test.h>
#include <synch.h>


struct whalehelper* whalehelper;


/*
 * Called by the driver during initialization.
 */

void whalemating_init() {

	whalehelper = kmalloc(sizeof(*whalehelper));
        whalehelper->malelock = lock_create("malelock");
        whalehelper->femalelock = lock_create("femalelock");
        whalehelper->mathcherlock = lock_create("matcherlock");
	whalehelper->verifierlock = lock_create("verifierlock");
        whalehelper->verifier = cv_create("verifier");
        whalehelper->num_whales = 0;
	whalehelper->readytomate=false;
	whalehelper->num_whales_exited = 0;
}

/*
 * Called by the driver during teardown.
 */

void
whalemating_cleanup() {
	KASSERT(whalehelper != NULL);
	lock_destroy(whalehelper->malelock);
	lock_destroy(whalehelper->femalelock);
	lock_destroy(whalehelper->mathcherlock);
	lock_destroy(whalehelper->verifierlock);
	cv_destroy(whalehelper->verifier);
	kfree(whalehelper);
}

void
male(uint32_t index)
{
//	(void)index;
	/*
	 * Implement this function by calling male_start and male_end when
	 * appropriate.
	 */
	male_start(index);	
	lock_acquire(whalehelper->malelock);
	lock_acquire(whalehelper->verifierlock);
	whalehelper->num_whales++;
	while(whalehelper->num_whales%3!=0)
		cv_wait(whalehelper->verifier,whalehelper->verifierlock);
	whalehelper->num_whales_exited++;
        if(whalehelper->num_whales!=whalehelper->num_whales_exited)
                cv_signal(whalehelper->verifier,whalehelper->verifierlock);
	lock_release(whalehelper->verifierlock);
	lock_release(whalehelper->malelock); 	
	male_end(index);
}

void
female(uint32_t index)
{
	(void)index;
	/*
	 * Implement this function by calling female_start and female_end when
	 * appropriate.
	 */
	female_start(index);
        lock_acquire(whalehelper->femalelock);
        lock_acquire(whalehelper->verifierlock);
        whalehelper->num_whales++;
        while(whalehelper->num_whales%3!=0)
                cv_wait(whalehelper->verifier,whalehelper->verifierlock);
         whalehelper->num_whales_exited++;
        if(whalehelper->num_whales!=whalehelper->num_whales_exited)
                cv_signal(whalehelper->verifier,whalehelper->verifierlock);
	lock_release(whalehelper->verifierlock);
        lock_release(whalehelper->femalelock);
        female_end(index);

}

void
matchmaker(uint32_t index)
{
	(void)index;
	/*
	 * Implement this function by calling matchmaker_start and matchmaker_end
	 * when appropriate.
	 */
	matchmaker_start(index);
        lock_acquire(whalehelper->mathcherlock);
        lock_acquire(whalehelper->verifierlock);
        whalehelper->num_whales++;            
        while(whalehelper->num_whales%3!=0)
                cv_wait(whalehelper->verifier,whalehelper->verifierlock);
        whalehelper->num_whales_exited++;
	if(whalehelper->num_whales!=whalehelper->num_whales_exited)
        	cv_signal(whalehelper->verifier,whalehelper->verifierlock);
        
        lock_release(whalehelper->verifierlock);
	lock_release(whalehelper->mathcherlock);
        matchmaker_end(index);       

}

