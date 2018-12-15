#include "VirtualMachine.h"
#include "Machine.h"
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <vector>
#include <queue>
#include <list>
#include <map>
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
	}					//Overloaded Constructor
} ThreadControlBlock;

const TVMMemoryPoolID VM_MEMORY_POOL_ID_SYSTEM = 0;	//MAIN MEMORY POOL CREATED IN VMSTART
const TVMMemoryPoolID VM_MEMORY_POOL_ID_SHARED = 1;

typedef struct memChunk		//STRUCT FOR CHUNKS OF MEMORY CURRENTLY ALLOCATED
{
	TVMMemoryPoolID pooling_from;	//ID OF WHICH POOL CHUNK IS ALLOCATING FROM
	TVMMemorySize chunk_size;		//SIZE OF CHUNK
	uint8_t *mem_base;				//PTR TO WHERE THE CHUNK IS

	memChunk(){}
	memChunk(TVMMemoryPoolID to_pool, TVMMemorySize size, void* base)
	{
		pooling_from = to_pool;	//CONSTRUCTOR ASSIGNMENT
		chunk_size = size;
		mem_base = (uint8_t *) base;
	}
}memChunk;

typedef memChunk* chunkRef;	//REFERENCE TO MEMORY CHUNKS
typedef std::vector<chunkRef> ChunkVector; //CHUNK VECTOR
ChunkVector chunksInUse;
std::list<chunkRef>::iterator listIter1;
std::list<chunkRef>::iterator listIter2;
std::list<chunkRef>::iterator listIter3;

typedef std::map<uint8_t*, TVMMemorySize> memoryMaps;	//MAP TYPEDEFS
memoryMaps::iterator memIter;
memoryMaps::iterator memIter1;
memoryMaps::iterator memIter2;
typedef std::map<TVMMemoryPoolID, unsigned int> IDmaps;	//A TYPEDEF TO MAP LONG IDS TO SMALLER IDS
IDmaps poolIDS;
TVMMemoryPoolID Shared_StorageID;

typedef struct memPool	//MAIN STRUCT FOR MEMORY POOL
{
	TVMMemoryPoolID pool_id;	//ID OF THE POOL
	TVMMemorySize pool_size;	//TOTAL SIZE OF POOL
	TVMMemorySize totalFree_space;	//TOTAL CURRENT FREE SPACE
	TVMMemorySizeRef totalFree_spaceRef;
	TVMMemorySize totalAllocated_space; //TOTAL CURRENT ALLOCATED SPACE
	TVMMemorySizeRef totalAllocated_spaceRef;
	uint8_t *pool_base; //pool_base = (uint8_t *)base	BASE POINTER OF THE MEM POOL
	//Need a list of free spaces. When a chunk becomes deallocated - the chunk is moved to the free spaces list
	//SO USE A LIST FOR FREE SPACES
	//Sort free spaces 
	//You need to keep track of the free spaces. This will be a list of the locations and their sizes.
	//The total of the sizes of the free locations should also be kept in a variable so that you don't have to iterate through the list each time to get the total free space.
	//memoryMaps free_chunks;		//MAP OF FREE CHUNKS
	std::list<chunkRef> free_chunks;
	//AND USE A MAP FOR ALLOCATED SPACES
	//Need a mapping of allocated space. This needs a ptr to the base of the section and a size
	memoryMaps allocated_chunks;//MAP OF ALLOCATED CHUNKS
//for sharef mem just use memcopy

	//Constructor for easy creation
	memPool(){}
	memPool(TVMMemoryPoolID poolID, TVMMemorySize poolSize, void* base)
	{						
		pool_id = poolID;					//ASSIGN ID TO PARAMETER
		pool_size = poolSize;				//ASSIGN SIZE TO PARAMETER
		totalFree_space = pool_size;		//TOTALFREE_SPACE IS CURRENT POOL_SIZE
		pool_base = (uint8_t *)base;		//ASSIGN BASE PTR TO PARAMETER
		totalFree_spaceRef = &totalFree_space;
		totalAllocated_spaceRef = &totalAllocated_space;
		chunkRef fullChunk = new memChunk(poolID, poolSize, base);
		free_chunks.push_back(fullChunk);	//SET THE FREE CHUNKS TO FULL MEMORY POOL
	}

} memPool;

