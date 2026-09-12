# DisasterServer

## Building source
```bash
git clone --recursive https://github.com/AX88M2/DisasterServer.git
cmake -S . -B build && cmake --build build
```

# Alternative building (without UI)
```bash
cmake -S . -B build
cmake --build build --config Release
```

# Alternatvive building (with UI)
```bash
cmake -B build -S . -G "MinGW Makefiles" -DBUILD_UI=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

If using ninja, then:

```bash
cmake -B build -S . -DBUILD_UI=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```