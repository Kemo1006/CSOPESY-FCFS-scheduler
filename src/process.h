#pragma once

#include <fstream>
#include <string>

struct Process {
    enum class State { Ready, Running, Finished };

    std::string name;
    int totalInstructions = 0;
    int executed = 0;
    int core = -1;
    State state = State::Ready;
    // Start time while running, finish time once finished.
    std::string timestamp;
    std::ofstream log;
};