typedef memPool* memoryRef;	//REFERENCE TO MEMORY POOL
typedef std::vector<memoryRef> MEMVector;	//MEMORY POOL VECTOR
MEMVector allMemPools;



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

TCBQueue low_prio_AlloQueue;
TCBQueue norm_prio_AlloQueue;
TCBQueue high_prio_AlloQueue;

static volatile TCBRef CURRENT_THREAD;	//Keep track of current running thread
static volatile TCBRef IDLE_THREAD;
static volatile int SLEEPING_THREADS = 0;	//Global Tick used for the basic sleep
static volatile int IOWAITING_THREADS = 0;
static volatile bool IS_IDLE = false;				//Global idle check for current thread

TVMMainEntry VMLoadModule(const char *module);
void VMUnloadModule(void);


/////////////////////////////////////////////////////////////////////////
////////////////              UTILITY              //////////////////////
/////////////////////////////////////////////////////////////////////////
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

	toPush->t_state = VM_THREAD_STATE_READY;
	pushDownReady(toPush);			//Push to correct queue

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
	//std::cout << "Leaving callback............." << std::endl;
}

/////////////////////////////////////////////////////////////////////////
////////////////              MEMORY               //////////////////////
/////////////////////////////////////////////////////////////////////////
bool sortChunks(chunkRef first, chunkRef second)
{
	return ((uint64_t) first->mem_base < (uint64_t) second->mem_base);
}

void printAll(const char *call)
{
	std::cout << "\n=======================\n";
	std::cout << "Print All Call: " << call << std::endl;
	for (unsigned int x = 0; x < allThreads.size(); x++)
	{
		std::cout << "Thread ID: " << allThreads[x]->t_id << " Thread State: " << allThreads[x]->t_state << std::endl;
	}
	for (unsigned int x = 0; x < allMemPools.size(); x++)
	{
		std::cout << "---------------------------------" << std::endl;
		std::cout << "Mem ID: " << allMemPools[x]->pool_id << " Mem Size: " << allMemPools[x]->pool_size << " Base: " << (uint64_t) allMemPools[x]->pool_base << " Free Space: " << allMemPools[x]->totalFree_space << std::endl;
		//std::cout << "First Free: " << &allMemPools[x]->first_free << std::endl;
		//for (memIter = allMemPools[x]->free_chunks.begin(); memIter != allMemPools[x]->free_chunks.end(); memIter++)
		//{
		//	std::cout << "Free Chunks Size: " << allMemPools[x]->free_chunks.size() << " Free Chunk Base: " << &memIter->first << " Free Chunk Size: " << memIter->second << std::endl;
		//}
		for (memIter = allMemPools[x]->allocated_chunks.begin(); memIter != allMemPools[x]->allocated_chunks.end(); memIter++)
		{
			std::cout << "Allocated Size: " << allMemPools[x]->allocated_chunks.size() << " Allocated Chunk Base: " << (uint64_t) memIter->first << " Allocated Chunk Size: " << memIter->second << std::endl;
		}
	}
	std::cout << "=======================\n";

}

void checkAlloQueues()
{
	TCBRef toSched = NULL;
	if (!(high_prio_AlloQueue.empty()))
	{
		toSched = high_prio_AlloQueue.front();
		high_prio_AlloQueue.pop_front();
		toSched->t_state = VM_THREAD_STATE_READY;
		pushDownReady(toSched);
	}
	else if (!(norm_prio_AlloQueue.empty()))
	{
		toSched = norm_prio_AlloQueue.front();
		norm_prio_AlloQueue.pop_front();
		toSched->t_state = VM_THREAD_STATE_READY;
		pushDownReady(toSched);
	}
	else if (!(low_prio_AlloQueue.empty()))
	{
		toSched = low_prio_AlloQueue.front();
		low_prio_AlloQueue.pop_front();
		toSched->t_state = VM_THREAD_STATE_READY;
		pushDownReady(toSched);
	}
}

