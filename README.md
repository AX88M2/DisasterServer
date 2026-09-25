# DisasterServer

## Building source
```bash
git clone --recursive  --branch 1101-cxx https://github.com/AX88M2/DisasterServer.git
cmake -S . -B build && cmake --build build
```

## Alternative method building source
```bash
cmake -S . -B build
cmake --build build --config Release
```

## Building source from linux
```bash
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=/mnt/c/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-linux-release -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
```
