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

#ifndef _THREAD_H

#define _THREAD_H

/*
http://msdn.microsoft.com/library/default.asp?url=/library/en-us/dnucmg/html/UCMGch09.asp
http://www.relisoft.com/win32/active.html
*/

#include <stdio.h>

#if defined(WIN32)
#include <windows.h>
#else
#include <pthread.h>
#endif

#define MUTEX_GET(x) x.request()
#define MUTEX_RELEASE(x) x.release()

class Mutex
{
private:

#if defined(WIN32)
	HANDLE mp;
#else
	pthread_mutex_t mp;
#endif

public:
	Mutex();
	~Mutex();
	void request();
	void release();
};

//! thread class
class PThread
{
private:
	static int id;
	bool running;

protected:
	int myid;

#if defined(WIN32)

	HANDLE th, ev[2];
	DWORD tid;
	#define THREAD_ENTRY(N) DWORD WINAPI N(void *v)

#else

	pthread_t th;
	pthread_cond_t cv[2];
	pthread_mutex_t mutex;
	#define THREAD_ENTRY(N) void *N(void *v)

#endif

	bool end;
	const void *param;

	static THREAD_ENTRY(Entrypoint);
	int waitSignal(int,int timeout=0);	// 0= INFINITE

public:
	PThread(void *v= NULL);
	virtual ~PThread();

	void run();
	void requestEnd();
	void waitEnd();

#if defined(WIN32)
	#define cancel_point() {if(end) return;}
#else
	void cancel_point();
#endif

	virtual void user_func()= 0;
	void sendSignal(int);
};

#endif