TVMStatus VMMemoryPoolCreate(void *base, TVMMemorySize size, TVMMemoryPoolIDRef memory)
{
	TMachineSignalState sigState; //Create variable to hold current signal state

	if (base == NULL || memory == NULL || size == 0)
		return VM_STATUS_ERROR_INVALID_PARAMETER;
	

	//Suspend the signals
	MachineSuspendSignals(&sigState);
	//memPool(TVMMemoryPoolID poolID, TVMemorySize poolSize, void* base)
	memoryRef newPool = new memPool(*memory, size, base);	//create new pool using parameters of the function
	allMemPools.push_back(newPool);	//push to full list of pools
//	std::cout << "poolIDS Size Before (Should be 1): " << poolIDS.size() << std::endl;
	poolIDS[*memory] = (poolIDS.size() - 1); //setup the mapping for ID to easier nums

//	const char *call = "Pool Create";
//	printAll(call);

	MachineResumeSignals(&sigState);

	return VM_STATUS_SUCCESS;
}

TVMStatus VMMemoryPoolDelete(TVMMemoryPoolID memory)
{

	TMachineSignalState sigState;

	if (memory < 0 || poolIDS.find(memory) == poolIDS.end())	//error check
		return VM_STATUS_ERROR_INVALID_PARAMETER;

   
	if (allMemPools[poolIDS.find(memory)->second]->totalFree_space != allMemPools[poolIDS.find(memory)->second]->pool_size)
		return VM_STATUS_ERROR_INVALID_STATE;

	MachineSuspendSignals(&sigState);

	//std::cout << "Deleting....." << std::endl;

	delete allMemPools[poolIDS.find(memory)->second];	//delete the memory pool

    allMemPools[poolIDS.find(memory)->second] = NULL;	//remove the memory pool from the vector
	poolIDS.erase(poolIDS.find(memory));			//erase the ID to simpleID mapping



	MachineResumeSignals(&sigState);

	return VM_STATUS_SUCCESS;
}

TVMStatus VMMemoryPoolQuery(TVMMemoryPoolID memory, TVMMemorySizeRef bytesleft)
{
	TMachineSignalState sigState;

	if (memory < 0 || poolIDS.find(memory) == poolIDS.end() || bytesleft == NULL)	//error checking
		return VM_STATUS_ERROR_INVALID_PARAMETER;

	MachineSuspendSignals(&sigState);

	*bytesleft = allMemPools[poolIDS.find(memory)->second]->totalFree_space;	//set bytes left equal to free_space of pool

	//const char *call = "Pool Query";
	//printAll(call);

	MachineResumeSignals(&sigState);

	return VM_STATUS_SUCCESS;
}



