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

If `app.exe` exits immediately or double-clicking it shows no window, run it from
PowerShell to see the exit code:

```powershell
.\build\app.exe
echo $LASTEXITCODE
```

Exit code `-1073741515` (`0xC0000135`) means Windows could not find a required
DLL before the program reached `main()`. This project links Assimp through
`lib/libassimp.dll.a`, which is only an import library. At runtime, Windows also
needs the matching MinGW/UCRT DLL:

```text
libassimp-6.dll
```

Put `libassimp-6.dll` either next to `build/app.exe` or in `lib/`, then rebuild:

```powershell
cmake -S . -B build
cmake --build build
.\build\app.exe
```

The CMake build copies MinGW runtime DLLs into `build/` automatically and also
copies `libassimp-6.dll` when it is present in `lib/`.

The `build` directory is generated locally and should not be committed.
