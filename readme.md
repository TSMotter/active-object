# Active Objects and Hierarchical State Machines (HSM)

- This repository contains an approach of a C++ implementation of the Active Objects (Actors) pattern as is described in the book "Practical UML Statecharts in C/C++: Event-Driven Programming for Embedded Systems - Miro Samek".

- In this pattern, Active Objects (Actors) are event-driven, strictly encapsulated software objects running in their own threads of control that communicate with one another asynchronously exclusively by exchanging events.

- Each Actor object implements a Hierarchical State Machines (HSM)

- The intention here is to create an open source active object platform that can be easily integrated into a C++ project

- There is a folder of sample code that contains different working examples..

    - Each of which contains it's own documentation, for example: [samples/intermediate/readme.md](/samples/intermediate/readme.md)

## Roadmap

- This is working simple implementation but I envision these to be some of the next steps for this project (in no particular order):

1. Implement transition actions
1. More unit tests (specially for `StateManager` class)
1. Add SFINAE for `StateManager`
1. Implement `IState` class that will define an interface for states
1. Currently, there are usages of raw pointers in the `State` classes, I believe that this could be improved
1. Superstates to be able to handle events that are unhandled by child states
1. Create another common level of abstraction that will define an "active object" object, which is composed of all the common components:
    - HSM, event queue, thread and common interface methods like `start`, `stop`, etc
1. Improve "shutdown" of an object to properly stop it's thread
1. Currently, there is no block-less way of sleeping an object.
    - Implement a centralized infrastructure for timers that might have it's own thread of execution and that can exchange events with the objects rather than having the objects call `std::this_thread::sleep_for()` within their own threads
    - Idea is to use boost's `asio::io_service` and `asio::deadline_timer`
1. Improve user experience:
    - Way to describe HSM in a structured language (json? xml? GUI?) and run a python script that would generate the boiler plate code
1. Improve CMake structure to allow compiling this platform into a shared object (.so file) and installing it in a system

## How to operate the repository

- If you wish to use docker to operate the repository, build the image and launch it using the helper scripts inside of the `docker` folder
- The repository can be operated outside of the docker container if all the dependencies are met
- Once the environment is set (either inside or outside the container), the following commands can be issued:

```bash
cmake -S . -B build -D TARGET_GROUP=demo
cmake --build build --parallel `nproc`
```

- Alternatively, use the `bbuild.sh` script, which is an abstraction to `cmake` and `clang-format` commands

- To format the code base with `clang-format`:
```bash
./bbuild.sh -f
```

- To perform an static analysis in the code base with `clang-tidy`:
```bash
./bbuild.sh -s
```

- In the following examples, `<target>` is either `demo` or `test`

- To build:
```bash
./bbuild.sh -b <target>
```

- To rebuild:
```bash
./bbuild.sh -r <target>
```

- To execute the built binary:
```bash
./bbuild.sh -e <target>
```

- To format, analyze, rebuild and execute with verbose turned ON:
```bash
./bbuild.sh -v -f -s -r -e <target>
```

- Examples:
    - `./bbuild.sh -v -f -s -r -e demo`
    - `./bbuild.sh -v -f -s -r -e test`

- To check all options available::
```bash
./bbuild.sh --help
```