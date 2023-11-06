# Practice OpenGL

## References

### CMake and Project settings
* [OpenGLSampleCmake](https://github.com/tgalaj/OpenGLSampleCmake/tree/master)

### OpenGL
* [LearnOpenGL](https://learnopengl.com/)



---

## Building on Win32

### Prerequisites

Install the following dependencies

- CMake (to download click [here](https://cmake.org/download/))
- IDE (e.g. [Visual Studio](https://visualstudio.microsoft.com/vs/community/))

### Build instructions

1. Make sure that CMake is installed and added to the system PATH variable.
2. Open terminal in the CMakeLists.txt directory and run:

```
  mkdir build
  cd build
  cmake -A Win32 ..
```

**Note:** The project must be configured as Win32, due to precompiled x86 binaries of thirdparty libraries.

---

## Building on Ubuntu

* The following recipe was successfully tested on [Ubuntu 20.04](https://releases.ubuntu.com/20.04/) platform.
* So far, we haven't tested [macOS](https://en.wikipedia.org/wiki/MacOS), nor any other Unix flavors.
* However, we anticipate most Linux distros should build similarly, aside from a few package-management tweaks.
* So... if you're developing on those platforms, please feel free to contribute :-)

### Prerequisites

```bash
  sudo apt update
  sudo apt install -y build-essential cmake mesa-common-dev mesa-utils freeglut3-dev
```

### Build & run

```
  mkdir build
  cd build
  cmake ..
  make -j4
  ./OpenGLExample
```

---
