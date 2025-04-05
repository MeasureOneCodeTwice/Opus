# About

Opus is a work-in-progress Minecraft Discord bot written in C.
There are two ways to run Opus:
1. Natively on your machine
2. Through docker

I recommend running opus with the makefile as it is simpler, 
and if you encounter any issues with dependencies, try docker instead.


## Dependencies 

##### Using docker
- just **Docker**

##### Running directly
If you opt to use the `Makefile` directly, please check the `dependencies` file for a comprehensive list of the project's dependencies. The dependencies listed in the `dependencies` file are for a minimal installation of Alpine Linux.

- **Make**
- **Clang**
- **Valgrind***
- **Bash*** 

*required only for running test suite

> **Note**: If you're using a package-rich distribution like Ubuntu, some dependencies in the `dependencies` file (e.g., `musl-dev` for basic C libraries) may already be installed.

## Execution

You can choose to run either the discord bot or the test suite.

### Build & Run with Docker 

##### Opus
```bash
docker build -t Opus .
docker run Opus
```

##### Opus test suite
```bash
docker build --build-arg directory=tests -t Opus/tests .
docker run Opus/tests
```

### Makefile 

The command to run the test suite is the same as running the program,
but to run tests you must be in the `tests` directory.
```bash
make run
```
