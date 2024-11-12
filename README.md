# Project: Semaphore and Shared Memory Communication in C

## Project Overview

This project implements an inter-process communication (IPC) mechanism in C using semaphores, shared memory, and signal handling to simulate communication between processes. The project showcases process synchronization and data exchange using semaphores and shared memory segments. The program coordinates three child processes to perform different tasks, ensuring safe and consistent data transfer using system-level IPC techniques.

## Features

1. **Semaphore Management**: 
   - Utilizes POSIX semaphores for synchronizing processes and controlling access to shared resources.
   - Implements custom functions to increment (`podnies`) and decrement (`opusc`) semaphore values, ensuring atomic access to shared memory.

2. **Shared Memory**:
   - Allocates shared memory segments for storing hexadecimal values produced by different processes.
   - Allows communication and data sharing between parent and child processes.

3. **Signal Handling**:
   - Supports signal-based control to start, stop, and terminate processes gracefully.
   - Signals include `SIGINT` for terminating, `SIGUSR1` for pausing, and `SIGCONT` for resuming processes.

4. **Inter-process Communication**:
   - Three child processes perform different tasks:
     - Process 1: Converts characters from input (keyboard, file, or `/dev/urandom`) to hexadecimal and writes them to shared memory.
     - Process 2: Reads hexadecimal data from shared memory and writes it to a temporary file.
     - Process 3: Reads hexadecimal data from the file and displays it in the terminal.

5. **Pause and Resume Mechanism**:
   - Allows the user to pause and resume data transfer using signals.
   - Implements shared memory-based flags to control process synchronization.
