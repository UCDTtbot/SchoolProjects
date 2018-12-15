/*


  order of getting it to work:
  1. hello,  [DONE]
  2. sleep,  [DONE]
  3. thread, 
  4. file, 
  5. mutex
*/


#include "VirtualMachine.h"
#include "Machine.h"
#include <cstdlib>
#include <unistd.h>
#include <vector>
#include <queue>
#include <iostream>

using namespace std;

#define VM_THREAD_PRIORITY_IDLE ((TVMThreadPriority)0x00)

typedef struct ThreadControlBlock{
    TVMThreadID t_id;
    TVMThreadState t_state;
    TVMThreadPriority t_prio;
    TVMThreadEntry t_entry;
    void* entryParam;
    TVMMemorySize t_memSize;
    TVMTick t_tick;
    TVMMutexID t_mutexid;
    void* stackptr;
    size_t stacksize;
    SMachineContext mcntx;

    ThreadControlBlock (){} // constructor, no param
    ThreadControlBlock (TVMThreadID tid,
                        TVMThreadState state,
                        TVMThreadPriority prio,
                        TVMMemorySize memSize,
                        TVMThreadEntry entry,
                        void* param) { 
        t_id = tid;
        t_state = state;
        t_prio = prio;
        t_memSize = memSize;
        t_entry = entry;
        t_entry = param;
    } // constructor, with param
};

typedef ThreadControlBlock* TCBRef;
typedef vector<TCBRef> TCBvector; // store all threads
typedef queue<TCBRef> TCBqueue; // priority queues (for ready threads)

TCBvector allThreads;
TCBqueue low_prio_thread;
TCBqueue normal_prio_thread;
TCBqueue high_prio_thread;

static volatile TCBRef currthread;
static volatile int GlobalTickCounter;
static volatile bool isIdle;

