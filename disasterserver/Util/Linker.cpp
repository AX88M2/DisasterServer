#include "Linker.hpp"

using namespace DisasterServer;

Linker::Linker(const std::string &name) {
#if WIN32
    handle = LoadLibrary(name.c_str());
#else
    handle = (Handle)dlopen(name.c_str(), RTLD_LOCAL | RTLD_LAZY);
#endif
}

Linker::~Linker() {
    if (handle != nullptr) {
#if WIN32
        FreeLibrary(handle);
#else
        dlclose(handle);
#endif
    }
}

std::string Linker::getError() {
#if WIN32
    return getLastErrorAsString();
#else
    return std::string(dlerror());
#endif
}

void *Linker::getSymbol(const std::string &symbol) {
    void* func = nullptr;
#if WIN32
    func = reinterpret_cast<void *>(GetProcAddress(handle, symbol.c_str()));
#else
    func = reinterpret_cast<void *>(dlsym(handle, symbol.c_str()));
#endif

    if (func != nullptr) {
        return func;
    }

    return nullptr;
}

#if WIN32
std::string Linker::getLastErrorAsString() {
    // Get the error message ID, if any.
    DWORD errorMessageID = GetLastError();
    if(errorMessageID == 0) {
        return std::string(); //No error message has been recorded
    }

    LPSTR messageBuffer = nullptr;

    //Ask Win32 to give us the string version of that message ID.
    //The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 nullptr, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, nullptr);

    //Copy the error message into a std::string.
    std::string message(messageBuffer, size);

    //Free the Win32's string's buffer.
    LocalFree(messageBuffer);

    return message;
}
#endif