TVMStatus VMMemoryPoolAllocate(TVMMemoryPoolID memory, TVMMemorySize size, void **pointer)
{
	if (memory < 0 || poolIDS.find(memory) == poolIDS.end() || size == 0 || pointer == NULL)	//error check
	{
		std::cout << "<<<<<<<<<SHIT>>>>>>>>>>" << std::endl;
		std::cout << "Pool: " << memory << " Size: " << size << " Pointer: " << (uint64_t)pointer << std::endl;
		return VM_STATUS_ERROR_INVALID_PARAMETER;
	}

	TVMMemorySize adjSize = ((size + 0x3F) & (~0x3F));

	//const char *call = "Pool Allocate";
	//printAll(call);

	TMachineSignalState sigState;
	MachineSuspendSignals(&sigState);
	
	memoryRef pool = allMemPools[poolIDS.find(memory)->second];	//retrieve pool

	if (pool->totalFree_space < adjSize) // Error check with rounding (might wanna make the stupid size+0x3F & ~0x3F just a friken variable so I stop copy pasting)
	{
		std::cout << "<<<<<<<<<NO RESOURCES>>>>>>>>>>" << std::endl;
		return VM_STATUS_ERROR_INSUFFICIENT_RESOURCES;
	}
	listIter1 = pool->free_chunks.begin();
	while ((*listIter1)->chunk_size < adjSize && listIter1 != pool->free_chunks.end())
	{
		listIter1++;
	}
					//memChunk(TVMMemoryPoolID to_pool, TVMMemorySize size, void* base)
	chunkRef newChunk = new memChunk(pool->pool_id, adjSize, (*listIter1)->mem_base);	//Create a memory chunk, and assign it shit
	chunksInUse.push_back(newChunk);	//push chunk to chunks vector
	//std::cout << "chunksInUse size = " << chunksInUse.size() << " Should be ~" << calls+1 << std::endl;
	//ALRIGHT	new chunk has an ID of which pool its taking from, how big the chunk is, and where it points which will be first free chunk
	//Now we must take up the space memChunk just took up via taking first_free+size and 'deleting' this from free. This can be done by taking first_free+size and adding this free_chunks[first_free]->second - size? 
	//So free_chunks[first_free] will give us first = ptr_to_space and second = size. if we have a huge block with ptr1 and tSize = 256 with ptr2 and nSize = 64 we would do:
	//ptr2 = &ptr1 + nSize; free_chunks[ptr2] = tSize - nSize; This moves the ptr up 64 bits, makes the total size smaller, and now we put what we just took away into allocated soooooooooooooooooooooooooo
	//allocated_chunks
	chunkRef newFreeChunk = new memChunk(pool->pool_id, pool->totalFree_space - adjSize, newChunk->mem_base + adjSize);
	pool->free_chunks.push_back(newFreeChunk);
	//std::cout << "\n New free:   " << (uint64_t) new_free << " ___ " << std::endl;
	//std::cout << ".................." <<  pool->free_chunks.erase(pool->first_free) << " deleted \n";
//	std::cout << "Size: " << size << " With bitwise: " << ((size + 0x3F) & (~0x3F)) << std::endl;	//see above
	listIter1 = pool->free_chunks.begin();
	bool found = false;
	while (listIter1 != pool->free_chunks.end() && !found)
	{
		if ((*listIter1)->mem_base == newChunk->mem_base)
		{
			delete *listIter1;
			pool->free_chunks.erase(listIter1);
		}

		listIter1++;
		found = true;
	}
	//std::cout << " First free: " << (uint64_t) pool->first_free << " ___ \n" << std::endl;
	pool->totalFree_space = pool->totalFree_space - adjSize;
	pool->allocated_chunks[newChunk->mem_base] = adjSize;
	pool->totalAllocated_space = pool->pool_size - pool->totalFree_space;	//see above
	*pointer = newChunk->mem_base;
	//std::cout << "\n___Allo_FreeChunks After: ";
	//for (memIter = pool->free_chunks.begin(); memIter != pool->free_chunks.end(); memIter++)
	//{
	//	std::cout << " Free Chunk Base: " << (uint64_t) memIter->first << " Free Chunk Size: " << memIter->second << std::endl;
	//}
	pool->free_chunks.sort(sortChunks);
	MachineResumeSignals(&sigState);

	return VM_STATUS_SUCCESS;
}

