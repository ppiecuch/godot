
#include "cmdlib.h"
#include "threads.h"

#define MAX_THREADS 64

int dispatch;
int workcount;
int oldf;
static qboolean pacifier;

/*
=============
GetThreadWork

=============
*/
int GetThreadWork (void)
{
	ThreadLock ();

	if (dispatch == workcount)
	{
		ThreadUnlock ();
		return -1;
	}

	int f = 10*dispatch / workcount;
	if (f != oldf)
	{
		oldf = f;
		if (pacifier)
			qprintf ("%i...", f);
	}

	int r = dispatch;
	dispatch++;
	ThreadUnlock ();

	return r;
}


void (*workfunction) (int);

void ThreadWorkerFunction (int threadnum)
{
	int work;

	while (1)
	{
		work = GetThreadWork ();
		if (work == -1)
			break;
		// qprintf ("thread %i, work %i\n", threadnum, work);
		workfunction(work);
	}
}

void RunThreadsOnIndividual (int workcnt, qboolean showpacifier, void(*func)(int))
{
	if (numthreads == -1)
		ThreadSetDefault ();
	workfunction = func;
	RunThreadsOn (workcnt, showpacifier, ThreadWorkerFunction);
}


/*
===================================================================

WIN32

===================================================================
*/
#ifdef WIN32

#define USED

#include <windows.h>

int numthreads = -1;
CRITICAL_SECTION crit;
static int enter;
static qboolean threaded;

void ThreadSetDefault (void)
{
	SYSTEM_INFO info;

	if (numthreads == -1) // not set manually
	{
		GetSystemInfo (&info);
		numthreads = info.dwNumberOfProcessors;
		if (numthreads < 1 || numthreads > 32)
			numthreads = 1;
	}

	qprintf ("%i threads\n", numthreads);
}


void ThreadLock (void)
{
	if (!threaded)
		return;
	EnterCriticalSection (&crit);
	if (enter)
		Error ("Recursive ThreadLock\n");
	enter = 1;
}

void ThreadUnlock (void)
{
	if (!threaded)
		return;
	if (!enter)
		Error ("ThreadUnlock without lock\n");
	enter = 0;
	LeaveCriticalSection (&crit);
}

/*
=============
RunThreadsOn
=============
*/
void RunThreadsOn (int workcnt, qboolean showpacifier, void(*func)(int))
{
	int threadid[MAX_THREADS];
	HANDLE threadhandle[MAX_THREADS];

	int start = I_FloatTime ();
	dispatch = 0;
	workcount = workcnt;
	oldf = -1;
	pacifier = showpacifier;
	threaded = true;

	// run threads in parallel
	InitializeCriticalSection (&crit);

	if (numthreads == 1) // use same thread
	{
		func (0);
	}
	else
	{
		for (int i=0 ; i<numthreads ; i++)
		{
			threadhandle[i] = CreateThread(
			   NULL, // LPSECURITY_ATTRIBUTES lpsa,
			   0, // DWORD cbStack,
			   (LPTHREAD_START_ROUTINE)func, // LPTHREAD_START_ROUTINE lpStartAddr,
			   (LPVOID)i, // LPVOID lpvThreadParm,
			   0, //   DWORD fdwCreate,
			   &threadid[i]);
		}

		for (int i=0 ; i<numthreads ; i++)
			WaitForSingleObject (threadhandle[i], INFINITE);
	}
	DeleteCriticalSection (&crit);

	threaded = false;
	if (pacifier)
		printf (" (%i)\n", I_FloatTime ()-start);
}


#endif

/*
===================================================================

OSF1

===================================================================
*/

#ifdef __osf__
#define USED

int numthreads = 4;

void ThreadSetDefault (void)
{
	if (numthreads == -1) // not set manually
	{
		numthreads = 4;
	}
}


#include <pthread.h>

pthread_mutex_t	*my_mutex;

void ThreadLock (void)
{
	if (my_mutex)
		pthread_mutex_lock (my_mutex);
}

void ThreadUnlock (void)
{
	if (my_mutex)
		pthread_mutex_unlock (my_mutex);
}


