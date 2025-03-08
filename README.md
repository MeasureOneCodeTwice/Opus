# About

Opus is a work-in-progress Minecraft Discord bot written in C.

---

## Installation

You can run Opus using either `Docker` or directly with the `Makefile`.

### Option 1: Docker (Recommended for simplicity)

- **Docker** is the only dependency required for this option.

### Option 2: Using Makefile

If you opt to use the `Makefile` directly, please check the `dependencies` file for a comprehensive list of the project's dependencies. The dependencies listed in the `dependencies` file are for a minimal installation of Alpine Linux.

> **Note**: If you're using a package-rich distribution like Ubuntu, some dependencies (e.g., `musl-dev` for basic C libraries) may already be installed.

#### Required Dependencies (for Makefile option)
- **Make**
- **Clang**
- **Valgrind***
- **Bash*** <br> <br> <br>
*required only for running test suite
## Building

This step is only necessary if you're using Docker to run Opus.

**Build the main image**:

```bash
docker build -t Opus .
```

**Build the test suite image**:

```bash
docker build --build-arg directory=tests -t Opus/tests .
```

## Running

You can run Opus either through the `Makefile` or `Docker`.

### Option 1: Using Makefile (Recommended)

**Run the main program**:

```bash
make run
```

**Run the test suite** (navigate to `tests` directory first):

```bash
make run
```

### Option 2: Using Docker

**Run the main program**:

```bash
docker run Opus
```

**Run the test suite**:

```bash
docker run Opus/tests
```
