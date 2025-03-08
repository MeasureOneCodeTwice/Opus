# Description
Opus is a work in progress minecraft discord bot written in C

# Installation
You can either run the project with `Docker` or directory with the `Makefile`

If you opt to use docker, docker is the sole dependency.

If using the makefile directly check the `dependencies` file for a comprehensive list of the project's dependencies.
Note that the dependencies listed `dependencies` are those for a minimal installation of Alpine linux. 
If you are using a distribution more package rich by default some dependencies such as `musl-dev` which
contains basic C libraries will likely already be installed on you system.

Below is a list of the dependencies which are not unlikely to be missing from an distribution such as Ubuntu.
- Make
- Clang
The following package is also required for running the test suite.
- Valgrind 

# Building 
This step is only required if you intend to run Opus in docker.

To build the main image use
`docker build -t Opus .`
To build the test suite image use 
`docker build --build-arg directory=tests -t Opus/tests .

# Running

### With Makefile (Recommended)
To run the main program run
`make run`
To run the test suite navigate to the `tests` directory and run
`make run`

### With Docker
To run the main program run
`docker run Opus`
To run the test suite run
`docker run Opus/tests`