TVMStatus VMMemoryPoolDeallocate(TVMMemoryPoolID memory, void *pointer)
{
	if (memory < 0 || poolIDS.find(memory) == poolIDS.end() || pointer == NULL)
	{
		std::cout << "<<<<<<<<<SHITS FUCKED>>>>>>>>>>" << std::endl;
		std::cout << "Memory: " << memory << " Pointer: " << &pointer << std::endl;	//error checking
		return VM_STATUS_ERROR_INVALID_PARAMETER;
	}

	TMachineSignalState sigState;
	MachineSuspendSignals(&sigState);
	//std::cout << "\nMem id: " << memory << std::endl;
	//std::cout << "Chunks in use: " << chunksInUse.size() << std::endl;
	
	//const char *call = "Pool Deallocate";
	//printAll(call);

	memoryRef pool = allMemPools[poolIDS.find(memory)->second];	//and the pool to deallocate
	unsigned int chunkSpot = 0;
	for (unsigned int x = 0; x < chunksInUse.size(); x++)
	{
		if (chunksInUse[x]->mem_base == pointer && chunksInUse[x]->pooling_from == pool->pool_id)	//find the chunk to deallocate
		{
			chunkSpot = x;
		}
	}
	//std::cout << "\n Found Chunk" << std::endl;

	chunkRef chunk = chunksInUse[chunkSpot];
	//DEALLOCATE TIME GOD DAMMIT. Ok so we have a chunk that needs to be deleted. After the chunk has been deleted, we set its position in the vector to NULL
	//After the chunk is taken care of, we must set the allocated space ptr to that chunk to null and delete the mapping via pool->allocated_chunks.erase(find the damn thing)
	//Once that is erased, subtract the total allocated space down. SO now we have a deleted chunk, WAIT KEEP THE PTR TO BASE FOR NOW, the allocated mapping deleted, and allocated space subtracted

	//delete chunksInUse[chunkSpot];
	chunksInUse.erase(chunksInUse.begin() + chunkSpot);
	//std::cout << " Chunk Erased" << std::endl;
	//std::cout << "\nDeleted chunk: " << chunkSpot << " pooling from: " << pool->pool_id << " with base: " << (uint64_t) curBase << " ___ " << std::endl;

	pool->allocated_chunks.erase(chunk->mem_base); 
	pool->totalAllocated_space = pool->totalAllocated_space - chunk->chunk_size;
	//Now we must do free. Do the inverse of what we just did. How the fuck do you merge though..... ok first free
	//Now that the chunks ptr is deleted from the allocated mapping, move it to the free mapping. Kinda like the inverse of allocation
	pool->free_chunks.push_back(chunk);
	pool->totalFree_space = pool->totalFree_space + chunk->chunk_size;
	//std::cout << " About to sort" << std::endl;
	pool->free_chunks.sort(sortChunks);
//	std::cout << " Sorted" << std::endl;
	//Alright cool shit^^^ this should work. Uhhh it compiles. BUT SEGFAULTS AT PRINTING VALUES THIS IS BECAUSE MERGING ISN'T DONE
	//for (memIter = allMemPools[x]->free_chunks.begin(); memIter != allMemPools[x]->free_chunks.end(); memIter++)
	listIter1 = pool->free_chunks.begin();
	listIter2 = pool->free_chunks.begin();
//	std::cout << " Iter Set" << std::endl;
	if (listIter2 != pool->free_chunks.end())
		listIter2++;
//	std::cout << " Going Into While=================================================" << std::endl;
	while(listIter1 != pool->free_chunks.end())
	{
//		std::cout << "Sorted free chunks by base: " << std::endl;
		for (listIter3 = pool->free_chunks.begin(); listIter3 != pool->free_chunks.end(); ++listIter3)
		{
		//	std::cout << "Chunk: " << (uint64_t)(*listIter3)->mem_base << "::" << (*listIter3)->chunk_size << std::endl;;
		}
		//std::cout << "Theoretical end is: " << (uint64_t)(*pool->free_chunks.end())->mem_base;
		pool->free_chunks.sort(sortChunks);
	//	if (listIter1 == pool->free_chunks.begin())
			//std::cout << "\n listIter1 at BEGIN" << std::endl;
	//	if (listIter2 == pool->free_chunks.begin())
		//	std::cout << " listIter2 at BEGIN" << std::endl;
	//	if (listIter2 == pool->free_chunks.end())
		//	std::cout << " listIter2 at END" << std::endl;
		//if (listIter1 == pool->free_chunks.end())
		//	std::cout << " listIter1 at END" << std::endl;
	//	uint8_t* dummyB = (*listIter1)->mem_base;		
	//	std::cout << " Passed dummyB check" << std::endl;
	//	TVMMemorySize dummyS = (*listIter1)->chunk_size;
	//	std::cout << " Passed dummyS check" << std::endl;
	//	uint8_t* dummyB2 = (*listIter2)->mem_base;
	//	std::cout << " Passed dummyB2 check" << std::endl;
	//	uint64_t dummy4 = (uint64_t) dummyB + dummyS;
	//	std::cout << " Passed dummy4 check" << std::endl;
	//	std::cout << "-\n";
	//	if (listIter1 != pool->free_chunks.end())
		//	std::cout << " Base1: " << (uint64_t) (*listIter1)->mem_base << ", Base1 Size: " << (*listIter1)->chunk_size << " Added " << (uint64_t) (*listIter1)->mem_base + (*listIter1)->chunk_size << std::endl;
	//	std::cout << "--\n";
	//	std::cout << "---\n";
		//if(listIter2 != pool->free_chunks.end())
	///		std::cout << " Base2: " << (uint64_t) (*listIter2)->mem_base << ", Base2 Size: " << (*listIter2)->chunk_size << std::endl;
	//	std::cout << "-----\n";
	//	std::cout << " Hitting if" << std::endl;
		bool moved = false;
		if ( listIter2 != pool->free_chunks.end() && (*listIter1)->mem_base + (*listIter1)->chunk_size == (*listIter2)->mem_base)
		{
			if (listIter1 != pool->free_chunks.end())
		//		std::cout << " Before assignment, size: " << (*listIter1)->chunk_size << std::endl;
			(*listIter1)->chunk_size = (*listIter1)->chunk_size + (*listIter2)->chunk_size;
			if (listIter1 != pool->free_chunks.end())
		//		std::cout << " Assigned, size" << (*listIter1)->chunk_size <<std::endl;
			if (listIter2 != pool->free_chunks.end())
		//		std::cout << "listIter2: " << (uint64_t)(*listIter2)->mem_base << std::endl;
			listIter3 = listIter2; listIter3++; 
			moved = true;
			delete *listIter2;
		//	std::cout << " Deleted" << std::endl;
			pool->free_chunks.erase(listIter2);
		//	std::cout << " Erased" << std::endl;
			listIter2 = listIter3;
		}
	//	std::cout << " Out of if" << std::endl;
		listIter1++;
		if (listIter2 != pool->free_chunks.end() && !moved)
			listIter2++;
		pool->free_chunks.sort(sortChunks);
		if (listIter1 == listIter2)
			listIter2++;
	}
		//const char *call = "Should've merged";
		//printAll(call);

	
	//Ok so fucking merging with maps. From facebook help confirmed my hypothesis. we will want to check if base+size = other base. 
	//So have two map iterators, and one be a step behind the other, say base1 and base2 (would be say map 0 and 1)
	//If base1+size = base2 then heey we get to merge. Basically delete base2 and make base1+size+base2'sSize, and BAM we've merged motherfuckers.
	//Also careful of edge cases.

	//From facebook: "anytime you merge two freespaces, be careful not to increment something that you've already erased
	//Always make sure one iterator is always ahead of the other."
	pool->free_chunks.sort(sortChunks);
//	std::cout << "Sorted free chunks by base: " << std::endl;
	for (listIter3 = pool->free_chunks.begin(); listIter3 != pool->free_chunks.end(); ++listIter3)
	{
	//	std::cout << "Chunk: " << (uint64_t)(*listIter3)->mem_base << "::" << (*listIter3)->chunk_size << std::endl;;
	}
	checkAlloQueues();

    MachineResumeSignals(&sigState);
	return VM_STATUS_SUCCESS;
}


