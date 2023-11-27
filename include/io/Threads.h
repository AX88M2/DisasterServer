#ifndef THREADS_H
#define THREADS_H

#if defined(__unix) || defined(__unix__)
	#include <pthread.h>
	#include <unistd.h>
	#include <time.h>
	#include <sched.h>

	typedef pthread_t Thread;
	typedef void* (*pthread_cbfunc_t) (void*);
	#define ThreadPrioritse(thr) { struct sched_param param; param.sched_priority = 20; pthread_setschedparam(thr, SCHED_FIFO, &param); }
	#define ThreadSpawn(thr, cb, arg) { pthread_create(&thr, NULL, (pthread_cbfunc_t)(cb), (arg)); pthread_detach(thr); }
	#define ThreadJoin(thr) pthread_join(thr, NULL)
	#define ThreadSleep(ms) { struct timespec ts;\
	ts.tv_sec = ms / 1000;\
	ts.tv_nsec = (ms % 1000) * 1000000;\
	nanosleep(&ts, NULL); }

	typedef pthread_mutex_t Mutex;
	#define MutexCreate(mut) RAssert(pthread_mutex_init(&mut, NULL) == 0)
	#define MutexLock(mut) RAssert(pthread_mutex_lock(&mut) == 0)
	#define MutexUnlock(mut) RAssert(pthread_mutex_unlock(&mut) == 0)
#else
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>

	typedef HANDLE Thread;
	#define ThreadPrioritse(thr) SetThreadPriority(thr, THREAD_PRIORITY_HIGHEST)
	#define ThreadSpawn(thr, cb, arg) (thr = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)(cb), (arg), 0, NULL))
	#define ThreadJoin(thr) WaitForSingleObject(thr, INFINITE)
	#define ThreadSleep(ms) Sleep(ms)

	typedef HANDLE Mutex;
	#define MutexCreate(mut) (mut = CreateMutex(NULL, FALSE, NULL))
	#define MutexLock(mut) RAssert(WaitForSingleObject(mut, INFINITE) == WAIT_OBJECT_0)
	#define MutexUnlock(mut) RAssert(ReleaseMutex(mut))
#endif

	#endif
