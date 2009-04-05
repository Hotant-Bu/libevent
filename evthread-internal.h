/*
 * Copyright (c) 2008 Niels Provos <provos@citi.umich.edu>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef _EVTHREAD_INTERNAL_H_
#define _EVTHREAD_INTERNAL_H_

#ifdef __cplusplus
//extern "C" {
#endif

#include "event-config.h"

struct event_base;

#ifndef _EVENT_DISABLE_THREAD_SUPPORT
extern void (*_evthread_locking_fn)(int mode, void *lock);
extern unsigned long (*_evthread_id_fn)(void);
extern void *(*_evthread_lock_alloc_fn)(void);
extern void (*_evthread_lock_free_fn)(void *);

#define EVBASE_USING_LOCKS(base)			\
	(base != NULL && (base)->th_base_lock != NULL)

#define EVTHREAD_GET_ID() \
	(_evthread_id_fn ? _evthread_id_fn() : 1)

#define EVBASE_IN_THREAD(base)				 \
	(_evthread_id_fn == NULL ||			 \
	(base)->th_owner_id == _evthread_id_fn())

#define EVTHREAD_ALLOC_LOCK(lockvar)		\
	((lockvar) = _evthread_lock_alloc_fn ?	\
	    _evthread_lock_alloc_fn() : NULL)

#define EVTHREAD_FREE_LOCK(lockvar)				\
	do {							\
		if (lockvar && _evthread_lock_free_fn)		\
			_evthread_lock_free_fn(lockvar);	\
	} while (0)

#define EVLOCK_LOCK(lock,mode)					\
	do {								\
		if (lock)						\
			_evthread_locking_fn(EVTHREAD_LOCK|mode, lock);	\
	} while (0)
#define EVLOCK_UNLOCK(lock,mode)					\
	do {								\
		if (lock)						\
			_evthread_locking_fn(EVTHREAD_UNLOCK|mode, lock); \
	} while (0)
#define _EVLOCK_SORTLOCKS(lockvar1, lockvar2)				\
	do {								\
		if (lockvar1 && lockvar2 && lockvar1 > lockvar2) {	\
			void *tmp = lockvar1;				\
			lockvar1 = lockvar2;				\
			lockvar2 = tmp;					\
		}							\
	} while (0)

#define EVLOCK_LOCK2(lock1,lock2,mode1,mode2)				\
	do {								\
		void *_lock1_tmplock = (lock1);				\
		void *_lock2_tmplock = (lock2);				\
		_EVLOCK_SORTLOCKS(_lock1_tmplock,_lock2_tmplock);	\
		EVLOCK_LOCK(_lock1_tmplock,mode1);			\
                if (_lock2_tmplock != _lock1_tmplock)                   \
                        EVLOCK_LOCK(_lock2_tmplock,mode2);              \
	} while (0)

#define EVLOCK_UNLOCK2(lock1,lock2,mode1,mode2)				\
	do {								\
		void *_lock1_tmplock = (lock1);				\
		void *_lock2_tmplock = (lock2);				\
		_EVLOCK_SORTLOCKS(_lock1_tmplock,_lock2_tmplock);	\
                if (_lock2_tmplock != _lock1_tmplock)                   \
                        EVLOCK_UNLOCK(_lock2_tmplock,mode2);            \
		EVLOCK_UNLOCK(_lock1_tmplock,mode1);			\
	} while (0)


#define EVBASE_ACQUIRE_LOCK(base, mode, lock) do {			\
		if (EVBASE_USING_LOCKS(base))				\
			_evthread_locking_fn(EVTHREAD_LOCK | mode,	\
			    (base)->lock);				\
	} while (0)

#define EVBASE_RELEASE_LOCK(base, mode, lock) do {			\
		if (EVBASE_USING_LOCKS(base))				\
			_evthread_locking_fn(EVTHREAD_UNLOCK | mode,	\
			    (base)->lock);				\
	} while (0)
#else /* _EVENT_DISABLE_THREAD_SUPPORT */
#define EVTHREAD_GET_ID()	1
#define EVTHREAD_ALLOC_LOCK(lockvar)
#define EVTHREAD_FREE_LOCK(lockvar)

#define EVBASE_IN_THREAD()	1
#define EVBASE_ACQUIRE_LOCK(base, mode, lock)
#define EVBASE_RELEASE_LOCK(base, mode, lock)
#endif

#ifdef __cplusplus
}
#endif

#endif /* _EVTHREAD_INTERNAL_H_ */