/////////////////////////////////////////////////////////////////////////
////////////////              THREADS              //////////////////////
/////////////////////////////////////////////////////////////////////////
TVMStatus VMStart(int tickms, TVMMemorySize heapsize, int machinetickms, TVMMemorySize sharedsize, int argc, char *argv[])
{
	//Get the module name loaded
	char *moduleName = argv[0];
	TVMMainEntry mainModule;
	mainModule = VMLoadModule(moduleName);
	//Need to make an idle thread idle thread will be 1

	if (mainModule != NULL)
	{
		TVMMemorySize sharedSizeRounded = (sharedsize + 0xFFF) & (~0xFFF);
		//Initialize the Machine with tickms for machine
		
		//New Init: void *MachineInitialize(int timeout, size_t sharesize);
		uint8_t *shared_base = (uint8_t *)MachineInitialize(machinetickms, sharedSizeRounded);
		//VM_MEMORY_POOL_ID_SHARED
		memoryRef sharedMemPool = new memPool(VM_MEMORY_POOL_ID_SHARED, sharedSizeRounded, shared_base);
		// OLD INIT: MachineInitialize(machinetickms);
	//if(VM_STATUS_SUCCESS != VMMemoryPoolCreate(shared_base, sharedsize, &Shared_StorageID))
      //         VMPrintError("Failed to create Shared Storage\n");     
        
		//memPool(TVMMemoryPoolID poolID, TVMMemorySize poolSize, void* base)
		uint8_t *baseOfArray = new uint8_t[heapsize];
		memoryRef mainMemPool = new memPool(VM_MEMORY_POOL_ID_SYSTEM, heapsize, baseOfArray);
		allMemPools.push_back(mainMemPool);
		allMemPools.push_back(sharedMemPool);
		poolIDS[VM_MEMORY_POOL_ID_SYSTEM] = 0;
		poolIDS[VM_MEMORY_POOL_ID_SHARED] = 1;

		//const char *call = "VMStart";
		//printAll(call);

		//Request alarm
		TMachineAlarmCallback alarmCallBack = AlarmCallBack;
		MachineRequestAlarm(tickms * 1000, alarmCallBack, NULL);
		//Signals
		MachineEnableSignals();

		//MEMORY
		//1) You dynamically allocate with new, then you use the heapsize and the newly created array to pass to the creation of your memory pool.This needs to be referenced as the VM_MEMORY_POOL_ID_SYSTEM.
		//"2) You are only creating the stack for the thread with memory allocate, not the TCB itself. You still use new for the TCB, but not for the base address." 



		//Setup the main thread
		TCBRef mainThread = new ThreadControlBlock((TVMThreadID) 0, VM_THREAD_STATE_RUNNING, VM_THREAD_PRIORITY_NORMAL);
		allThreads.push_back(mainThread);
		CURRENT_THREAD = mainThread;
//		std::cout << "___MAIN THREAD CREATED AND SET___\n";
		//Create the idle thread
		TCBRef idleThread = new ThreadControlBlock((TVMThreadID)1, VM_THREAD_STATE_READY, VM_THREAD_PRIORITY_IDLE , 0x100000, idleEntry, NULL);
		//TVMStatus VMMemoryPoolAllocate(TVMMemoryPoolID memory, TVMMemorySize size, void **pointer)
		idleThread->stackAddr = new uint8_t[0x100000];
		VMMemoryPoolAllocate(0, 0x100000, (void**)idleThread->stackAddr);
		IDLE_THREAD = idleThread;
		allThreads.push_back(idleThread);
		MachineContextCreate(&(idleThread->mcntx), idleThread->t_entry, NULL, idleThread->stackAddr, idleThread->t_memSize);
//		std::cout << "___IDLE THREAD CREATED AND SET___\n";
		//Start VMMain if Module wasn't 
//		std::cout << "___CALLING MAIN ARG___\n";
//		std::cout << "___ARGC: " << argc << "___\n";
//		std::cout << "___ARGV_ " << argv[0] << "___\n";
		mainModule(argc, argv);
		//const char *call2 = "Terminating Soon";
		//printAll(call2);
		VMUnloadModule();
		//std::cout << "Unloaded module................." << std::endl;
		MachineTerminate();
		//std::cout << "MachineTerminated.............." << std::endl;
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
	//TVMStatus VMMemoryPoolAllocate(TVMMemoryPoolID memory, TVMMemorySize size, void **pointer)
	//std::cout << "Calling Pool Allocate with size: " << memsize << " and address: " << (uint64_t)newThread->stackAddr << std::endl;
	VMMemoryPoolAllocate(0, memsize, (void**)newThread->stackAddr);
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
	VMMemoryPoolDeallocate(0, (void**)allThreads[thread]->stackAddr);
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

/////////////////////////////////////////////////////////////////////////
////////////////              MUTEX                //////////////////////
/////////////////////////////////////////////////////////////////////////
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


/////////////////////////////////////////////////////////////////////////
////////////////                FILE               //////////////////////
/////////////////////////////////////////////////////////////////////////

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

void pushFileToAllocationWait(TCBRef toWait)
{
	toWait->t_state = VM_THREAD_STATE_WAITING;

	if (toWait->t_prio == VM_THREAD_PRIORITY_HIGH)
	{
		high_prio_AlloQueue.push_back(toWait);
	}
	else if (toWait->t_prio == VM_THREAD_PRIORITY_NORMAL)
	{
		norm_prio_AlloQueue.push_back(toWait);
	}
	else if (toWait->t_prio == VM_THREAD_PRIORITY_LOW)
	{
		low_prio_AlloQueue.push_back(toWait);
	}
	else
		std::cerr << "Failed at pushing allocation to wait " << std::endl;
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
	//std::cout << "Closing File...." << std:: endl;
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
	//void MachineFileRead(int fd, void *data, int length, TMachineFileCallback callback, void *calldata);

//	const char *call = "VMFileRead";
//	printAll(call);

	while(allMemPools[VM_MEMORY_POOL_ID_SHARED]->totalFree_space < *length)
	{
		pushFileToAllocationWait(fileThread);
		scheduleThread();
	}
	
	
	/*If allocation of shared memory fails, we need to push the thread to a waiting queue waiting for allocation
		if allocation failed, pushed to wait queue, schedule, come back and try again
		when deallocating, check to see if there are any waiting for allocation for that pool and schedule up -> put highest prio into ready as before, and schedule immediatly
		TCBQueue low_prio_AlloQueue;
		TCBQueue norm_prio_AlloQueue;
		TCBQueue high_prioAlloQueue;*/

	VMMemoryPoolAllocate(VM_MEMORY_POOL_ID_SHARED, std::min(*length, 512), (void**) &allMemPools[VM_MEMORY_POOL_ID_SHARED]->pool_base);
	//allMemPools[VM_MEMORY_POOL_ID_SHARED]->pool_base
	int lengthLeft = *length;
	for (int i = 0; lengthLeft >= 0; i++, lengthLeft -= 512)
	{
		MachineFileRead(filedescriptor, allMemPools[VM_MEMORY_POOL_ID_SHARED]->pool_base, std::min(lengthLeft, 512), fileCallBack, fileThread);//IF PROBLEMS, PUT SCHEDULE BEHIND OPEN
		//Put file to wait
		//std::cout << "Push to wait .... " << std::endl;
		pushFileToWait(fileThread);
		//std::cout << "scheduling now...." << std::endl;
		scheduleThread();
		//memcpy(void*dest, void*src, length)
		memcpy(data, allMemPools[VM_MEMORY_POOL_ID_SHARED]->pool_base, std::min(lengthLeft, 512));
		//std::cout << "Resuming signals returning....." << std::endl;
	}
	//VMMemoryPoolDeallocate(VM_MEMORY_POOL_ID_SHARED, allMemPools[VM_MEMORY_POOL_ID_SHARED]->pool_base);
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
	
//	const char *call = "VMFileWrite";
//	printAll(call);

	TCBRef fileThread = allThreads[CURRENT_THREAD->t_id];
//	std::cout << "___WRITING WITH THREAD: " << fileThread->t_id << "___\n";
	while (allMemPools[VM_MEMORY_POOL_ID_SHARED]->totalFree_space < *length)
	{
		pushFileToAllocationWait(fileThread);
		scheduleThread();
	}
	int lengthLeft = *length;
	VMMemoryPoolAllocate(VM_MEMORY_POOL_ID_SHARED, std::min(*length, 512), (void**) &allMemPools[VM_MEMORY_POOL_ID_SHARED]->pool_base);
//	std::cout << "\nOut of Write Allocate" << std::endl;
	for (int i = 0; lengthLeft >= 0; i++, lengthLeft -= 512)
	{
		memcpy(allMemPools[VM_MEMORY_POOL_ID_SHARED]->pool_base, data, std::min(lengthLeft, 512));
		//Call the machine to write to a file, the the length transfered will be put into thread->fileResult
		//void MachineFileWrite(int fd, void *data, int length, TMachineFileCallback callback, void *calldata);
		MachineFileWrite(filedescriptor, allMemPools[VM_MEMORY_POOL_ID_SHARED]->pool_base, std::min(lengthLeft, 512), fileCallBack, fileThread);//IF PROBLEMS, PUT SCHEDULE BEHIND OPEN
		//Put file to wait
		pushFileToWait(fileThread);

		scheduleThread();
	}
	VMMemoryPoolDeallocate(VM_MEMORY_POOL_ID_SHARED, allMemPools[VM_MEMORY_POOL_ID_SHARED]->pool_base);
	//std::cout << "\nOut of Write Deallocate" << std::endl;

	//std::cout << "IN WRITE: resuming signals........" << std::endl;
	
	//Resume Signals
	//std::cout << "Leaving write..." << std::endl;
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
