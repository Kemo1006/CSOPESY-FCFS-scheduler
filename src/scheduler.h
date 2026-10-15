#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "process.h"

// Non-preemptive first-come-first-serve scheduler.
// One scheduler thread hands the head of the ready queue to an idle core;
// each core has its own worker thread that runs the process to completion.
class Scheduler {
public:
    Scheduler(int numCores, std::chrono::milliseconds instructionDelay,
              std::filesystem::path logDir);
    ~Scheduler();

    Scheduler(const Scheduler&) = delete;
    Scheduler& operator=(const Scheduler&) = delete;

    void start();
    void stop();

    // Creates the process and its log file, then appends it to the ready queue.
    void addProcess(const std::string& name, int totalInstructions);

    // Text for the "screen -ls" command.
    std::string report();

private:
    struct Core {
        std::thread thread;
        std::condition_variable cv;
        Process* assigned = nullptr;  // set by the scheduler, taken by the worker
        bool busy = false;            // true from assignment until the process finishes
    };

    void schedulerLoop();
    void coreLoop(int id);
    void run(Process& p, int coreId);
    bool hasIdleCore() const;

    std::chrono::milliseconds delay_;
    std::filesystem::path logDir_;

    std::mutex mutex_;
    std::condition_variable schedulerCv_;
    std::deque<Process*> ready_;
    std::vector<std::unique_ptr<Process>> all_;  // creation order
    std::vector<std::unique_ptr<Core>> cores_;
    std::thread schedulerThread_;
    bool stopping_ = false;
    bool started_ = false;
};
