# Project Overview

This project is a task execution library designed to handle non-blocking, heavy tasks on a single-core system. It is implemented in **pure C++**, making it a lightweight and efficient solution for task management. The library can also be adapted for **bare-metal systems in C**, providing flexibility for embedded applications. It is designed to be easy to use, with minimal dependencies, making it suitable for both high-performance applications and simple embedded systems.

## Features:
- **Pure C++ implementation**: No dependencies on external libraries.
- **No threads**: The tasks are handled in a non-blocking, efficient manner using state machines.
- **Bare-metal compatibility**: Can be implemented in C for use in bare-metal projects.
- **Efficient and lightweight**: Optimized for performance and minimal overhead.
- **Easy to use**: The API is simple to integrate into existing systems.

## Diagram Overview

The diagram below represents the execution flow of tasks (such as frame processing) over multiple stages, showing how tasks are distributed across different frames and their execution costs over time.

### Task Execution Table (Frame vs. Time)


| T/F | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | a |
|-----|---|---|---|---|---|---|---|---|---|---|---|
| 0   | 0 | 1 | 2 | 3 |   |   |   |   |   |   |   |
| 1   |   | 0 | 1 | 2 | 3 |   |   |   |   |   |   |
| 2   |   |   | 0 | 1 | 2 | 3 |   |   |   |   |   |
| 3   |   |   |   | 0 | 1 | 2 | 3 |   |   |   |   |
| 4   |   |   |   |   | 0 | 1 | 2 | 3 |   |   |   |
| 5   |   |   |   |   |   | 0 | 1 | 2 | 3 |   |   |
| 6   |   |   |   |   |   |   | 0 | 1 | 2 | 3 |   |
| 7   |   |   |   |   |   |   |   | 0 | 1 | 2 | 3 |
| 8   |   |   |   |   |   |   |   |   |   |   |   |
|     |   |   |   | L | L | L | L | L |   |   |   |


### Frame/Cost Breakdown

- **Frame 0**
  - 101 |--->

- **Frame 1**
  - 101 |--->
  - 57  |--------->

- **Frame 2**
  - 101 |--->
  - 57  |--------->
  - 94  |------>

- **Frame 3**
  - 101 |--->
  - 57  |--------->
  - 94  |------>
  - 193 |---->

- **Frame 4**
  - 101 |--->
  - 57  |--------->
  - 94  |------>
  - 193 |---->

- **Frame 5**
  - ...

## How it Works

The library uses a state-machine approach to manage task execution. Each task is executed in a pipeline, where each "frame" represents a stage in the execution of the task. The time for each task is measured, and the execution cost (in terms of time) is tracked for each frame. This allows for efficient task management and scheduling without the need for multi-threading or complex scheduling algorithms.

### Advantages:
- **Efficiency**: By using a state-machine approach and avoiding threads, the system can manage multiple tasks concurrently without incurring the overhead of thread management.
- **Flexibility**: The algorithm can be used on both bare-metal systems (in C) and higher-level embedded systems (in C++).
- **Minimal dependencies**: The library is written in pure C++, making it lightweight and easy to integrate into existing projects without external dependencies.
- **Easy Integration**: The API is designed for ease of use, making it simple to integrate into both new and existing projects.

This approach ensures that tasks are handled in an optimal way, improving the performance of systems running on single-core processors and reducing the need for multi-threading, which can often be cumbersome in embedded and bare-metal systems.




## How to Build

To build the project, you'll need **CMake** and a C++ compiler (such as GCC). Follow these steps to configure and build the project:

1. Clone the repository
2. Create a build directory:
```
mkdir build
cd build
```

3. Run CMake to configure the project. To enable tests, you can set the BUILD_TESTS option:

```
cmake ../ -DBUILD_TESTS=ON
```

4. Build the project:

```
cmake --build .
```

5. Run the test:

```
./test_pipe_state_machine
```

