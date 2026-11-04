# DisasterServer on C++

<details open>

<summary>Progress</summary>

### Characters
- [x] Tails
- [x] Knuckles
- [x] Eggman
- [x] Amy
- [x] Cream
- [x] Sally
- [x] Original Exe
- [x] Chaos 
- [x] Exetior
- [x] Exeller
### Maps
- [x] Hide and Seek 2
- [x] Ravine Mist
- [x] ...
- [x] Desert Town
- [x] You Can't Run
- [x] Limp City
- [x] Not Perfect
- [x] Kind and Fair
- [x] Act 9
- [x] Nasty Paradise
- [x] Priceless Freedom
- [x] Volcano Valley
- [x] Hill
- [x] Majin Forest
- [x] Hide and Seek
- [x] Torture Cave
- [x] Dark Tower
- [x] Haunting Dream
- [x] Mystic Wood
- [x] Echidna Ruins
- [x] Fart Zone

</details>


## Building source
#### For Microsoft Windows

Required Installing 
1. [Git](https://git-scm.com/)
2. [Microsoft Visual Studio](https://visualstudio.microsoft.com)
3. [vcpkg](https://github.com/microsoft/vcpkg.git)

```bash
git clone --recursive https://github.com/AX88M2/DisasterServer.git
cmake --preset windows-release
cmake --build build/windows-release
```

#### For GNU Linux
Required Installing
1. [Git](https://git-scm.com/)
2. [vcpkg](https://github.com/microsoft/vcpkg.git)
```bash
git clone --recursive https://github.com/AX88M2/DisasterServer.git
cmake --preset linux-release
cmake --build build/linux-release
```

### ...or build with the helper scripts

Both scripts check dependencies, verify `VCPKG_ROOT`, and run the correct `cmake --preset` + `cmake --build` for your platform.

#### Linux
```bash
chmod +x build_linux.sh
./build_linux.sh
```

#### Windows
`build_windows.sh` is a bash script, so it must run in a shell where `cl.exe` is already on `PATH`. Either:

- **x64 Native Tools Command Prompt for VS** → run `bash`, then:
  ```bash
  cd /c/path/to/DisasterServer
  ./build_windows.sh
  ```
- **Git Bash** after loading the VS environment:
  ```bash
  cmd //c "call \"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat\" && bash"
  cd /c/path/to/DisasterServer
  ./build_windows.sh
  ```

If `cl.exe` is not on `PATH`, the script will stop with a hint pointing to `vcvars64.bat`.
