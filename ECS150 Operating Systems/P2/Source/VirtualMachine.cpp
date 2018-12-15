#include "VirtualMachine.h"
#include "Machine.h"
#include <cstdlib>
#include <unistd.h>
#include <vector>
#include <queue>
#include <list>
#include <iostream>

/*
1. hello.so: (VMStart, VMFileWrite)
2. sleep.so: (VMThreadSleep and use the alarm)
3. thread.so: (VMThreadCreate, VMThreadState, and VMThreadActivate)
4. file.so: (VMFileOpen, VMFileSeek, VMFileRead, and VMFileClose)
5. mutex.so: (VMMutexCreate, VMMutexAcquire, and VMMutexRelease)
*/

extern "C"{

typedef struct ThreadControlBlock
{
	TVMThreadID t_id;
	TVMThreadIDRef t_idRef;
	TVMThreadID s_id;
	TVMThreadID m_id;
	TVMThreadState t_state;
	TVMThreadPriority t_prio;
	TVMThreadEntry t_entry;
	void* t_entryParam;
	TVMMemorySize t_memSize;
	TVMTick t_tick;
	TVMMutexID t_mutexID;
	uint8_t *stackAddr;
	SMachineContext mcntx;
	int t_fileResult;	//Gonna try this constantly changing
	
	ThreadControlBlock(){} //Default Constructor
	ThreadControlBlock(TVMThreadID tid,	
					TVMThreadState state,
					TVMThreadPriority prio)
	{
		t_id = tid;
		t_idRef = &t_id;
		t_state = state;
		t_prio = prio;
	}			//Overload for the main function 
	ThreadControlBlock (TVMThreadID tid,
						TVMThreadState state,
						TVMThreadPriority prio,
						TVMMemorySize memSize,
						TVMThreadEntry entry,
						void* param)
	{
		t_id = tid;
		t_idRef = &t_id;
		t_state = state;
		t_prio = prio;
		t_memSize = memSize;
		t_entry = entry;
		t_entryParam = param;
		stackAddr = new uint8_t[memSize];
	}					//Overloaded Constructor
} ThreadControlBlock;

#define VM_THREAD_PRIORITY_IDLE					((TVMThreadPriority)0x00)
typedef ThreadControlBlock* TCBRef;	//Reference to our TCB
typedef std::vector<TCBRef> TCBVector; 	//TCB Vector Definition to hold all the threads
typedef std::list<TCBRef> TCBQueue;		//TCB Queue Definition to make priority queues
typedef std::list<TCBRef> MutexQueue;	//Queue for Mutexs by priority

typedef struct Mutex
{
	TCBRef ownerThread;
	TVMMutexID mutexID;
	unsigned int lockValue; //0 unlocked 1 locked
	unsigned int t_isWaiting;
	MutexQueue low_prio_MutexQueue;
	MutexQueue norm_prio_MutexQueue;
	MutexQueue high_prio_MutexQueue;
	Mutex()
	{
		ownerThread = NULL;
		t_isWaiting = 0;
		lockValue = 0;
	}
	//TVMMutexID, *TVMMutexIDRef;
} Mutex;

typedef Mutex* MutexRef;
typedef std::vector<MutexRef> MutexVector;

MutexVector allMutexs;
TCBVector t_waitingForMutex;
TCBVector allThreads;					//Place to store every thread
TCBVector sleepingThreads;				//Place to store sleeping threads
TCBVector IOWaiting;
TCBQueue low_prio_ReadyQueue;			//Low Priority Queue
TCBQueue norm_prio_ReadyQueue;			//Normal Priority Queue
TCBQueue high_prio_ReadyQueue;			//High Priority Queue

static volatile TCBRef CURRENT_THREAD;	//Keep track of current running thread
static volatile TCBRef IDLE_THREAD;
static volatile int SLEEPING_THREADS = 0;	//Global Tick used for the basic sleep
static volatile int IOWAITING_THREADS = 0;
static volatile bool IS_IDLE = false;				//Global idle check for current thread

TVMMainEntry VMLoadModule(const char *module);

void pushDownReady(TCBRef thread)
{
	if (thread->t_prio == VM_THREAD_PRIORITY_HIGH)
	{
		high_prio_ReadyQueue.push_back(thread);
//		std::cout << "___PUSHED TO HIGH PRIO___\n";
	}
	else if (thread->t_prio == VM_THREAD_PRIORITY_NORMAL)
	{
		norm_prio_ReadyQueue.push_back(thread);
//		std::cout << "___PUSHED TO NORMAL PRIO___\n";
	}
	else if (thread->t_prio == VM_THREAD_PRIORITY_LOW)
	{
		low_prio_ReadyQueue.push_back(thread);
//		std::cout << "___PUSHED TO LOW PRIO___\n";
	}
}

TCBRef getNextReadyThread()	//Function to grab the next ready thread in the queue
{						//Makes sure to consider prioritie
	TCBRef nextThread = NULL;
	//std::cout<<"___IN GET NEXT THREAD___\n";
	if(!(high_prio_ReadyQueue.empty()))		//Check High Prio Queue
	{//	std::cout<<"___GOT HIGH PRIO___\n";
		nextThread = high_prio_ReadyQueue.front();
		high_prio_ReadyQueue.pop_front();
	}
	else if(!(norm_prio_ReadyQueue.empty()))	//Check Norm Prio Queue
	{//	std::cout<<"___GOT NORM PRIO___\n";
		nextThread = norm_prio_ReadyQueue.front();
		norm_prio_ReadyQueue.pop_front();
	}	
	else if(!(low_prio_ReadyQueue.empty()))	//Check Low Prio Queue
	{//	std::cout<< "___GOT LOW PRIO___\n";
		nextThread = low_prio_ReadyQueue.front();
		low_prio_ReadyQueue.pop_front();
	}
	else if (CURRENT_THREAD->t_prio > 0)
	{//	std::cout<<"___GOT IDLE THREAD___\n";
		nextThread = IDLE_THREAD;
	}
	//std::cout<<"___RETURNING NOW___\n";
	return nextThread;	
}

void idleEntry(void* param) // Not Sure on this one. Puts thread in idle?
{
	IS_IDLE = true;
	MachineEnableSignals();
	//std::cout << "___WAITING IN IDLE___\n";
	while(IS_IDLE);
}

void scheduleThread()		//Thread Scheduling. Context switch the threads
{
	
	/*
	Check the Sleepers first. If a sleeper has been triggered, wake up and push to
	ready ----- MAYBE MOVE THIS TO NEW FUNCTION
	
	get next ready thread from queue (make sure to check code) set that thread to running
	and context switch
	*/

//	std::cout << "___IN SCHEDULE THREAD___\n";
	TMachineSignalState sigState;
	MachineSuspendSignals(&sigState);
	if (CURRENT_THREAD->t_state == VM_THREAD_STATE_RUNNING)
	{
		CURRENT_THREAD->t_state = VM_THREAD_STATE_READY;
		pushDownReady(CURRENT_THREAD);
	}
	TCBRef nextReadyThread = getNextReadyThread();		//Get the next ready thread from queue
	
	if (nextReadyThread != NULL)
	{
		//std::cout << "____CURRENT THREAD ID|STATE: " << CURRENT_THREAD->t_id << "|" << CURRENT_THREAD->t_state << "_____\n";
		//std::cout << "____NEXT THREAD ID|STATE: " << nextReadyThread->t_id << "|" << nextReadyThread->t_state << "_____\n";
		//if (CURRENT_THREAD->t_state != VM_THREAD_STATE_READY && CURRENT_THREAD->t_state != VM_THREAD_STATE_WAITING)
		//	CURRENT_THREAD->t_state = VM_THREAD_STATE_READY;
		//if (nextReadyThread->t_prio == CURRENT_THREAD->t_prio && CURRENT_THREAD->t_state != VM_THREAD_STATE_WAITING)
		//{
		//	nextReadyThread = CURRENT_THREAD; //currently broken kinda
		//}
		
		//std::cout << "NEXT THREAD ID: " << nextReadyThread->t_id << "_______\n;";
//	std::cout << "___SCHEDULING___\n";

		nextReadyThread->t_state = VM_THREAD_STATE_RUNNING;		//Set next ready to running
		TCBRef oldContextSwitcher = CURRENT_THREAD;
		CURRENT_THREAD = nextReadyThread;
//		std::cout << "___OLD ID: " << oldContextSwitcher->t_id << " CONTEXT: " << &oldContextSwitcher->mcntx << "\n";
//		std::cout << "___NEW ID: " << nextReadyThread->t_id << " CONTEXT: " << &nextReadyThread->mcntx << "___\n";
		MachineResumeSignals(&sigState);
		MachineContextSwitch(&(oldContextSwitcher->mcntx), &(nextReadyThread->mcntx));
	}
}

void scheduleSleeperThread(TCBRef sleeper)
{
	TCBRef toPush = sleepingThreads[sleeper->s_id];	//Get thread to push to queue
//	std::cout << "___SLEEPING THREADS ID: " << toPush->t_id << " AWAKING TO READY\n";
	//std::cout << "___SLEEPING THREADS CONTXT: " << &toPush->mcntx << " IDLE THREADS CONTXT " << &IDLE_THREAD->mcntx << " ___\n";
	toPush->t_state = VM_THREAD_STATE_READY;
	pushDownReady(toPush);			//Push to correct queue
	//need to make sure to remove from the vector
	//std::cout << "___ S_T SIZE: " << sleepingThreads.size() << "___\n";
	//delete sleepingThreads[sleeper->s_id];
	//sleepingThreads[sleeper->s_id] = NULL;
	//std::cout << "___ S_T SIZE AFTER: " << sleepingThreads.size() << "___\n";
	SLEEPING_THREADS--;
	scheduleThread();
}

void AlarmCallBack(void *param)	//Alarm callback for sleep.c, needs to be changed for implementing threads
{	//std::cout << "___ENTERING ALARM CALLBACK___\n";
	if (SLEEPING_THREADS > 0)
	{//	std::cout<< "___SLEEPING THREADS NO EMPTY___\n";
		for (unsigned int i = 0; i < sleepingThreads.size(); i++)
		{//	std::cout<<"___ENTERING CALLBACK LOOP___\n";
			sleepingThreads[i]->t_tick -= 1;
			//std::cout << sleepingThreads[i]->t_tick;
			if (sleepingThreads[i]->t_tick <= 0)
			{
			//	std::cout << "___SLEEPING THREAD UNSLEEPING: " << i << "___\n";
				scheduleSleeperThread(sleepingThreads[i]);
			}
		}
	}
}

void Skeleton(void* param) //Skeleton function for callback
{
	TVMThreadID tid = (TVMThreadID) param; 	//Gets the ID from param
	TCBRef thread = allThreads[tid];		//Gets the thread from the thread pool
//	std::cout << "^^^^Calling Thread: " << thread->t_id << "\n";
	MachineEnableSignals();
	thread->t_entry(thread->t_entryParam);	//Calls thread
	VMThreadTerminate(tid);					//Upon exiting, terminate
}


//FILE FUNCTION
void fileCallBack(void *calldata, int result)
{
//	std::cout << "___ENTERED FILE CALL BACK___\n";
	TCBRef fileThread = (TCBRef)calldata;
//	std::cout << "===SETTING RESULT TO FILERESULT: " << result << " FOR THREAD: " << fileThread->t_id << "===\n";
	fileThread->t_fileResult = result;
	fileThread->t_state = VM_THREAD_STATE_READY;
	pushDownReady(fileThread);
	IOWAITING_THREADS--;
//	std::cout << "___SCHEDULING THREADS FROM CALL BACK___\n";
	//if (fileThread->t_prio > CURRENT_THREAD->t_prio)
	scheduleThread();
}

//QUEUES
//readyQueueHigh
//readyQueueMed
//readyQueueLow

//High ready queue ALWAYS has precedence
//Keep TCBs as reference

//THREAD SWITCHES ON:
//Going in and out of waiting
//Alarms going off
//Mutex aquire and release
//I/O


TVMStatus VMStart(int tickms, int machinetickms, int argc, char *argv[])
{
	//Get the module name loaded
	char *moduleName = argv[0];
	TVMMainEntry mainModule;
	mainModule = VMLoadModule(moduleName);
	//Need to make an idle thread idle thread will be 1

	if (mainModule != NULL)
	{
		//Initialize the Machine with tickms for machine
		MachineInitialize(machinetickms);
		//Request alarm
		TMachineAlarmCallback alarmCallBack = AlarmCallBack;
		MachineRequestAlarm(tickms * 1000, alarmCallBack, NULL);
		//Signals
		MachineEnableSignals();

		//Setup the main thread
		TCBRef mainThread = new ThreadControlBlock((TVMThreadID) 0, VM_THREAD_STATE_RUNNING, VM_THREAD_PRIORITY_NORMAL);
		allThreads.push_back(mainThread);
		CURRENT_THREAD = mainThread;
//		std::cout << "___MAIN THREAD CREATED AND SET___\n";
		//Create the idle thread
		TCBRef idleThread = new ThreadControlBlock((TVMThreadID)1, VM_THREAD_STATE_READY, VM_THREAD_PRIORITY_IDLE , 0x100000, idleEntry, NULL);
		IDLE_THREAD = idleThread;
		allThreads.push_back(idleThread);
		MachineContextCreate(&(idleThread->mcntx), idleThread->t_entry, NULL, idleThread->stackAddr, idleThread->t_memSize);
//		std::cout << "___IDLE THREAD CREATED AND SET___\n";
		//Start VMMain if Module wasn't 
//		std::cout << "___CALLING MAIN ARG___\n";
//		std::cout << "___ARGC: " << argc << "___\n";
//		std::cout << "___ARGV_ " << argv[0] << "___\n";
		mainModule(argc, argv);
		return VM_STATUS_SUCCESS;
	}
	else
		return VM_STATUS_FAILURE;
	
}


//VM THREADS
TVMStatus VMThreadCreate(TVMThreadEntry entry, void *param, TVMMemorySize memsize, TVMThreadPriority prio, TVMThreadIDRef tid)
{
	if (entry == NULL || tid == NULL)		//Check if parameters ar ok
		return VM_STATUS_ERROR_INVALID_PARAMETER;
	
	TMachineSignalState sigState; //Create variable to hold current signal state

	//Suspend the signals
	MachineSuspendSignals(&sigState);

	//Initiate new therad
	TCBRef newThread = new ThreadControlBlock();
	newThread->t_entry = entry;
	newThread->t_entryParam = param;
	newThread->t_memSize = memsize;
	newThread->stackAddr = new uint8_t[memsize];
	newThread->t_prio = prio;
	newThread->t_state = VM_THREAD_STATE_DEAD;
	//Set threads ID to the vector size
	*tid = allThreads.size();
	newThread->t_id = *tid;
	newThread->t_idRef = &newThread->t_id;
	//Store thread into the all threads vector
	allThreads.push_back(newThread);
//	std::cout << "<Thread: " << newThread ->t_id << " created>>\n";
	MachineResumeSignals(&sigState);

	return VM_STATUS_SUCCESS;	
}
TVMStatus VMThreadDelete(TVMThreadID thread)
{
	if (thread < 0 || thread >= allThreads.size())
		return VM_STATUS_ERROR_INVALID_ID;
	else if (allThreads[thread] == NULL)
		return VM_STATUS_ERROR_INVALID_ID;
	else if(allThreads[thread]->t_state != VM_THREAD_STATE_DEAD)
		return VM_STATUS_ERROR_INVALID_STATE;

	delete allThreads[thread];
	allThreads[thread] = NULL;

	return VM_STATUS_SUCCESS;
}
TVMStatus VMThreadActivate(TVMThreadID thread)
{
	if (thread < 0 || thread >= allThreads.size())
		return VM_STATUS_ERROR_INVALID_ID;

	TCBRef inThread = allThreads[thread]; //Go get our new thread

	if (inThread->t_state != VM_THREAD_STATE_DEAD)
		return VM_STATUS_ERROR_INVALID_STATE;

	//suspends signals
	TMachineSignalState sigState;
	MachineSuspendSignals(&sigState);

	//Create the context for new activated thread
	TVMThreadEntry skeleton = Skeleton;
	//std::cout << "Thread: " << inThread->t_id << " Activating now........\n";
	MachineContextCreate(&(inThread->mcntx), skeleton, (void*)inThread->t_id, inThread->stackAddr, inThread->t_memSize);
	//void MachineContextCreate(SMachineContextRef mcntxref, void(*entry)(void *), void *param, void *stackaddr, size_t stacksize);

	//Set Thread to ready
	inThread->t_state = VM_THREAD_STATE_READY;
	//std::cout << "___ACTIVATED THREAD PUSHING TO QUEUE___\n";
	pushDownReady(inThread);
	//scheduleThread();

	MachineResumeSignals(&sigState);

	return VM_STATUS_SUCCESS;
}
TVMStatus VMThreadTerminate(TVMThreadID thread)
{
	/* 	Terminates dead thread specified by thread. After termination, the thread enters the state VM_THREAD_STATE_DEAD
		And must release any mutexs it holds
		Scheduling may happen here*/
	if(thread < 0 || thread >= allThreads.size())
		return VM_STATUS_ERROR_INVALID_ID;
	else if(allThreads[thread]->t_state == VM_THREAD_STATE_DEAD)
		return VM_STATUS_ERROR_INVALID_STATE;
		
	TMachineSignalState sigState;		//Suspend Signals
	MachineSuspendSignals(&sigState);

	TCBRef toTerminate = allThreads[thread]; //Get needed thread

	if (toTerminate->t_state == VM_THREAD_STATE_WAITING)	//If our thread was waiting, remove it from the waiting vector
	{
		delete sleepingThreads[thread];	//Delete
		sleepingThreads[thread] = NULL;
	}

	toTerminate->t_state = VM_THREAD_STATE_DEAD;	//Set state to dead
	if (toTerminate->t_prio == VM_THREAD_PRIORITY_LOW)	//Delete from respective priority queue
	{
		low_prio_ReadyQueue.remove(toTerminate);
	}
	else if (toTerminate->t_prio == VM_THREAD_PRIORITY_NORMAL)
	{
		norm_prio_ReadyQueue.remove(toTerminate);
	}
	else if (toTerminate->t_prio == VM_THREAD_PRIORITY_HIGH)
	{
		high_prio_ReadyQueue.remove(toTerminate);
	}


//	VMThreadDelete(thread);	//Delete thread
	scheduleThread();	//Schedule up if needed

	MachineResumeSignals(&sigState); //Resume signals

	return VM_STATUS_SUCCESS;
}
TVMStatus VMThreadID(TVMThreadIDRef threadref)
{
	//set id = threadref
	
	if(*threadref < 0 || *threadref >= allThreads.size())
		return VM_STATUS_ERROR_INVALID_ID;
	else if(threadref == NULL)
		return VM_STATUS_ERROR_INVALID_PARAMETER;
	
	threadref = &CURRENT_THREAD->t_id;
	return VM_STATUS_SUCCESS;
}
TVMStatus VMThreadState(TVMThreadID thread, TVMThreadStateRef stateref)
{	//Note thread is the ID, not an actual thread so we need to retrieve i

	if (stateref == NULL)
		return VM_STATUS_ERROR_INVALID_PARAMETER;

	if (thread >= 0 && thread < allThreads.size())
	{
		TCBRef inThread = allThreads[thread];	//get thread via ID
		*stateref = inThread->t_state;				//set stateRef to state of thread
		return VM_STATUS_SUCCESS;					//return good
	}
	return VM_STATUS_ERROR_INVALID_ID;
}
TVMStatus VMThreadSleep(TVMTick tick)
{
	if (tick == VM_TIMEOUT_INFINITE)
		return VM_STATUS_ERROR_INVALID_PARAMETER;

	TMachineSignalState sigState;
	MachineSuspendSignals(&sigState);

	TCBRef t_Sleeping = CURRENT_THREAD;

	//std::cout << "___IN SLEEP___\n";
	if (tick == VM_TIMEOUT_IMMEDIATE)
	{
		t_Sleeping->t_state = VM_THREAD_STATE_READY;
		pushDownReady(t_Sleeping);
		scheduleThread();
	}
	else
	{
		t_Sleeping->t_tick = tick;
//		std::cout << "___SETTING THREAD:" << t_Sleeping->t_id << " SLEEP TO: " << tick << "___\n";
		t_Sleeping->t_state = VM_THREAD_STATE_WAITING;
		t_Sleeping->s_id = sleepingThreads.size();	//Give the thread a new sleeper ID, s_id
		SLEEPING_THREADS++;
		sleepingThreads.push_back(t_Sleeping);			//Push to sleeper queue

		scheduleThread();

		//ticks = tick
		//set to wait
		//schedule
	}

	MachineResumeSignals(&sigState);

	return VM_STATUS_SUCCESS;
}


//VM MUTEX QUESTIONABLE FUNCTIONALITY
TVMStatus VMMutexCreate(TVMMutexIDRef mutexref)
{
	if (mutexref == NULL)
		return VM_STATUS_ERROR_INVALID_PARAMETER;

	//Suspend Signals
	TMachineSignalState sigState;
	MachineSuspendSignals(&sigState);

	//Create the new Mutex and set a reference to it
	//The constructor of the Mutex takes care of initializing ownerThread to NULL, mutexID to # of all mutex's, and sets lockValue to 0 (open)
	MutexRef newMutex = new Mutex();
	//Aquire mutex ID by the vector size
	newMutex->mutexID = allMutexs.size();
//	std::cout << "<<Mutex: " << newMutex->mutexID << " created>>\n";
	//Put the mutexID in the mutexRef
	*mutexref = newMutex->mutexID;
	allMutexs.push_back(newMutex);

	//Suspend Signals
	MachineResumeSignals(&sigState);

	return VM_STATUS_SUCCESS;
}
TVMStatus VMMutexDelete(TVMMutexID mutex)
{
	if (mutex < 0 || mutex >= allMutexs.size())
		return VM_STATUS_ERROR_INVALID_ID;
	else if (allMutexs[mutex] == NULL)
		return VM_STATUS_ERROR_INVALID_ID;
	else if (allMutexs[mutex]->lockValue == 1)
		return VM_STATUS_ERROR_INVALID_STATE;

	//Suspend Signals
	TMachineSignalState sigState;
	MachineSuspendSignals(&sigState);
	//Delete Mutex
	delete allMutexs[mutex];
	allThreads[mutex] = NULL;
	//Resume Signals
	MachineResumeSignals(&sigState);

	return VM_STATUS_SUCCESS;
}
TVMStatus VMMutexQuery(TVMMutexID mutex, TVMThreadIDRef ownerref)
{
	if (mutex < 0 || mutex >= allMutexs.size())
		return VM_STATUS_ERROR_INVALID_ID;
	else if (allMutexs[mutex] == NULL)
		return VM_STATUS_ERROR_INVALID_ID;
	else if (ownerref == NULL)
		return VM_STATUS_ERROR_INVALID_ID;
	else if (allMutexs[mutex]->lockValue == 0)
		return VM_THREAD_ID_INVALID;
	//Suspend Signals
	TMachineSignalState sigState;
	MachineSuspendSignals(&sigState);
	//Set ownerRef to the mutex's owner's ID
	*ownerref = allMutexs[mutex]->ownerThread->t_id;
	//Resume Signals
	MachineResumeSignals(&sigState);

	return VM_STATUS_SUCCESS;
}

void pushThreadToMutexQueue(TCBRef to_Push, MutexRef cur_Mutex)
{
	cur_Mutex->t_isWaiting++;
//	std::cout << "Number waiting for mutex: " << cur_Mutex->t_isWaiting << "\n";
	if (to_Push->t_prio == VM_THREAD_PRIORITY_HIGH)
	{
		cur_Mutex->high_prio_MutexQueue.push_back(to_Push);
	}
	else if (to_Push->t_prio == VM_THREAD_PRIORITY_NORMAL)
	{
		cur_Mutex->norm_prio_MutexQueue.push_back(to_Push);
	}
	else
	{
	//	std::cout << "Mutex: " << cur_Mutex->mutexID << " pushed to low\n";
		cur_Mutex->low_prio_MutexQueue.push_back(to_Push);
	}
}

TCBRef pullThreadFromMutexQueue(MutexRef mutexToPull)
{
	mutexToPull->t_isWaiting--;
	if (mutexToPull->t_isWaiting < 0)
		return VM_STATUS_FAILURE;
	TCBRef nextThread = NULL;
	
	if (!(mutexToPull->high_prio_MutexQueue.empty()))
	{
		nextThread = mutexToPull->high_prio_MutexQueue.front();
		mutexToPull->high_prio_MutexQueue.pop_front();
	}
	else if (!(mutexToPull->norm_prio_MutexQueue.empty()))
	{
		nextThread = mutexToPull->norm_prio_MutexQueue.front();
		mutexToPull->norm_prio_MutexQueue.pop_front();
	}
	else
	{
		nextThread = mutexToPull->low_prio_MutexQueue.front();
		mutexToPull->low_prio_MutexQueue.pop_front();
	}

	return nextThread;

}

TVMStatus VMMutexAcquire(TVMMutexID mutex, TVMTick timeout)
{
	if (mutex < 0 || mutex >= allMutexs.size())
		return VM_STATUS_ERROR_INVALID_ID;
	else if (allMutexs[mutex] == NULL)
		return VM_STATUS_ERROR_INVALID_ID;

	TMachineSignalState sigState;
	MachineSuspendSignals(&sigState);

	TCBRef to_Aquire = CURRENT_THREAD;
	MutexRef mutexToGet = allMutexs[mutex];

//	std::cout << "-------Thread: " << to_Aquire->t_id << " Attempting to grab Mutex: " << mutexToGet->mutexID << " With Lock Status: " << mutexToGet->lockValue << "\n";

	if (timeout == VM_TIMEOUT_IMMEDIATE)
	{
		
		if (mutexToGet->lockValue == 1)
		{
			return VM_STATUS_FAILURE;
		}
		else
		{
			mutexToGet->lockValue = 1;
			mutexToGet->ownerThread = to_Aquire;
			MachineResumeSignals(&sigState);
			return VM_STATUS_SUCCESS;
		}
	}
	else if (timeout == VM_TIMEOUT_INFINITE)
	{

		if (mutexToGet->lockValue == 1)
		{
	//		std::cout << "<<<<Mutex Already Aquired by: " << mutexToGet->ownerThread->t_id << ">>>>\n";
			to_Aquire->t_state = VM_THREAD_STATE_WAITING;
			to_Aquire->t_tick = VM_TIMEOUT_INFINITE;
			pushThreadToMutexQueue(to_Aquire, mutexToGet);
	//		std::cout << "Going to schedule after pushing down thread: " << to_Aquire->t_id << " with mutex: " << mutexToGet->mutexID << "\n";
			scheduleThread();
		}
		else
		{
			mutexToGet->lockValue = 1;
			mutexToGet->ownerThread = to_Aquire;
	//		std::cout << "---------Locking thread: " << to_Aquire->t_id << " With Mutex: " << mutexToGet->mutexID << " Lock Value now: " << mutexToGet->lockValue << "\n";
			MachineResumeSignals(&sigState);
			return VM_STATUS_SUCCESS;
		}
	}
	else
	{
		if (mutexToGet->lockValue == 1)
		{
			pushThreadToMutexQueue(to_Aquire, mutexToGet);
			to_Aquire->t_tick = timeout;
			to_Aquire->t_state = VM_THREAD_STATE_WAITING;
			to_Aquire->s_id = sleepingThreads.size();
			SLEEPING_THREADS++;
			sleepingThreads.push_back(to_Aquire);
			scheduleThread();

			if (mutexToGet->lockValue == 1)
			{
				MachineResumeSignals(&sigState);
				return VM_STATUS_FAILURE;
			}
		}	
		mutexToGet->lockValue = 1;
		mutexToGet->ownerThread = to_Aquire;
		MachineResumeSignals(&sigState);
		return VM_STATUS_SUCCESS;
	}
//	std::cout << "Resuming signals from acquire....\n";
	MachineResumeSignals(&sigState);
	return VM_STATUS_SUCCESS;
}
TVMStatus VMMutexRelease(TVMMutexID mutex)
{
	if (mutex < 0 || mutex >= allMutexs.size())
		return VM_STATUS_ERROR_INVALID_ID;
	else if (allMutexs[mutex] == NULL)
		return VM_STATUS_ERROR_INVALID_ID;
	TMachineSignalState sigState;
	MutexRef toRelease = allMutexs[mutex];
	TCBRef holding_Thread = CURRENT_THREAD;
//	std::cout << "Releasing mutex: " << toRelease->mutexID << " With Thread: " << holding_Thread->t_id << "///////////\n";
	if (toRelease->ownerThread != holding_Thread)
		return VM_STATUS_ERROR_INVALID_STATE;
	else
	{
		
		MachineSuspendSignals(&sigState);
		toRelease->lockValue = 0;
		toRelease->ownerThread = NULL;
		
		if (toRelease->t_isWaiting > 0)
		{
			TCBRef toSchedule = pullThreadFromMutexQueue(toRelease);
			if (toSchedule->t_tick != VM_TIMEOUT_INFINITE)
			{
				scheduleSleeperThread(toSchedule);
			}
			else
			{
				toSchedule->t_state = VM_THREAD_STATE_READY;
				pushDownReady(toSchedule);
		
				scheduleThread();
			}
		}

		
	}

	MachineResumeSignals(&sigState);
	return VM_STATUS_SUCCESS;
}

void pushFileToWait(TCBRef toWait)
{
//	std::cout << "___CALLED PUSH TO WAIT FOR THREAD: " << toWait->t_id << "___\n";
	toWait->t_state = VM_THREAD_STATE_WAITING;
	toWait->s_id = IOWaiting.size();	
//	std::cout << "___STATE SET S_ID GOT:" << toWait->s_id << "___\n";
	IOWAITING_THREADS++;
//	std::cout << "___PUSHING TO WAIT___\n";
	IOWaiting.push_back(toWait);
}

//VM FILE 4. file.so: (VMFileOpen, VMFileSeek, VMFileRead, and VMFileClose)
TVMStatus VMFileOpen(const char *filename, int flags, int mode, int *filedescriptor)
{
//	std::cout << "___AT VERY TOP OF VMFILEOPEN___\n";
	if (filename == NULL || filedescriptor == NULL)
		return VM_STATUS_ERROR_INVALID_PARAMETER;

	//Suspend Signals
	TMachineSignalState sigState;
	MachineSuspendSignals(&sigState);

	
	TCBRef fileThread = allThreads[CURRENT_THREAD->t_id];

	//Call the machine file open, the FD will be put into thread->fileResult
//	std::cout << "___CALLING MACHINE FILE OPEN___\n";
	MachineFileOpen(filename, flags, mode, fileCallBack, fileThread);	//IF PROBLEMS, PUT SCHEDULE BEHIND OPEN
	//Push file to waiting list
	pushFileToWait(fileThread);
//	std::cout << "___MADE IT OUT OF PUSHFILETOWAIT___\n";
//	std::cout << "___SCHEDULING THREADS FROM OPEN___\n";
	scheduleThread();
	
	//Resume Signals
	
	MachineResumeSignals(&sigState);
	if (fileThread->t_fileResult < 0)
		return VM_STATUS_FAILURE;
	else
	{
//		std::cout << "set filedescriptor to: " << fileThread->t_fileResult << "=-=-=-=\n";
		*filedescriptor = fileThread->t_fileResult;
		return VM_STATUS_SUCCESS;
	}
}
TVMStatus VMFileClose(int filedescriptor)
{
	//Suspend Signals
	TMachineSignalState sigState;
	MachineSuspendSignals(&sigState);

	TCBRef fileThread = allThreads[CURRENT_THREAD->t_id];

	//Call the machine to close a file, 
	MachineFileClose(filedescriptor, fileCallBack, fileThread);//IF PROBLEMS, PUT SCHEDULE BEHIND OPEN
	//Put file to wait
	pushFileToWait(fileThread);
	
	scheduleThread();

	//Resume Signals
	
	MachineResumeSignals(&sigState);
	if (fileThread->t_fileResult < 0)
		return VM_STATUS_FAILURE;
	else
	{
		return VM_STATUS_SUCCESS;
	}
	return VM_STATUS_SUCCESS;	
}      
TVMStatus VMFileRead(int filedescriptor, void *data, int *length)
{
	if (data == NULL || length == NULL)
		return VM_STATUS_ERROR_INVALID_PARAMETER;

	//Suspend Signals
	TMachineSignalState sigState;
	MachineSuspendSignals(&sigState);

	TCBRef fileThread = allThreads[CURRENT_THREAD->t_id];

	//Call the machine to read a file, the # of bytes transfered will be put into thread->fileResult
	MachineFileRead(filedescriptor, data, *length, fileCallBack, fileThread);//IF PROBLEMS, PUT SCHEDULE BEHIND OPEN
	//Put file to wait
	pushFileToWait(fileThread);
	
	scheduleThread();

	//Resume Signals
	
	MachineResumeSignals(&sigState);
	if (fileThread->t_fileResult < 0)
		return VM_STATUS_FAILURE;
	else
	{
		*length = fileThread->t_fileResult;
		return VM_STATUS_SUCCESS;
	}
	
}
TVMStatus VMFileWrite(int filedescriptor, void *data, int *length)
{
//	std::cout << "___AT VERY TOP OF VMFILEWRITE___\n";
	if (data == NULL || length == NULL)
		return VM_STATUS_ERROR_INVALID_PARAMETER;

	//Suspend Signals
	TMachineSignalState sigState;
	MachineSuspendSignals(&sigState);
	
	TCBRef fileThread = allThreads[CURRENT_THREAD->t_id];
//	std::cout << "___WRITING WITH THREAD: " << fileThread->t_id << "___\n";
	//void MachineFileWrite(int fd, void *data, int length, TMachineFileCallback callback, void *calldata);
	//Call the machine to write to a file, the the length transfered will be put into thread->fileResult
	MachineFileWrite(filedescriptor, data, *length, fileCallBack, fileThread);//IF PROBLEMS, PUT SCHEDULE BEHIND OPEN
	//Put file to wait
	pushFileToWait(fileThread);
//	std::cout << "___MADE IT OUT OF PUSHFILETOWAIT___\n";
	//std::cout << "___SCHEDULING THREADS FROM WRITE___\n";
	

	scheduleThread();

	//Resume Signals
	MachineResumeSignals(&sigState);
	if (fileThread->t_fileResult < 0)
		return VM_STATUS_FAILURE;
	else
	{
		*length = fileThread->t_fileResult;
		return VM_STATUS_SUCCESS;
	}
}
TVMStatus VMFileSeek(int filedescriptor, int offset, int whence, int *newoffset)
{
	if (newoffset != NULL)
	{
		//Suspend Signals
		TMachineSignalState sigState;
		MachineSuspendSignals(&sigState);

		TCBRef fileThread = allThreads[CURRENT_THREAD->t_id];
	
		//Call the machine to read a file, the # of bytes transfered will be put into thread->fileResult
		MachineFileSeek(filedescriptor, offset, whence, fileCallBack, fileThread);//IF PROBLEMS, PUT SCHEDULE BEHIND OPEN
		//Put file to wait
		pushFileToWait(fileThread);
		
		scheduleThread();

		//Resume Signals
		
		MachineResumeSignals(&sigState);
		if (fileThread->t_fileResult < 0)
			return VM_STATUS_FAILURE;
		else
		{
			*newoffset = fileThread->t_fileResult;
			return VM_STATUS_SUCCESS;
		}
	}
	//
	return VM_STATUS_SUCCESS;

}



}//C EXTERN
