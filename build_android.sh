# SDK
SDK="$LOCALAPPDATA/Android/Sdk"
NDK_VERSION="25.2.9519653"
CMAKE_VERSION="3.22.1"

# Android
PLATFORM="android-19"
declare -a ABI=("armeabi-v7a" "arm64-v8a" "x86_64")

if [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
    OS_TYPE="linux-x86_64"
    FILE_EXT=""
elif [ "$(expr substr $(uname -s) 1 10)" == "MINGW64_NT" ]; then
    OS_TYPE="windows-x86_64"
    FILE_EXT=".exe"
fi

NDK="${SDK}/ndk/${NDK_VERSION}"
CMAKE="${SDK}/cmake/${CMAKE_VERSION}/bin/cmake${FILE_EXT}"
MAKE="${NDK}/prebuilt/${OS_TYPE}/bin/make${FILE_EXT}"
TOOLCHAIN="${NDK}/build/cmake/android.toolchain.cmake"

echo "$CMAKE"

for i in "${ABI[@]}"
do
    echo "[]    Building for $i..."
    "${CMAKE}" -DCMAKE_TOOLCHAIN_FILE="${TOOLCHAIN}" -DANDROID_NDK="${NDK}" -DANDROID_ABI="$i" -DANDROID_PLATFORM="${PLATFORM}" -DCMAKE_MAKE_PROGRAM="${MAKE}" -G "Unix Makefiles" -B"build_android/$i" -H.
    (cd "build_android/$i" && "${MAKE}" -j $(nproc))
done