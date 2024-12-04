#include <iostream>
#include <pthread.h>
#include <sys/syscall.h> // For syscall() and SYS_gettid
#include <unistd.h>      // For getpid()
#include <vector>

#include <detectr.hpp>

#define NRANKS 4  // number of threads
#define NTASKS 4  // number of tasks per thread

// Define a mutex
pthread_mutex_t printMutex;

// Function to be executed by a thread
void* threadFunction(void* arg) {

    int id = *(int*)arg;
    
    // ovni init thread
    INSTRUMENTATION_THREAD_INIT();

    // Get the process ID (PID) and thread ID (TID)
    pid_t pid = getpid();              // Process ID
    pid_t tid = syscall(SYS_gettid);   // Thread ID

    // Lock the mutex before printing
    pthread_mutex_lock(&printMutex);
    std::cout << "Thread " << id << " is running."
              << " PID: " << pid << ", TID: " << tid << std::endl;
    // Unlock the mutex after printing
    pthread_mutex_unlock(&printMutex);

    // running tasks
    for(int i = 0; i < NTASKS; ++i) {
        uint32_t taskid = id*NTASKS + i;

        INSTRUMENTATION_TASK_INIT(taskid);

        INSTRUMENTATION_TASK_EXEC(taskid);

        std::cout << "Thread " << id << " is running task: " << taskid << std::endl;

        INSTRUMENTATION_TASK_END(taskid);
    }


    // ovni free thread
    INSTRUMENTATION_THREAD_END();

    return nullptr;
}

int main() {
    // ovni proc init
    INSTRUMENTATION_START();

    std::vector<pthread_t> threads(NRANKS);  // Vector to hold pthreads
    std::vector<int> threadIds(NRANKS);      // Vector to hold thread IDs

    // Initialize the mutex
    pthread_mutex_init(&printMutex, nullptr);

    // Create NRANKS number of threads
    for (int i = 0; i < NRANKS; ++i) {
        threadIds[i] = i;  // Assign thread ID
        pthread_create(&threads[i], nullptr, threadFunction, &threadIds[i]);
    }

    // Wait for all threads to finish
    for (int i = 0; i < NRANKS; ++i) {
        pthread_join(threads[i], nullptr);
    }

    std::cout << "All threads have finished." << std::endl;

    // Destroy the mutex
    pthread_mutex_destroy(&printMutex);

    // ovni free proc
    INSTRUMENTATION_END();

    return 0;
}
