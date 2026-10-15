#include "scheduler.h"

#include <iomanip>
#include <sstream>

#include "util.h"

Scheduler::Scheduler(int numCores, std::chrono::milliseconds instructionDelay,
                     std::filesystem::path logDir)
    : delay_(instructionDelay), logDir_(std::move(logDir)) {
    for (int i = 0; i < numCores; ++i) {
        cores_.push_back(std::make_unique<Core>());
    }
    std::filesystem::create_directories(logDir_);
}

Scheduler::~Scheduler() { stop(); }

void Scheduler::start() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (started_) return;
    started_ = true;
    for (int i = 0; i < static_cast<int>(cores_.size()); ++i) {
        cores_[i]->thread = std::thread(&Scheduler::coreLoop, this, i);
    }
    schedulerThread_ = std::thread(&Scheduler::schedulerLoop, this);
}

void Scheduler::stop() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!started_ || stopping_) return;
        stopping_ = true;
    }
    schedulerCv_.notify_all();
    for (auto& core : cores_) core->cv.notify_all();

    schedulerThread_.join();
    for (auto& core : cores_) core->thread.join();

    // Close the log files so they can be read (e.g. zipped) after stop() returns.
    for (auto& p : all_) p->log.close();
}

void Scheduler::addProcess(const std::string& name, int totalInstructions) {
    auto p = std::make_unique<Process>();
    p->name = name;
    p->totalInstructions = totalInstructions;
    p->log.open(logDir_ / (name + ".txt"), std::ios::trunc);
    p->log << "Process name: " << name << "\nLogs:\n\n" << std::flush;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        ready_.push_back(p.get());
        all_.push_back(std::move(p));
    }
    schedulerCv_.notify_one();
}

bool Scheduler::hasIdleCore() const {
    for (const auto& core : cores_) {
        if (!core->busy) return true;
    }
    return false;
}

void Scheduler::schedulerLoop() {
    std::unique_lock<std::mutex> lock(mutex_);
    while (true) {
        schedulerCv_.wait(lock, [this] {
            return stopping_ || (!ready_.empty() && hasIdleCore());
        });
        if (stopping_) return;

        // Dispatch in arrival order to the lowest-numbered idle core.
        for (auto& core : cores_) {
            if (ready_.empty()) break;
            if (core->busy) continue;
            core->assigned = ready_.front();
            core->busy = true;
            ready_.pop_front();
            core->cv.notify_one();
        }
    }
}

void Scheduler::coreLoop(int id) {
    Core& core = *cores_[id];
    while (true) {
        Process* p = nullptr;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            core.cv.wait(lock, [&] { return stopping_ || core.assigned != nullptr; });
            if (stopping_) return;
            p = core.assigned;
            core.assigned = nullptr;
            p->state = Process::State::Running;
            p->core = id;
            p->timestamp = ::timestamp();
        }

        run(*p, id);

        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (p->executed >= p->totalInstructions) {
                p->state = Process::State::Finished;
                p->timestamp = ::timestamp();
            }
            core.busy = false;
        }
        schedulerCv_.notify_one();
    }
}

// Executes the process's print commands one by one, logging each to its file.
void Scheduler::run(Process& p, int coreId) {
    for (int i = 0; i < p.totalInstructions; ++i) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (stopping_) return;
        }

        p.log << "(" << timestamp() << ") Core:" << coreId << " \"Hello world from "
              << p.name << "!\"\n"
              << std::flush;

        {
            std::lock_guard<std::mutex> lock(mutex_);
            ++p.executed;
        }
        std::this_thread::sleep_for(delay_);
    }
}

std::string Scheduler::report() {
    std::ostringstream out;
    const std::string rule(37, '-');

    std::lock_guard<std::mutex> lock(mutex_);

    out << rule << "\nRunning processes:\n";
    for (const auto& p : all_) {
        if (p->state != Process::State::Running) continue;
        out << std::left << std::setw(12) << p->name << " (" << p->timestamp << ")   Core: "
            << p->core << "   " << p->executed << " / " << p->totalInstructions << "\n";
    }

    out << "\nFinished processes:\n";
    for (const auto& p : all_) {
        if (p->state != Process::State::Finished) continue;
        out << std::left << std::setw(12) << p->name << " (" << p->timestamp << ")   "
            << std::setw(11) << "Finished" << p->executed << " / " << p->totalInstructions
            << "\n";
    }
    out << rule << "\n";
    return out.str();
}
