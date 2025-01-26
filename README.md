<h1 align="center">
    Multi-core Scheduling Simulator with Concurrent Queue Algorithm
</h1>

<div align="center">

[![PL](https://img.shields.io/badge/C-blue?style=for-the-badge&logo=c&logoColor=white)]()
[![Status](https://img.shields.io/badge/status-completed-green?style=for-the-badge)]()
[![License](https://img.shields.io/badge/license-MIT-red?style=for-the-badge)](/LICENSE)

</div>

Modern computing systems increasingly rely on multi-core processors to enhance computational performance. However, efficiently distributing tasks across multiple cores presents significant challenges in load balancing, synchronization, and maintaining system efficiency.

This simulator addresses these challenges by implementing an advanced task scheduling mechanism that dynamically balances workloads while minimizing performance overhead.

## Table of Contents
- [Concurrent Queue Design](#concurrent-queue-design)
- [Load Balancing Strategy](#load-balancing-strategy)
- [Compilation and Execution](#compilation-and-execution)
- [Potential Research Directions](#potential-research-directions)
- [Core Concepts](#core-concepts)
- [Performance Philosophy](#performance-philosophy)
- [License](#license)

## Concurrent Queue Design

### Two-Lock Synchronization Mechanism

The core of the simulator is a specialized concurrent queue implementation inspired by Michael and Scott's algorithm, featuring:

#### Structural Characteristics
- Doubly-linked list structure
- Dummy node at queue head
- Separate mutex locks for head and tail operations

#### Key Synchronization Features
- Minimal critical section locking
- Independent head and tail modifications
- Atomic queue size tracking
- Thread-safe task insertion and retrieval

### Synchronization Operation Details

#### Task Insertion (`submitTask`)
- Uses `tail_lock` to prevent concurrent modifications
- Atomic size incrementation
- Minimal critical section to reduce contention

#### Task Retrieval (`fetchTask`)
- Uses `head_lock` for thread-safe dequeuing
- Maintains strict FIFO order for owner thread
- Atomic size decrementing

#### Inter-Core Task Stealing (`fetchTaskFromOthers`)
- Acquires both `head_lock` and `tail_lock`
- Enables cross-core load balancing
- Prevents race conditions during task migration

## Load Balancing Strategy

### Dynamic Threshold Mechanism

The load balancing approach employs sophisticated adaptive techniques:

- Calculates average queue size across all cores
- Computes absolute deviation to dynamically adjust thresholds
- Implements High Watermark (HW) and Low Watermark (LW)

### Work-Stealing Algorithm Characteristics

- Underloaded cores identify and target overloaded cores
- Minimizes unnecessary task migrations
- Preserves cache affinity by intelligent task selection

### Performance Trade-offs

Key considerations in the design:
- Balancing load distribution against cache affinity
- Minimizing synchronization overhead
- Maintaining predictable performance characteristics


## Compilation and Execution

```bash
# Compile the simulator
make sim

# Run the simulator with a task file
./main task1.txt
```

## Potential Research Directions

- Integration of task duration complexity metrics
- Advanced lock-free synchronization techniques for better performance.
- Adaptive migration algorithms for diverse workloads

## Core Concepts

- Concurrent programming paradigms
- Multi-core scheduling algorithms
- POSIX thread synchronization
- Performance-critical system design

## Performance Philosophy

The simulator tackles multi-core scheduling by prioritizing:
- Minimal synchronization overhead
- Dynamic workload adaptation
- Preservation of computational efficiency

## License
This project is licensed under the MIT License - see the [LICENSE](/LICENSE) file for details.
