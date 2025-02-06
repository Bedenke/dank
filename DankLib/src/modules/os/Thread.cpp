#include "modules/os/Thread.h"
#include <assert.h>
#include <pthread.h>

using namespace dank;

namespace dank {

	class pThreadData : public ThreadData {
	public:
		pthread_t tID;
        bool running;

		pThreadData() {
			tID = NULL;
            running = false;
		}
		~pThreadData() {
		}
	};

}

Runnable::~Runnable(){};

void *startThreadRunnable(void* args) {
    Runnable* runnable = (Runnable*) args;
    runnable->run();
    return 0;
}

bool Thread::start(Runnable* runnable) {
	this->runnable = runnable;
	this->data = new pThreadData();

	pThreadData* td = (pThreadData*) data;
    if (td->running) return false;

    td->running = true;
    int tErr = pthread_create(&td->tID , NULL, startThreadRunnable, runnable);
	return tErr == 0;
}

void Thread::join() {
    pThreadData* td = (pThreadData*) data;
    if (td->running && td->tID != NULL) {
        pthread_join(td->tID, NULL);
    }
}
