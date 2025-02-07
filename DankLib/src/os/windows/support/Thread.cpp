#include "modules/os/Thread.h"
#include <Windows.h>
#include <process.h>

using namespace dank;

namespace dank {

class WindowsThreadData : public ThreadData {
public:
  HANDLE hThread;
  unsigned wThreadID;

  WindowsThreadData() {
    hThread = NULL;
    wThreadID = 0;
  }
  ~WindowsThreadData() {}
  static unsigned WINAPI startThreadRunnable(LPVOID pVoid) {
    Thread *runnableThread = static_cast<Thread *>(pVoid);
    runnableThread->runnable->run();
    WindowsThreadData *td = (WindowsThreadData *)runnableThread->data;
    CloseHandle(td->hThread);
    return 0;
  }
};

} // namespace dank

// Pure virtual destructor: function body required
Runnable::~Runnable() {};

bool Thread::start(Runnable *runnable) {
  this->runnable = runnable;
  WindowsThreadData *td = new WindowsThreadData();
  this->data = td;
  td->hThread =
      (HANDLE)_beginthreadex(NULL, 0, WindowsThreadData::startThreadRunnable,
                             (LPVOID)this, CREATE_SUSPENDED, &td->wThreadID);
  DWORD rc = ResumeThread(td->hThread);
  return rc ? true : false;
}