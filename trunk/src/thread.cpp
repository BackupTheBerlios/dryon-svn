/*
********************************************************************************
*                                 Dryon project
********************************************************************************
*
*  This is free software under the GPL license.
*  Since this software uses Small from Compuphase, it also use its license.
*  Copy of the two licenses are in the main dir (gpl.txt and amx_license.txt
*  Coded by Ammous Julien known as Anthalir
*
********************************************************************************
*/

#include "config.h"

#ifdef __FreeBSD__
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#endif

#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include "thread.h"

// mutex
Mutex::Mutex()
{
#if defined(WIN32)
	mp= CreateMutex(NULL, false, NULL);
#else
	pthread_mutex_init(&mp, NULL);
#endif
}

Mutex::~Mutex()
{
#if defined(WIN32)
	CloseHandle(mp);
#else
	pthread_mutex_destroy(&mp);
#endif
}

void Mutex::request()
{
#if defined(WIN32)
	WaitForSingleObject(mp, INFINITE);
#else
	pthread_mutex_lock(&mp);
#endif
}

void Mutex::release()
{
#if defined(WIN32)
	ReleaseMutex(mp);
#else
	pthread_mutex_unlock(&mp);
#endif
}


// thread

int PThread::id= 0;

PThread::PThread(void *v) : running(false), end(false), param(v)
{
	myid= id++;

#if defined(WIN32)

	ev[0]= CreateEvent(NULL, false, false, NULL);
	ev[1]= CreateEvent(NULL, false, false, NULL);

#else

	pthread_cond_init(&cv[0], NULL);
	pthread_cond_init(&cv[1], NULL);

#endif
}

void PThread::requestEnd()
{
	end= true;
	if( running )
	{
		//printf("[%d] Canceled\n", myid);
#if !defined(WIN32)
		pthread_cancel(th);
#endif

		sendSignal(1);
		sendSignal(2);
		sendSignal(1);
		sendSignal(2);
	}
}

void PThread::waitEnd()
{
	end= true;

	if( running )
	{
#if defined(WIN32)
    	WaitForSingleObject(th, INFINITE);
#else
		pthread_join(th, NULL);
#endif
	}
}

PThread::~PThread()
{
	requestEnd();
	waitEnd();
}

void PThread::run()
{
#if defined(WIN32)
	th= CreateThread(NULL, 0, Entrypoint, this, 0, &tid);
#else
	pthread_create(&th, NULL, Entrypoint, this);
#endif
	running= true;
}

void PThread::sendSignal(int n)
{
	assert((n==1) || (n==2));

	//printf("[%d] Receiving %d\n", myid, n);
#if defined(WIN32)
	SetEvent(ev[n-1]);
#else
	pthread_cond_signal(&cv[n-1]);
#endif

	sched_yield();
}

int PThread::waitSignal(int n, int timeout/*= 0*/)
{
	assert((n==1) || (n==2));
	assert(timeout>=0);
	int ret= 0;

	//printf("[%d] Waiting %d\n", myid, n);

#if defined(WIN32)

	DWORD ts= INFINITE;

	if( timeout!=0 ) ts= timeout;

	if( WaitForSingleObject(ev[n-1], INFINITE) != WAIT_OBJECT_0 )
	{
		//Output("error in wait: %d !\n", GetLastError());
	}

#else

	//printf("[%#x] Waiting sig %d\n", (int)pthread_self(), n);
	pthread_mutex_lock(&mutex);
	if( timeout==0 )
	{
		pthread_cond_wait(&cv[n-1], &mutex);
	}
	else
	{
		struct timespec ts;
		ts.tv_sec= timeout;
		ts.tv_nsec= 0;

		if( pthread_cond_timedwait(&cv[n-1], &mutex, &ts) == ETIMEDOUT )
			ret= -1;
	}

	pthread_mutex_unlock(&mutex);
	//printf("[%#x] Received sig %d\n", (int)pthread_self(), n);


#endif

	return ret;
}

#if !defined(WIN32)
void PThread::cancel_point()
{
	//printf("[%d] Cancel Point\n", myid);
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_testcancel();
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
}
#endif

THREAD_ENTRY(PThread::Entrypoint)
#if defined(WIN32)
{
	PThread *p= (PThread*)v;
	p->user_func();
	ExitThread(0);
	return 0;
}
#else
{
	PThread *p= (PThread*)v;
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
	p->user_func();
	pthread_exit(0);
}

#endif

#ifdef TEST

Mutex M1;

class TestThread1 : public PThread
{
private:

public:
 	TestThread1() : PThread(NULL) {}
	void user_func()
	{
		for(int i= 0; i< 10; i++)
		{
			M1.Lock();
			printf("%#x - Lock\n", this);
			sleep(1);
			printf("%#x - UnLock\n", this);
			M1.UnLock();
		}
	}
};

class TestThread2 : public PThread
{
public:
	TestThread2() : PThread(NULL) {}
	void user_func()
	{
		//printf("t> waiting signal 1...\n");
		//waitSignal(1);
		//printf("t> received\n");
		//printf("t> waiting signal 2\n");
		//waitSignal(2);
		//printf("t> received\n");
		sleep(2);
		printf("t> sending signal 1\n");
		sendSignal(1);
		sleep(2);
		printf("t> waiting signal 1\n");
		waitSignal(1);
		printf("t> 1 received\n");
	}

	void waitSIG(int n)
	{
		waitSignal(n);
	}
};


void main()
{
/*
TestThread2 *t= new TestThread2;
	sleep(2);
	//printf("sending sig 1\n");
	//t->sendSignal(1);
	//sleep(2);
	//printf("sending sig 2\n");
	//t->sendSignal(2);
	printf("waiting sig 1\n");
	t->waitSIG(1);
	printf("1 received\n");
	sleep(5);
	printf("sending sig 1\n");
	t->sendSignal(1);
*/
	TestThread1 *t1= new TestThread1, *t2= new TestThread1;
	sleep(10);
	delete t1;
	delete t2;
}
#endif







