# CSOPESY-FCFS-scheduler

Multi-threaded first-come-first-serve (FCFS) CPU scheduler for our OS emulator (C++17).

- 1 scheduler thread + 1 worker thread per CPU core (4 cores).
- On startup, 10 processes are created, each with 100 print commands.
- Each process writes its prints to `logs/processNN.txt` with a timestamp and the core that ran it.
- Scheduling is non-preemptive: a process runs to completion on its core.

## Requirements (Windows)

- g++ with C++17 support. We use MSYS2 UCRT64: `C:\msys64\ucrt64\bin\g++.exe` and `gdb.exe`.
- PowerShell (used for the auto-zip on exit).
- Optional: VS Code with the C/C++ extension for F5 debugging. CMake is not required.

## Build and run

### VS Code (what we use for the recording)

Open the project folder and press **F5** (the "Run emulator" configuration). It builds
`build\emulator.exe` and opens a console window running the emulator. If the console window
is hidden, check the taskbar.

### Command line

```powershell
.\build.ps1
cd build
.\emulator.exe
```

(CMake also works: `cmake -S . -B build-cmake; cmake --build build-cmake`. We haven't tested this.)

## Commands

| Command | What it does |
|---|---|
| `screen -ls` | Lists running processes (name, start time, core, progress) and finished processes |
| `exit` | Stops the emulator and zips the logs into `logs.zip` |

Wait for all 10 processes to show as finished before typing `exit`, or the logs will be partial.

## Output

Written to the folder the emulator is run from (`build\` when started with F5):

- `logs\process01.txt` … `logs\process10.txt`, each looking like:
  ```
  Process name: process01
  Logs:

  (10/15/2026 06:02:50PM) Core:0 "Hello world from process01!"
  ```
- `logs.zip`, created automatically on `exit`. This is what we submit.

## Configuration

Constants at the top of [src/main.cpp](src/main.cpp):

| Constant | Default | Meaning |
|---|---|---|
| `kNumCores` | 4 | CPU cores |
| `kNumProcesses` | 10 | Processes created at startup |
| `kPrintsPerProcess` | 100 | Print commands per process |
| `kInstructionDelay` | 100 ms | Delay after each print. A full run takes about 30 s |