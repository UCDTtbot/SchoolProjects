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
#include <fcntl.h>
#include <iomanip>
#include <bitset>

/*
1. hello.so: (VMStart, VMFileWrite)
2. sleep.so: (VMThreadSleep and use the alarm)
3. thread.so: (VMThreadCreate, VMThreadState, and VMThreadActivate)
4. file.so: (VMFileOpen, VMFileSeek, VMFileRead, and VMFileClose)
5. mutex.so: (VMMutexCreate, VMMutexAcquire, and VMMutexRelease)
*/

extern "C"{

void parseBPB(uint8_t* buffer);
void printBPB();
void parseRoot(uint8_t* buffer, uint16_t offset);
void printDir();
void readSector(unsigned int location, unsigned int offset);
void writeSector();
void convert16toTime(uint16_t time);
void convert16toDate(uint16_t date);
void printFat();
void readCluster(uint16_t clusNum, unsigned int size);
void writeCluster(uint16_t clusNum, void *data, unsigned int size);
void findOpenFile(unsigned int *vecOffSet, int fd);
bool findClusCache(uint16_t clusLow, unsigned int *pos);
void findFreeClus(uint16_t* clusLow);
void findOpenDir(unsigned int *vecOffSet, int fd);
TVMMutexID FAT_Mutex;
uint16_t FirstRootSector;
uint16_t RootDirectorySectors;
uint16_t FirstDataSector;
uint16_t ClusterCount;
char curDir[VM_FILE_SYSTEM_MAX_PATH];

typedef struct FAT
{
	uint8_t FATNum;
	uint16_t FATNext;

	FAT(){}
	FAT(uint8_t Num, uint16_t Next){ FATNum = Num; FATNext = Next; }

}FATEntry, *FATEntryRef;

typedef std::vector<uint16_t> FATVector;
FATVector FAT_Vector;

typedef struct DirEntry{
	
	SVMDirectoryEntryRef SVMData = new SVMDirectoryEntry;
	/*    SVMDirectoryEntry Holds:
	char DLongFileName[VM_FILE_SYSTEM_MAX_PATH];
    char DShortFileName[VM_FILE_SYSTEM_SFN_SIZE];
    unsigned int DSize;
    unsigned char DAttributes;
    SVMDateTime DCreate;
    SVMDateTime DAccess;
    SVMDateTime DModify;
	*/
	uint16_t dirOffset;
	uint16_t clusLow[1];
	uint16_t NTRes[1];
	unsigned int openFD;

	DirEntry(){}

} DirEntry, *DirEntryRef;

void searchRoot(const char *fileName, int *fd, DirEntryRef entryRef);
typedef std::map<uint16_t, uint16_t> FD_MAP;
FD_MAP FD_Map;
typedef std::vector<DirEntryRef> DIRVector;
DIRVector freeDirs;
DIRVector currentDirs;
DIRVector openFiles;
DIRVector openDirect;


typedef struct Cluster
{
	uint16_t clusNum;
	uint16_t dirty;
	uint8_t* data;
	unsigned int dataSize;

	Cluster(){}
	Cluster(uint16_t num, uint8_t dirtyBit, unsigned int size){ clusNum = num; dirty = dirtyBit; data = new uint8_t[size * 512]; dataSize = size * 512; }
}Cluster, *ClusterRef;

typedef std::map<uint16_t, ClusterRef> CLUSTER_MAP;
CLUSTER_MAP clusterCache;

typedef struct BPB
{
	uint8_t jmpBoot[3];
	uint8_t OEMName[8];
	uint16_t BytsPerSec[2];
	uint16_t SecPerClus[1];
	uint16_t RsvdSecCnt[2];
	uint16_t NumFATs[1];
	uint16_t RootEntCnt[2];
	uint16_t TotSec16[2];
	uint16_t Media[1];
	uint16_t FATSz16[2];
	uint16_t SecPerTrk[2];
	uint16_t NumHeads[2];
	uint16_t HiddSec[4];
	uint16_t TotSec32[4];
	uint16_t DrvNum[1];
	uint16_t Reserved1[1];
	uint16_t BootSig[1];
	uint16_t VolID[4];
	uint8_t VolLab[11];
	uint8_t FilSysType[8];

	BPB(){}
}BPB, *BPBRef;

BPB Main_BPB;
uint8_t* SHARED_BUFFER;

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
unsigned int IMAGE_FD;

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
void VMDirCopy(char *dest, const char *src);
void VMStringCopyN(char *dest, const char *src, int32_t n);
void pushFileToWait(TCBRef toWait);


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
	//std::cout << "___ENTERED FILE CALL BACK___\n";
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
		std::cout << "Memory: " << memory << " Pointer: " << pointer << std::endl;	//error checking
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
//TVMStatus VMStart(int tickms, TVMMemorySize heapsize, int machinetickms, TVMMemorySize sharedsize, const char *mount, int argc, char *argv[]);
TVMStatus VMStart(int tickms, TVMMemorySize heapsize, int machinetickms, TVMMemorySize sharedsize, const char *mount, int argc, char *argv[])
{
	//Get the module name loaded
	char *moduleName = argv[0];
	unsigned int offset = 0;
	uint8_t* originalSpot;
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

	//	const char *call = "VMStart";
	//	printAll(call);

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
		VMMemoryPoolAllocate(0, 0x100000, (void**)&idleThread->stackAddr);
		IDLE_THREAD = idleThread;
		allThreads.push_back(idleThread);
		MachineContextCreate(&(idleThread->mcntx), idleThread->t_entry, NULL, idleThread->stackAddr, idleThread->t_memSize);
//		std::cout << "___IDLE THREAD CREATED AND SET___\n";
		//Start VMMain if Module wasn't 
//		std::cout << "___CALLING MAIN ARG___\n";
//		std::cout << "___ARGC: " << argc << "___\n";
//		std::cout << "___ARGV_ " << argv[0] << "___\n";

		/////////////////////////////////////////////////////
		////////////////      FAT     ///////////////////////
		/////////////////////////////////////////////////////
		//void MachineFileOpen(const char *filename, int flags, int mode, TMachineFileCallback callback, void *calldata);
		//void fileCallBack(void *calldata, int result)

		//In C and C++ programs remember that numbers with leading zeros are always assumed to be in octal, so the mode 0751 represents rwxr-x--x. 4 = r, 2 = w, 1 = x
		//User | Group | Other ||| rwx rwx rwx = 0777
		//O_RDONLY - READ ONLY | O_WRONLY - WRITE ONLY | O_RDWR - READ WRITE ONLY 
		MachineFileOpen(mount, O_RDWR, 0644, fileCallBack, CURRENT_THREAD);	//Open FAT, see above for modes
		pushFileToWait(CURRENT_THREAD);
		scheduleThread();
		//Now: Write a function to read and write whole sectors using MachineFileRead and MachineFileWrite. Use MachineFileSeek to move the file pointer to the location.
		//The read function needs the sector number and a pointer to the data (the file pointer, uint8_t*)
		IMAGE_FD = CURRENT_THREAD->t_fileResult;
		//std::cout << "Got FD: " << IMAGE_FD << std::endl;
		//For reading in initial BPD, we should not use the VMRead Function, but the machine one
		//Use MachineFileSeek and Read to read in the BPD and store it in memory
		VMMemoryPoolAllocate(VM_MEMORY_POOL_ID_SHARED, 512, (void**)&SHARED_BUFFER);
		MachineFileRead(IMAGE_FD, SHARED_BUFFER, 512, fileCallBack, CURRENT_THREAD);
		pushFileToWait(CURRENT_THREAD);
		scheduleThread();
		parseBPB(SHARED_BUFFER);
		VMMemoryPoolDeallocate(VM_MEMORY_POOL_ID_SHARED, SHARED_BUFFER);
	//	printBPB();
		//After BPB Read in, calculate needed entries for finding FAT and Root
		FirstRootSector = *Main_BPB.RsvdSecCnt + *Main_BPB.NumFATs * *Main_BPB.FATSz16;
		RootDirectorySectors = (*Main_BPB.RootEntCnt * 32) / 512;
		FirstDataSector = FirstRootSector + RootDirectorySectors;
		ClusterCount = (*Main_BPB.TotSec32 - FirstDataSector) / *Main_BPB.SecPerClus;
		strcpy(curDir, "/");
	//	std::cout << "First Root Sector: " << FirstRootSector << std::endl;
		//std::cout << "Root Directory Sectors: " << RootDirectorySectors << std::endl;
		//std::cout << "First Data Sector: " << FirstDataSector << std::endl;
		//std::cout << "Cluster Count: " << ClusterCount << std::endl;
		//Now we must read in the FAT
		uint16_t FATBuffer[*Main_BPB.FATSz16*256];
		VMMemoryPoolAllocate(VM_MEMORY_POOL_ID_SHARED, 512, (void**)&SHARED_BUFFER);
		//MachineFileSeek(IMAGE_FD, (512 * *Main_BPB.RsvdSecCnt), 0, fileCallBack, CURRENT_THREAD);
	//	pushFileToWait(CURRENT_THREAD);
	//	scheduleThread();
		for (uint16_t i = 0; i < *Main_BPB.FATSz16; i++)
		{
			//MachineFileRead(IMAGE_FD, SHARED_BUFFER, 512, fileCallBack, CURRENT_THREAD);
			//pushFileToWait(CURRENT_THREAD);
			//scheduleThread();
			readSector((512 * *Main_BPB.RsvdSecCnt), offset);
			memcpy(FATBuffer, SHARED_BUFFER, 512);
			for (int x = 0; x < 256; x++)
				FAT_Vector.push_back(FATBuffer[x]);
			offset += 512;
		}
		//printFat();
		offset = 0;
		VMMemoryPoolDeallocate(VM_MEMORY_POOL_ID_SHARED, SHARED_BUFFER);
		//FAT is now read in and the vector is populated with the cluster entries. 
		//Now we must read in the Root Dir
		VMMemoryPoolAllocate(VM_MEMORY_POOL_ID_SHARED, 512, (void**)&SHARED_BUFFER);
		originalSpot = SHARED_BUFFER;
		//MachineFileSeek(IMAGE_FD, (512 * FirstRootSector), 0, fileCallBack, CURRENT_THREAD);
		//pushFileToWait(CURRENT_THREAD);
		//scheduleThread();
		for (uint16_t i = 0; i < (*Main_BPB.RootEntCnt * 32) / 512; i++)
		{
			//MachineFileRead(IMAGE_FD, SHARED_BUFFER, 512, fileCallBack, CURRENT_THREAD);
			//pushFileToWait(CURRENT_THREAD);
			//scheduleThread();
			readSector((512 * FirstRootSector), offset);
			for (uint16_t x = 0; x < 512; x += 32)
			{
				parseRoot(SHARED_BUFFER, x);
				SHARED_BUFFER += 32;
			}
			offset += 512;
		}
		offset = 0;
		VMMemoryPoolDeallocate(VM_MEMORY_POOL_ID_SHARED, originalSpot);
		//printDir();

		//VMMemoryPoolAllocate(VM_MEMORY_POOL_ID_SHARED, 512, (void**)&SHARED_BUFFER);
		//originalSpot = SHARED_BUFFER;
		//std::cout << "Test Cluster Read" << std::endl << "======================" << std::endl;
		//readCluster(0x14, 0xB6);
		//VMMemoryPoolDeallocate(VM_MEMORY_POOL_ID_SHARED, originalSpot);
		//int fd = 0;
		//DirEntryRef DirRef = new DirEntry();
		//searchRoot("MACHINE .CPP", &fd, DirRef);
		//std::cout << std::dec << "FD: " << fd << std::endl;
		//std::cout << "Got Dir: " << DirRef->SVMData.DShortFileName << std::endl;

		mainModule(argc, argv);
		//const char *call2 = "Terminating Soon";
		//printAll(call2);

		printBPB();
		std::cout << "First Root Sector: " << FirstRootSector << std::endl;
		std::cout << "Root Directory Sectors: " << RootDirectorySectors << std::endl;
		std::cout << "First Data Sector: " << FirstDataSector << std::endl;
		std::cout << "Cluster Count: " << ClusterCount << std::endl;
		printFat();
		printDir();

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
	//newThread->stackAddr = new uint8_t[memsize];
	//TVMStatus VMMemoryPoolAllocate(TVMMemoryPoolID memory, TVMMemorySize size, void **pointer)
	VMMemoryPoolAllocate(0, memsize, (void**)&newThread->stackAddr);
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

	//TCBRef fileThread = allThreads[CURRENT_THREAD->t_id];

	char DirName[VM_FILE_SYSTEM_SFN_SIZE];
	VMDirCopy(DirName, filename);
	DirEntryRef DirRef = new DirEntry();
	searchRoot(DirName, filedescriptor, DirRef);
	//flags & O_ACCMODE gives RDONLY, WRONLY, and RDWR
	if (*filedescriptor == -1 && (flags & O_CREAT) != O_CREAT)
	{
		MachineResumeSignals(&sigState);
		return VM_STATUS_FAILURE;
	}
	else if (*filedescriptor == -1 && (flags & O_CREAT) == O_CREAT)
	{
		//Create file
		DirRef = freeDirs.back();
		freeDirs.pop_back();
		strcpy((DirRef->SVMData)->DShortFileName, DirName);
		(DirRef->SVMData)->DSize = 0;
		(DirRef->SVMData)->DAttributes = 0;
		VMDateTime(&(DirRef->SVMData)->DCreate);
		VMDateTime(&(DirRef->SVMData)->DAccess);
		VMDateTime(&(DirRef->SVMData)->DModify);
		//Should already have an offset
		findFreeClus(DirRef->clusLow);
		*DirRef->NTRes = 0;
		currentDirs.push_back(DirRef);
		openFiles.push_back(DirRef);
		*filedescriptor = DirRef->openFD;
	}
	else
	{
		//File exists. Push to openFiles
		//Since file exists, it will already have an offset
		DirRef->openFD = *filedescriptor;
		openFiles.push_back(DirRef);
	}

	//Resume Signals
	MachineResumeSignals(&sigState);
	return VM_STATUS_SUCCESS;

}

TVMStatus VMFileClose(int filedescriptor)
{
	//Suspend Signals
	TMachineSignalState sigState;
	MachineSuspendSignals(&sigState);

	TCBRef fileThread = allThreads[CURRENT_THREAD->t_id];
	unsigned int vecOffset = -1;

	if (filedescriptor < 3)
	{
		//Call the machine to close a file, 
		MachineFileClose(filedescriptor, fileCallBack, fileThread);//IF PROBLEMS, PUT SCHEDULE BEHIND OPEN
		//Put file to wait
		pushFileToWait(fileThread);
		scheduleThread();

		if (fileThread->t_fileResult < 0)
		{
			MachineResumeSignals(&sigState);
			return VM_STATUS_FAILURE;
		}
		//std::cout << "Closing File...." << std:: endl;
		//Resume Signals
	}
	else
	{
		//void findOpenFile(int *vecOffSet, int fd)
		findOpenFile(&vecOffset, filedescriptor);

		if (vecOffset >= 0)
		{
			VMDateTime(&(openFiles[vecOffset]->SVMData)->DAccess);
			VMDateTime(&(openFiles[vecOffset]->SVMData)->DModify);
			openFiles.erase(openFiles.begin() + vecOffset);
		}
		else
		{
			MachineResumeSignals(&sigState);
			return VM_STATUS_FAILURE;
		}
	}
	std::cout << "Should have successfully closed the file " << std::endl;
	MachineResumeSignals(&sigState);
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

	if (filedescriptor < 3)
	{
		while (allMemPools[VM_MEMORY_POOL_ID_SHARED]->totalFree_space < *length)
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

		VMMemoryPoolAllocate(VM_MEMORY_POOL_ID_SHARED, std::min(*length, 512), (void**)&allMemPools[VM_MEMORY_POOL_ID_SHARED]->pool_base);
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
	else	//If the FD > 3 we are working with the FAT and need to do stuff differently
	{
		unsigned int vecOffset = -1;
		unsigned int pos = 0;
		findOpenFile(&vecOffset, filedescriptor);
		if (vecOffset < 0)	//File was never open
		{
			MachineResumeSignals(&sigState);
			return VM_STATUS_FAILURE;
		}
		if (!findClusCache(*openFiles[vecOffset]->clusLow, &pos))
		{
			VMMemoryPoolAllocate(VM_MEMORY_POOL_ID_SHARED, 512, (void**)&SHARED_BUFFER);
			uint8_t* originalSpot = SHARED_BUFFER;
			readCluster(*openFiles[vecOffset]->clusLow, (openFiles[vecOffset]->SVMData)->DSize);
			VMMemoryPoolDeallocate(VM_MEMORY_POOL_ID_SHARED, originalSpot);
			if (findClusCache(*openFiles[vecOffset]->clusLow, &pos))
				;
			else
			{
				std::cout << "Failed in VMRead, couldn't find open file." << std::endl;
				MachineResumeSignals(&sigState);
				return VM_STATUS_FAILURE;
			}
		}
		else
		{
			memcpy(data, clusterCache[pos]->data, clusterCache[pos]->dataSize);
		}
	}
	MachineResumeSignals(&sigState);
	return VM_STATUS_SUCCESS;

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

	if (filedescriptor < 3)
	{

		//	const char *call = "VMFileWrite";
		//	printAll(call);


		//	std::cout << "___WRITING WITH THREAD: " << fileThread->t_id << "___\n";
		while (allMemPools[VM_MEMORY_POOL_ID_SHARED]->totalFree_space < *length)
		{
			pushFileToAllocationWait(fileThread);
			scheduleThread();
		}
		int lengthLeft = *length;
		VMMemoryPoolAllocate(VM_MEMORY_POOL_ID_SHARED, std::min(*length, 512), (void**)&allMemPools[VM_MEMORY_POOL_ID_SHARED]->pool_base);
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

		MachineResumeSignals(&sigState);
		if (fileThread->t_fileResult < 0)
			return VM_STATUS_FAILURE;
		else
		{
			*length = fileThread->t_fileResult;
			return VM_STATUS_SUCCESS;
		}
	}
	else
	{
		unsigned int vecOffset = -1;
		unsigned int pos = 0;
		findOpenFile(&vecOffset, filedescriptor);
		if (vecOffset < 0)	//File was never opened
		{
			MachineResumeSignals(&sigState);
			return VM_STATUS_FAILURE;
		}
		if (!findClusCache(*openFiles[vecOffset]->clusLow, &pos))	//If cluster does not exist
		{
			writeCluster(*openFiles[vecOffset]->clusLow, data, *length);
			if (findClusCache(*openFiles[vecOffset]->clusLow, &pos))
				;
			else
			{
				std::cout << "Failed in VMWrite, couldn't find open file." << std::endl;
				MachineResumeSignals(&sigState);
				return VM_STATUS_FAILURE;
			}
		}
		else
		{
			ClusterRef toWriteTo = clusterCache[*openFiles[vecOffset]->clusLow];
			memcpy(toWriteTo->data, data, *length);
			toWriteTo->dirty = 1;
		}
	}
	MachineResumeSignals(&sigState);
	return VM_STATUS_SUCCESS;
}
TVMStatus VMFileSeek(int filedescriptor, int offset, int whence, int *newoffset)
{
	if (filedescriptor < 3 )
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
	}
	else
	{
		uint16_t offset = 0;			
		
		//MachineFileSeek(IMAGE_FD, location + offset, 0, fileCallBack, CURRENT_THREAD);

		TMachineSignalState sigState;
		MachineSuspendSignals(&sigState);
		TCBRef fileThread = allThreads[CURRENT_THREAD->t_id];
		unsigned index = -1;
		for (unsigned int x = 0; x < openFiles.size(); x++)
		{
			if (filedescriptor == openFiles[x]->openFD)
				index = x;
		}
		if (index < 0)
		{
			std::cout << "Failed to seek" << std::endl;
			MachineResumeSignals(&sigState);
			return VM_STATUS_FAILURE;
		}
		
		offset = (FirstDataSector + ((*openFiles[index]->clusLow - 2) * *Main_BPB.SecPerClus)) * 512;

		MachineFileSeek(filedescriptor, offset, 0, fileCallBack, fileThread);

		pushFileToWait(fileThread);
		scheduleThread();

		MachineResumeSignals(&sigState);
		if (fileThread->t_fileResult < 0)
			return VM_STATUS_FAILURE;
		else
		{
			*newoffset = fileThread->t_fileResult;
			return VM_STATUS_SUCCESS;
		}
	}
	
	return VM_STATUS_SUCCESS;

}

/////////////////////////////////////////////////////////////////////////
////////////////                FAT	               //////////////////////
/////////////////////////////////////////////////////////////////////////
/*
typedef memChunk* chunkRef;	//REFERENCE TO MEMORY CHUNKS
typedef std::vector<chunkRef> ChunkVector; //CHUNK VECTOR
ChunkVector chunksInUse;
std::list<chunkRef>::iterator listIter1;
std::list<chunkRef>::iterator listIter2;
std::list<chunkRef>::iterator listIter3;
*/

TVMStatus VMDirectoryOpen(const char *dirname, int *dirdescriptor)
{
	//Non-Extra credit, only care for root
	//Suspend Signals
	TMachineSignalState sigState;
	MachineSuspendSignals(&sigState);

	//TCBRef fileThread = allThreads[CURRENT_THREAD->t_id];

	char DirName[VM_FILE_SYSTEM_SFN_SIZE];
	VMDirCopy(DirName, dirname);
	bool found = false;
	unsigned int index;
	DirEntryRef DirRef = new DirEntry();
	//flags & O_ACCMODE gives RDONLY, WRONLY, and RDWR
	//Create file
	for (unsigned int x = 0; x < openDirect.size(); x++)
	{
		if (strcmp((openDirect[x]->SVMData)->DShortFileName, DirName))
		{
			found = true;
			index = x;
		}
	}
	if (!found)
	{
		strcpy((DirRef->SVMData)->DShortFileName, DirName);
		(DirRef->SVMData)->DSize = 0;
		(DirRef->SVMData)->DAttributes = 0;
		//Should already have an offset
		DirRef->dirOffset = 0;
		*DirRef->clusLow = FirstRootSector;
		*DirRef->NTRes = 0;
		DirRef->openFD = currentDirs.size();
		*dirdescriptor = DirRef->openFD;
		currentDirs.push_back(DirRef);
		openDirect.push_back(DirRef);
	}
	else
	{
		DirRef->openFD = currentDirs.size();
		*dirdescriptor = DirRef->openFD;
		currentDirs.push_back(DirRef);
		openDirect.push_back(DirRef);
	}

	//Resume Signals
	MachineResumeSignals(&sigState);
	return VM_STATUS_SUCCESS;
}

TVMStatus VMDirectoryClose(int dirdescriptor)
{
	unsigned int vecOffset = -1;

	TMachineSignalState sigState;
	MachineSuspendSignals(&sigState);

	findOpenDir(&vecOffset, dirdescriptor);


	if (vecOffset >= 0)
	{
		openDirect.erase(openDirect.begin() + vecOffset);
	}
	else
	{
		MachineResumeSignals(&sigState);
		return VM_STATUS_FAILURE;
	}

	MachineResumeSignals(&sigState);

	return VM_STATUS_SUCCESS;
}

TVMStatus VMDirectoryRead(int dirdescriptor, SVMDirectoryEntryRef dirent)
{
	//Might need to hardwire root in for now
	TMachineSignalState sigState;
	MachineSuspendSignals(&sigState);

	unsigned int index = 0;
	unsigned int vecOffset = -1;
	findOpenDir(&vecOffset, dirdescriptor);
	if (vecOffset < 0)
	{
		std::cout << "Dir not open" << std::endl;
		MachineResumeSignals(&sigState);
		return VM_STATUS_FAILURE;
	}
	if (!strcmp((openDirect[vecOffset]->SVMData)->DShortFileName, "/"))
	{
		std::cout << "Not Root" << std::endl;
		MachineResumeSignals(&sigState);
		return VM_STATUS_FAILURE;
	}
	else
	{
		if (openDirect[vecOffset]->dirOffset >= currentDirs.size())
		{
			std::cout << "No More Files" << std::endl;
			MachineResumeSignals(&sigState);
			return VM_STATUS_FAILURE;
		}
		strcpy(dirent->DShortFileName, currentDirs[openDirect[vecOffset]->dirOffset]->SVMData->DShortFileName);
		dirent->DSize = currentDirs[openDirect[vecOffset]->dirOffset]->SVMData->DSize;
		dirent->DAttributes = currentDirs[openDirect[vecOffset]->dirOffset]->SVMData->DAttributes;
		dirent->DCreate = currentDirs[openDirect[vecOffset]->dirOffset]->SVMData->DCreate;
		dirent->DAccess = currentDirs[openDirect[vecOffset]->dirOffset]->SVMData->DAccess;
		dirent->DModify = currentDirs[openDirect[vecOffset]->dirOffset]->SVMData->DModify;
		openDirect[vecOffset]->dirOffset++;
	}
	 //currently open directories
	MachineResumeSignals(&sigState);
	return VM_STATUS_SUCCESS;
}

TVMStatus VMDirectoryRewind(int dirdescriptor)
{
	TMachineSignalState sigState;
	MachineSuspendSignals(&sigState);

	unsigned int index = 0;
	unsigned int vecOffset = -1;
	findOpenDir(&vecOffset, dirdescriptor);
	//Rewind to the beginning
	openDirect[vecOffset]->dirOffset = 0;

	MachineResumeSignals(&sigState);
	return VM_STATUS_SUCCESS;
}

TVMStatus VMDirectoryCurrent(char *abspath)
{
	VMFileSystemGetAbsolutePath(abspath, curDir, curDir);
	return VM_STATUS_SUCCESS;
}

TVMStatus VMDirectoryChange(const char *path)
{
	std::cout << "Must stay in ROOT (Cause I ran out of time) " << std::endl;
	return VM_STATUS_FAILURE;
}

TVMStatus VMDirectoryCreate(const char *dirname)	//EXTRA CREDIT
{
	return VM_STATUS_SUCCESS;
}

TVMStatus VMDirectoryUnlink(const char *path)		//EXTRA CREDIT
{
	return VM_STATUS_SUCCESS;
}

void parseBPB(uint8_t* buffer)
{
	//memcpy(void*dest, void*src, length)
	unsigned int x = 0;
	//std::cout << x << std::endl;

	//for (; x < 3; ++x)
	memcpy(Main_BPB.jmpBoot, buffer, 3);
	buffer += 3;

	//for (; x < 11; ++x)
	memcpy(Main_BPB.OEMName, buffer, 8);
	buffer += 8;

	//for (; x < 13; ++x)
	memcpy(Main_BPB.BytsPerSec, buffer, 2);
	buffer += 2;

	//for (; x < 14; ++x)
	memcpy(Main_BPB.SecPerClus, buffer, 1);
	buffer += 1;

	//for (; x < 16; ++x)
	memcpy(Main_BPB.RsvdSecCnt, buffer, 2);
	buffer += 2;

	//for (; x < 17; ++x)
	memcpy(Main_BPB.NumFATs, buffer, 1);
	buffer += 1;

	//for (; x < 19; ++x)
	memcpy(Main_BPB.RootEntCnt, buffer, 2);
	buffer += 2;

	//for (; x < 21; ++x)
	memcpy(Main_BPB.TotSec16, buffer, 2);
	buffer += 2;

	//for (; x < 22; ++x)
	memcpy(Main_BPB.Media, buffer, 1); 
	buffer += 1;

	//for (; x < 24; ++x)
	memcpy(Main_BPB.FATSz16, buffer, 2); 
	buffer += 2;

	//for (; x < 26; ++x)
	memcpy(Main_BPB.SecPerTrk, buffer, 2);
	buffer += 2;	

	//for (; x < 28; ++x)
	memcpy(Main_BPB.NumHeads, buffer, 2); 
	buffer += 2;

	//for (; x < 32; ++x)
	memcpy(Main_BPB.HiddSec, buffer, 4);
	buffer += 4;

	//for (; x < 36; ++x)
	memcpy(Main_BPB.TotSec32, buffer, 2);
	buffer += 4;

	//for (; x < 37; ++x)
	memcpy(Main_BPB.DrvNum, buffer, 1);
	buffer += 1;

	//for (; x < 38; ++x)
	memcpy(Main_BPB.Reserved1, buffer, 1);
	buffer += 1;

	//for (; x < 39; ++x)
	memcpy(Main_BPB.BootSig, buffer, 1);
	buffer += 1; 

	//for (; x < 43; ++x)
	memcpy(Main_BPB.VolID, buffer, 4);
	buffer += 4;

	//for (; x < 54; ++x)
	memcpy(Main_BPB.VolLab, buffer, 11); 
	buffer += 11;

	//for (; x < 64; ++x)
	memcpy(Main_BPB.FilSysType, buffer, 8);
	buffer += 8;
		
}

void printBPB()
{
	std::cout << "OEM Name: " << Main_BPB.OEMName << std::endl;
	std::cout << "Byts Per Sec: " << *Main_BPB.BytsPerSec << std::endl;
	std::cout << "Sec Per Clus: " << *Main_BPB.SecPerClus << std::endl;
	std::cout << "Rsvd Sec Cnt: " << *Main_BPB.RsvdSecCnt << std::endl;
	std::cout << "Num FATs: " << *Main_BPB.NumFATs << std::endl;
	std::cout << "RootEntCnt: " << *Main_BPB.RootEntCnt << std::endl;
	std::cout << "TotSec16: " << *Main_BPB.TotSec16 << std::endl;
	std::cout << "Media: " << *Main_BPB.Media << std::endl;
	std::cout << "FATSz16: " << *Main_BPB.FATSz16 << std::endl;
	std::cout << "Sec Per Trk:" << *Main_BPB.SecPerTrk << std::endl;
	std::cout << "Num Heads: " << *Main_BPB.NumHeads << std::endl;
	std::cout << "Hidd Sec: " << *Main_BPB.HiddSec << std::endl;
	std::cout << "TotSec32: " << *Main_BPB.TotSec32 << std::endl;
	std::cout << "Fil Sys Type: " << Main_BPB.FilSysType << std::endl;
}

void parseRoot(uint8_t* buffer, uint16_t offset)
{
	//memcpy(void*dest, void*src, length)
	/*
	SVMDirectoryEntry SVMData;
	
	SVMDirectoryEntry Holds:
	char DLongFileName[VM_FILE_SYSTEM_MAX_PATH];
	char DShortFileName[VM_FILE_SYSTEM_SFN_SIZE];
	unsigned int DSize;
	unsigned char DAttributes;
	SVMDateTime DCreate;
	SVMDateTime DAccess;
	SVMDateTime DModify;
	
	uint16_t dirOffset;
	uint16_t clusLow[2];
	uint16_t NTRes[1];
	*/
	//currentDirs

	DirEntryRef newDir = new DirEntry();
	bool dontAdd = false;
	bool freeDir = false;
	newDir->dirOffset = offset;
	char Temp[VM_FILE_SYSTEM_SFN_SIZE];

	memcpy(Temp, buffer, 11);
	buffer += 11;

	if (Temp[0] == 0xE5 || Temp[0] == 0x00) //Means a free entry
	{
		dontAdd = true;
		freeDir = true;
	}

	(newDir->SVMData)->DAttributes = *buffer;
	buffer += 1;
	if ((newDir->SVMData)->DAttributes == (0x01 | 0x02 | 0x04 | 0x08))
		dontAdd = true;
	else
		VMDirCopy((newDir->SVMData)->DShortFileName, Temp);

	//ignore NTRes
	buffer += 1;

	uint8_t tenthTemp[1];
	memcpy(tenthTemp, buffer, 1);
	(newDir->SVMData)->DCreate.DHundredth = tenthTemp[0] % 100;
	buffer += 1;

	uint16_t tempCrtTime[1];
	memcpy(tempCrtTime, buffer, 2);
	(newDir->SVMData)->DCreate.DHour = (tempCrtTime[0] >> 11);
	(newDir->SVMData)->DCreate.DMinute = (tempCrtTime[0] >> 5) & 0x3F;
	(newDir->SVMData)->DCreate.DSecond = (tempCrtTime[0] & 0x1F) << 1;
	buffer += 2;

	uint16_t tempCrtDate[1];
	memcpy(tempCrtDate, buffer, 2);
	(newDir->SVMData)->DCreate.DYear = (tempCrtDate[0] >> 9) + 1980;
	(newDir->SVMData)->DCreate.DMonth = (tempCrtDate[0] >> 5) & 0xF;
	(newDir->SVMData)->DCreate.DDay = (tempCrtDate[0] & 0x1F);
	buffer += 2;

	uint16_t tempAccDate[1];
	memcpy(tempAccDate, buffer, 2);
	(newDir->SVMData)->DAccess.DYear = (tempAccDate[0] >> 9) + 1980;
	(newDir->SVMData)->DAccess.DMonth = (tempAccDate[0] >> 5) & 0xF;
	(newDir->SVMData)->DAccess.DDay = (tempAccDate[0] & 0x1F);
	buffer += 2;

	//Ignore high cluster
	buffer += 2;

	uint16_t tempWrtTime[1];
	memcpy(tempWrtTime, buffer, 2);
	(newDir->SVMData)->DModify.DHour = (tempWrtTime[0] >> 11);
	(newDir->SVMData)->DModify.DMinute = (tempWrtTime[0] >> 5) & 0x3F;
	(newDir->SVMData)->DModify.DSecond = (tempWrtTime[0] & 0x1F) << 1;
	buffer += 2;

	uint16_t tempWrtDate[1];
	memcpy(tempWrtDate, buffer, 2);
	(newDir->SVMData)->DModify.DYear = (tempWrtDate[0] >> 9) + 1980;
	(newDir->SVMData)->DModify.DMonth = (tempWrtDate[0] >> 5) & 0xF;
	(newDir->SVMData)->DModify.DDay = (tempWrtDate[0] & 0x1F);
	buffer += 2;

	memcpy(newDir->clusLow, buffer, 2);
	buffer += 2;

	(newDir->SVMData)->DSize = *buffer;
	buffer += 4;

	if (!dontAdd)
	{
		currentDirs.push_back(newDir);
		FD_Map[currentDirs.size()] = 3 + currentDirs.size();
		newDir->openFD = FD_Map[currentDirs.size()];
	}
	else if (freeDir)
	{
		FD_Map[currentDirs.size()] = 3 + currentDirs.size();
		newDir->openFD = FD_Map[currentDirs.size()];
		freeDirs.push_back(newDir);
	}
}

void printDir()
{
	std::cout << std::endl;
	for (unsigned int i = 0; i < currentDirs.size(); i++)
	{
		std::cout << "File Name: ";
		for (unsigned int x = 0; x < 13; x++)
			std::cout << (currentDirs[i]->SVMData)->DShortFileName[x];
		std::cout << "  First Cluster: " << currentDirs[i]->clusLow[0];
		std::cout << "  Size: " << (currentDirs[i]->SVMData)->DSize;
		std::cout << "  Creation Date/Time: " << (unsigned int)(currentDirs[i]->SVMData)->DCreate.DMonth << "/" << (unsigned int)(currentDirs[i]->SVMData)->DCreate.DDay << "/" << (unsigned int)(currentDirs[i]->SVMData)->DCreate.DYear;
		std::cout << " " << (unsigned int)(currentDirs[i]->SVMData)->DCreate.DHour << ":" << (unsigned int)(currentDirs[i]->SVMData)->DCreate.DMinute << ":" << (unsigned int)(currentDirs[i]->SVMData)->DCreate.DSecond;
		std::cout << std::endl;
		std::cout << "     Access Date: " << (unsigned int)(currentDirs[i]->SVMData)->DModify.DMonth << "/" << (unsigned int)(currentDirs[i]->SVMData)->DModify.DDay << "/" << (unsigned int)(currentDirs[i]->SVMData)->DModify.DYear;
		std::cout << "  Modify Date/Time: " << (unsigned int)(currentDirs[i]->SVMData)->DModify.DMonth << "/" << (unsigned int)(currentDirs[i]->SVMData)->DModify.DDay << "/" << (unsigned int)(currentDirs[i]->SVMData)->DModify.DYear;
		std::cout << " " << (unsigned int)(currentDirs[i]->SVMData)->DCreate.DHour << ":" << (unsigned int)(currentDirs[i]->SVMData)->DCreate.DMinute << ":" << (unsigned int)(currentDirs[i]->SVMData)->DCreate.DSecond;
		std::cout << std::endl;
		std::cout << std::endl;
	}
}

void printFat()
{
	std::cout << "FAT Out: " << std::endl;

	for (unsigned int i = 0; i < FAT_Vector.size(); i++)
	{
		std::cout << std::hex << FAT_Vector[i] << " ";
		if (i % 25 == 0)
			std::cout << std::endl;
	}
}

void readCluster(uint16_t clusNum, unsigned int size)
{
	//Get Offset using Cluster Num (Cluster Low)
	uint16_t offset = 0;
	uint16_t dataOffset = 0;
	ClusterRef newClus = new Cluster(clusNum, 0, size);
	bool endOfClus = false;
	while (!endOfClus)
	{
		offset = (FirstDataSector + ((clusNum - 2) * *Main_BPB.SecPerClus)) * 512;
		//offset already accounts for FirstDataSector, so the second argument of FileSeek can just be offset
		for (unsigned int x = 0; x < *Main_BPB.SecPerClus; x++)	//Might need to account for size
		{
			readSector(0, offset);
			memcpy((newClus->data + dataOffset), SHARED_BUFFER, 512);
			offset += 512; dataOffset += 512;
		}
		clusNum = FAT_Vector[clusNum];
		if (clusNum >= 0xFFF8)
			endOfClus = true;
	}
	//clusterCache - map for open clusters
	clusterCache[clusterCache.size()] = newClus;
}

void writeCluster(uint16_t clusNum, void *data, unsigned int size)
{
	uint16_t offset = 0;
	uint16_t dataOffset = 0;
	ClusterRef newClus = new Cluster(clusNum, 1, size);
	int lenLeft = size;
	while (lenLeft > 0)
	{
		offset = (FirstDataSector + ((clusNum - 2) * *Main_BPB.SecPerClus)) * 512;
		for (unsigned int x = 0; x < *Main_BPB.SecPerClus; x++)
		{
			//Read the data from shared buffer into the correct cluster location. If you need a new cluster, find a free cluster and link to it
			memcpy((newClus->data + dataOffset), data + dataOffset, 512);
			offset += 512; dataOffset += 512; lenLeft -= 512;
		}
		clusNum = FAT_Vector[clusNum];
		if (clusNum >= 0XFFF8 && lenLeft > 0)
		{
			uint16_t newNum;
			findFreeClus(&newNum);
			FAT_Vector[clusNum] = newNum;
		}
	}
	clusterCache[clusterCache.size()] = newClus;
}

void readSector(unsigned int location, unsigned int offset)
{
	//Allocate before calling
	//Seek First
	VMMutexCreate(&FAT_Mutex);
	VMMutexAcquire(FAT_Mutex, VM_TIMEOUT_INFINITE);

	MachineFileSeek(IMAGE_FD, location + offset, 0, fileCallBack, CURRENT_THREAD);
	pushFileToWait(CURRENT_THREAD);
	scheduleThread();

	//Read
	MachineFileRead(IMAGE_FD, SHARED_BUFFER, 512, fileCallBack, CURRENT_THREAD);
	pushFileToWait(CURRENT_THREAD);
	scheduleThread();

	VMMutexRelease(FAT_Mutex);

}
void writeSector()
{

}

void searchRoot(const char *fileName, int *fd, DirEntryRef entryRef)
{
	bool found = false;
	for (unsigned int x = 0; x < currentDirs.size(); x++)
	{
		if (strcmp(fileName, (currentDirs[x]->SVMData)->DShortFileName) == 0)
		{
			found = true;
			*entryRef = *currentDirs[x];
			*fd = FD_Map[x];
		}
	}
	if (!found)
		*fd = -1;
}

void findOpenFile(unsigned int *vecOffSet, int fd)
{
	for (unsigned int x = 0; x < openFiles.size(); x++)
	{
		if (openFiles[x]->openFD == fd)
			*vecOffSet = x;
	}
}

void findOpenDir(unsigned int *vecOffSet, int fd)
{
	for (unsigned int x = 0; x < openDirect.size(); x++)
	{
		if (openDirect[x]->openFD == fd)
			*vecOffSet = x;
	}
}

bool findClusCache(uint16_t clusLow, unsigned int *pos)
{
	for (unsigned int x = 0; x < clusterCache.size(); x++)
	{
		if (clusLow == clusterCache[x]->clusNum)
		{
			*pos = x;
			return true;
		}
	}
	*pos = -1;
	return false;
}

void findFreeClus(uint16_t *clusLow)
{
	bool found = false;
	unsigned int x = 0;
	unsigned int position = 0;
	while (!found)
	{
		if (FAT_Vector[x] == 0)
		{
			position = x;
			FAT_Vector[x] = 0xFFFF;
			*clusLow = position;
			found = true;
		}
		x++;
	}
}

void VMDirCopy(char *dest, const char *src)
{
	/*							Remake of Nitta's function in Utils
	    while(*src && n){
        n--;
        *dest++ = *src++;
    }
    *dest = '\0';
	*/
	uint32_t x = 8;
	while (*src && x)
	{
		x--;
		*dest++ = *src++;
	}
	*dest++ = '.';
	//*src++;
	x = 3;
	while (*src && x)
	{
		x--;
		*dest++ = *src++;
	}
	*dest = '\0';
}


}//C EXTERN