/*
=============
RunThreadsOn
=============
*/
void RunThreadsOn (int workcnt, qboolean showpacifier, void(*func)(int))
{
	pthread_t work_threads[MAX_THREADS];
	pthread_addr_t status;
	pthread_attr_t attrib;
	pthread_mutexattr_t mattrib;

	int start = I_FloatTime ();
	dispatch = 0;
	workcount = workcnt;
	oldf = -1;
	pacifier = showpacifier;
	threaded = true;

	if (pacifier)
		setbuf (stdout, NULL);

	if (!my_mutex)
	{
		my_mutex = malloc (sizeof(*my_mutex));
		if (pthread_mutexattr_create (&mattrib) == -1)
			Error ("pthread_mutex_attr_create failed");
		if (pthread_mutexattr_setkind_np (&mattrib, MUTEX_FAST_NP) == -1)
			Error ("pthread_mutexattr_setkind_np failed");
		if (pthread_mutex_init (my_mutex, mattrib) == -1)
			Error ("pthread_mutex_init failed");
	}

	if (pthread_attr_create (&attrib) == -1)
		Error ("pthread_attr_create failed");
	if (pthread_attr_setstacksize (&attrib, 0x100000) == -1)
		Error ("pthread_attr_setstacksize failed");
	
	for (int i=0 ; i<numthreads ; i++)
	{
		if (pthread_create(&work_threads[i], attrib, (pthread_startroutine_t)func, (pthread_addr_t)i) == -1)
			Error ("pthread_create failed");
	}

	for (int i=0 ; i<numthreads ; i++)
	{
		if (pthread_join (work_threads[i], &status) == -1)
			Error ("pthread_join failed");
	}

	threaded = false;

	if (pacifier)
		printf (" (%i)\n", I_FloatTime ()-start);
}

#endif

/*
===================================================================

IRIX

===================================================================
*/

#ifdef _MIPS_ISA 
#define USED

#include <task.h>
#include <abi_mutex.h>
#include <sys/types.h>
#include <sys/prctl.h>


int numthreads = -1;
abilock_t lck;

void ThreadSetDefault (void)
{
	if (numthreads == -1)
		numthreads = prctl(PR_MAXPPROCS);
	printf ("%i threads\n", numthreads);
	usconfig (CONF_INITUSERS, numthreads);
}


void ThreadLock (void)
{
	spin_lock (&lck);
}

void ThreadUnlock (void)
{
	release_lock (&lck);
}


/*
=============
RunThreadsOn
=============
*/
void RunThreadsOn (int workcnt, qboolean showpacifier, void(*func)(int))
{
	int pid[MAX_THREADS];

	int start = I_FloatTime ();
	dispatch = 0;
	workcount = workcnt;
	oldf = -1;
	pacifier = showpacifier;
	threaded = true;

	if (pacifier)
		setbuf (stdout, NULL);

	init_lock (&lck);

	for (int i=0 ; i<numthreads-1 ; i++)
	{
		pid[i] = sprocsp ( (void (*)(void *, size_t))func, PR_SALL, (void *)i, NULL, 0x100000);
		// pid[i] = sprocsp ( (void (*)(void *, size_t))func, PR_SALL, (void *)i, NULL, 0x80000);
		if (pid[i] == -1)
		{
			perror ("sproc");
			Error ("sproc failed");
		}
	}
		
	func(i);

	for (int i=0 ; i<numthreads-1 ; i++)
		wait (NULL);

	threaded = false;

	if (pacifier)
		printf (" (%i)\n", I_FloatTime ()-start);
}

#endif

/*
=======================================================================

  SINGLE THREAD

=======================================================================
*/

#ifndef USED

int numthreads = 1;

void ThreadSetDefault (void)
{
	numthreads = 1;
}

void ThreadLock (void)
{
}

void ThreadUnlock (void)
{
}

/*
=============
RunThreadsOn
=============
*/
void RunThreadsOn (int workcnt, qboolean showpacifier, void(*func)(int))
{
	dispatch = 0;
	workcount = workcnt;
	oldf = -1;
	pacifier = showpacifier;
	int start = I_FloatTime ();
#ifdef NeXT
	if (pacifier)
		setbuf (stdout, NULL);
#endif
	func(0);

	if (pacifier)
		printf (" (%i)\n", (int)I_FloatTime ()-start);
}

#endif
