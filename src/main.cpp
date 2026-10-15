#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "scheduler.h"

namespace {
constexpr int kNumCores = 4;
constexpr int kNumProcesses = 10;
constexpr int kPrintsPerProcess = 100;
// Delay after each print so progress is visible in "screen -ls"
// (100 ms -> ~10 s per process, ~30 s for all 10 on 4 cores).
constexpr std::chrono::milliseconds kInstructionDelay{100};
const char* const kLogDir = "logs";
}  // namespace

int main() {
    Scheduler scheduler(kNumCores, kInstructionDelay, kLogDir);

    for (int i = 1; i <= kNumProcesses; ++i) {
        std::ostringstream name;
        name << "process" << std::setw(2) << std::setfill('0') << i;
        scheduler.addProcess(name.str(), kPrintsPerProcess);
    }
    scheduler.start();

    std::string line;
    while (std::cout << "root:\\> " << std::flush, std::getline(std::cin, line)) {
        // Tolerate stray whitespace around the command.
        const auto first = line.find_first_not_of(" \t\r");
        line = (first == std::string::npos)
                   ? ""
                   : line.substr(first, line.find_last_not_of(" \t\r") - first + 1);

        if (line == "screen -ls") {
            std::cout << scheduler.report();
        } else if (line == "exit") {
            break;
        } else if (!line.empty()) {
            std::cout << "Unknown command: " << line << "\n";
        }
    }

    scheduler.stop();

#ifdef _WIN32
    // Bundle the log files for submission (written next to the executable's working dir).
    std::string cmd = std::string("powershell -NoProfile -Command \"Compress-Archive -Path '") +
                      kLogDir + "\\*.txt' -DestinationPath 'logs.zip' -Force -ErrorAction Stop\"";
    if (std::system(cmd.c_str()) == 0 && std::filesystem::exists("logs.zip")) {
        std::cout << "Logs zipped to logs.zip\n";
    } else {
        std::cout << "Failed to create logs.zip\n";
    }
#endif
    return 0;
}