extern "C" {

// push the ready thread into its corresponding ready queue (based on priority level)
void pushready(TCBRef thread) {
    switch(thread->t_prio) {
        case VM_THREAD_PRIORITY_LOW: low_prio_thread.push(thread); break;
        case VM_THREAD_PRIORITY_NORMAL: normal_prio_thread.push(thread); break;
        case VM_THREAD_PRIORITY_HIGH: high_prio_thread.push(thread); break;
    } // switch
}

// get the first TCBRef in the queue with corresponding priority
// return NULL if corresponding queue is empty
TCBRef getnextready() {
    TCBRef next = NULL;

    if(!high_prio_thread.empty()) {
        next = high_prio_thread.front(); 
        high_prio_thread.pop(); 
    }
    else if(!normal_prio_thread.empty()) {
        next = normal_prio_thread.front(); 
        normal_prio_thread.pop(); 
    }
    else if(!low_prio_thread.empty()) {
        next = low_prio_thread.front(); 
        low_prio_thread.pop(); 
    }

    return next;
}

void idleEntry(void* param) {
    isIdle = true;
    while(isIdle);
}

// wake up from idle thread
void idleCallBack(void* param) {
    isIdle = false;
}

/////////////// Thread.c /////////////////
TVMStatus VMThreadTerminate(TVMThreadID thread){
    VMPrint("Enter thread terminate\n");
};

void skeletonThreadEntry (void* param) { // param == thread id
    TVMThreadID tid = (TVMThreadID) param;
    TCBRef tcb = allThreads[tid];
    tcb->t_entry(tcb->entryParam);
    VMThreadTerminate(tid);
}

// incoming thread has higher prio than current thread
// schedule & prepare for context switching (switching thread)
void threadSchedule(TCBRef thisthread) {
    // need to call MachineContextSwitch

    thisthread->t_state = VM_THREAD_STATE_READY;
    pushready(thisthread);

    TCBRef next = getnextready();
        
    next->t_state = VM_THREAD_STATE_RUNNING;
    MachineContextSwitch(&(currthread->mcntx), &(next->mcntx));

} // threadSchedule

TVMStatus VMThreadCreate(TVMThreadEntry entry, void *param, TVMMemorySize memsize, TVMThreadPriority prio, TVMThreadIDRef tid) {
    
    if(entry == NULL || tid == NULL)
        return VM_STATUS_ERROR_INVALID_PARAMETER;

    TMachineSignalState sigstate;

    // suspend signals
    MachineSuspendSignals(&sigstate);

    // init thread
    TCBRef tcb = new ThreadControlBlock();
    tcb->t_entry = entry;
    tcb->entryParam = param;
    tcb->t_memSize = memsize;
    tcb->t_prio = prio;
    tcb->t_state = VM_THREAD_STATE_DEAD;
    // using vector size to get tid & update param's tid
    *tid = allThreads.size();
    tcb->t_id = *tid;
    // store thread obj
    allThreads.push_back(tcb);

    // resume signals
    MachineResumeSignals(&sigstate);

    return VM_STATUS_SUCCESS;

}; // VMThreadCreate

TVMStatus VMThreadState(TVMThreadID thread, TVMThreadStateRef stateref){
    
    if(stateref == NULL)
        return VM_STATUS_ERROR_INVALID_PARAMETER;

    if(thread >= 0 && thread < allThreads.size()) {
        TCBRef thisthread = allThreads[(int)thread];
        *stateref = thisthread->t_state;
        return VM_STATUS_SUCCESS;
    }
    else
        return VM_STATUS_ERROR_INVALID_PARAMETER;

}; // VMThreadState

// activates a dead thread
TVMStatus VMThreadActivate(TVMThreadID thread){

    if(thread < 0 || thread >= allThreads.size())
        return VM_STATUS_ERROR_INVALID_PARAMETER;
    
    TCBRef thisthread = allThreads[thread];

    if(thisthread->t_state != VM_THREAD_STATE_DEAD)
        return VM_STATUS_ERROR_INVALID_STATE;

    // enters ready state & begin at the entry function specified
    TMachineSignalState sigstate;

    // suspend signals
    MachineSuspendSignals(&sigstate);

    // context create with callback in skeleton
    TVMThreadEntry skeleton = skeletonThreadEntry;
    MachineContextCreate(&(thisthread->mcntx), skeleton, (void*)thisthread->t_id, thisthread->stackptr, thisthread->stacksize);

    // change thread's state to ready
    thisthread->t_state = VM_THREAD_STATE_READY;

    // push in queue based on priority
    //     - handle higher prios coming in
    //     - schedule if necessary
    threadSchedule(thisthread);
/*    
    if(currthread->t_prio >= thisthread->t_prio) {
        // not of higher prio, put next into ready queue
        pushready(thisthread);
    }
    else {
        // higher prio thread comes in (next)
        // switch to next immediately; scheduler
        threadSchedule(thisthread);
    }
*/
    // resume signals
    MachineResumeSignals(&sigstate);

    return VM_STATUS_SUCCESS;

}; // VMThreadActivate


/////////////// Sleep.c //////////////////
TVMStatus VMThreadSleep(TVMTick tick) {

    if(tick == VM_TIMEOUT_INFINITE)
        return VM_STATUS_ERROR_INVALID_PARAMETER;

    currthread->t_tick = tick;
    
    // wait while tick is not zero yet;
    while(currthread->t_tick > 0) {
        cout << currthread->t_tick << endl;
    }

    return VM_STATUS_SUCCESS;

}; // VMThreadSleep

void AlarmCallback(void *param) {
    cout << "alarm call back is called!" << endl;
    for(unsigned int i = 0; i < allThreads.size(); i++)
        allThreads[i]->t_tick -= 1;
/*
    if(GlobalTickCounter > 0)
        GlobalTickCounter--;
*/
};

/////////////// Hello.c //////////////////
TVMMainEntry VMLoadModule(const char* module);

TVMStatus VMStart(int tickms, int machinetickms, int argc, char *argv[]) {

    TVMMainEntry VMMain;
    TMachineAlarmCallback SleepACB = AlarmCallback; // for sleep.so

    VMMain = VMLoadModule(argv[0]);

    // check to make sure the return address from vmloadmod wasn't an error value
    if(VMMain != NULL) {
        MachineInitialize(machinetickms); // timeout = #in millisec that machine will sleep
        cout << "Request ALarm" << endl;
        MachineRequestAlarm(machinetickms*1000, SleepACB, NULL); // in second
        MachineEnableSignals();

		/*    ThreadControlBlock (TVMThreadID tid,
                        TVMThreadState state,
                        TVMThreadPriority prio,
                        TVMMemorySize memSize,
                        TVMThreadEntry entry,
                        void* param) { 
		*/
        // create the Main thread
        TCBRef mainthread = new ThreadControlBlock((TVMThreadID)0, VM_THREAD_STATE_RUNNING, VM_THREAD_PRIORITY_NORMAL, 0, NULL, NULL);
        allThreads.push_back(mainthread);
        currthread = mainthread;
        
        // create the idle thread (its state should either be ready or running)
        TVMThreadEntry ientry = idleEntry;
        TCBRef idlethread = new ThreadControlBlock((TVMThreadID)1, VM_THREAD_STATE_READY, VM_THREAD_PRIORITY_IDLE, 0x100000, ientry, NULL);
        MachineContextCreate(&(idlethread->mcntx), ientry, NULL, idlethread->stackptr, idlethread->stacksize);
        allThreads.push_back(idlethread);

        VMMain(argc, argv);
        return VM_STATUS_SUCCESS;
    }
    else
        return VM_STATUS_FAILURE;

}; // VMStart

TVMStatus VMFileWrite(int filedescriptor, void *data, int *length){

    if(data == NULL || length == NULL)
        return VM_STATUS_ERROR_INVALID_PARAMETER;

    char* string = (char*) data;
    int res = write(filedescriptor, string, *length);

    if(res == -1)
        return VM_STATUS_FAILURE;
    else
        return VM_STATUS_SUCCESS;

}; // VMFileWrite








} // extern C
