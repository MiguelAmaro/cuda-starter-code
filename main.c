#include <stdio.h>
#include <stdint.h>
#include "mycuda.h"
#include "windows.h"

int GetLogicalCoreCount(void)
{
  //https://stackoverflow.com/questions/28893786/how-to-get-the-number-of-actual-cores-on-the-cpu-on-windows
  SYSTEM_INFO SystemInfo = {0};
  SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX ProcessorInfo= {0};
  DWORD Length = 0;
  GetSystemInfo(&SystemInfo);
  GetLogicalProcessorInformationEx(RelationProcessorPackage, &ProcessorInfo, &Length);
  uint32_t Result = SystemInfo.dwNumberOfProcessors;
  return Result;
}

typedef DWORD threadproc(void *);
uint64_t ThreadCreate(void *Param, threadproc ThreadProc)
{
  DWORD ThreadId;
  HANDLE ThreadHandle = CreateThread(0, 0, ThreadProc, Param, 0, &ThreadId);
  return (uint64_t)ThreadHandle;
}
void ThreadKill(void)
{
  ExitThread(0);
  return;
}
void ThreadSync(uint64_t *ThreadHandles, uint32_t ThreadCount)
{
  WaitForMultipleObjects(ThreadCount, (HANDLE *)ThreadHandles, TRUE, INFINITE);
  return;
}
DWORD ThreadProc(void *Param)
{
  uint64_t BeginSignal = *((uint64_t *)Param);
  WaitForSingleObject((HANDLE)BeginSignal, INFINITE); 
  printf("hello from thread[%5lu]\n", GetCurrentThreadId());
  return 0;
}
void ThreadBeginWork(uint64_t BeginSignal)
{
  if(SetEvent((HANDLE)BeginSignal)) { fprintf(stderr, "work begin signal set.\n"); }
  else                              { fprintf(stderr, "Error setting work queue begin signal.\n"); }
  return;
}
void ThreadLaunchGroup(uint64_t *ThreadHandles, int ThreadCount, uint64_t *BeginSignal)
{
  for(uint32_t ThreadIdx=0; ThreadIdx<ThreadCount; ThreadIdx++)
  {
    uint64_t ThreadHandle = ThreadCreate(BeginSignal, ThreadProc);
    printf("thread launched... id:%lu\n", GetThreadId((HANDLE)ThreadHandle));
    ThreadHandles[ThreadIdx] = ThreadHandle;
  }
  return;
}
int main(void)
{
  printf("hello from c\n");
  
  printf("parallel cpu c\n");
  uint32_t  LogicalCoreCount = GetLogicalCoreCount();
  uint64_t *ThreadHandles = VirtualAlloc(0, LogicalCoreCount*sizeof(uint64_t), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
  uint64_t BeginSignal = (uint64_t)CreateEvent(NULL, TRUE, FALSE, TEXT("Thread Group Begin Signal"));
  ThreadLaunchGroup(ThreadHandles, LogicalCoreCount, &BeginSignal);
  ThreadBeginWork(BeginSignal);
  ThreadSync(ThreadHandles, LogicalCoreCount);
  
  printf("parallel gpu c\n");
  CudaRunCodeFromC();
  return;
}