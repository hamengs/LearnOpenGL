# LearnOpenGL

Small CMake-based LearnOpenGL project.

## macOS setup

Install the required tools and GLFW:

```sh
brew install cmake glfw
```

Build and run:

```sh
cmake -S . -B build
cmake --build build
./build/app
```

## Windows setup

From the project root:

```powershell
cmake -S . -B build
cmake --build build
.\build\app.exe
```

The `build` directory is generated locally and should not be committed.
