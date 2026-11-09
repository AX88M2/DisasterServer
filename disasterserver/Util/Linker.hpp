#pragma once

#if _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif
#if WIN32
using Handle = HMODULE;
#else
using Handle = void*;
#endif

namespace DisasterServer
{
    class Linker {
        Handle handle = nullptr;
    public:
        Linker(const std::string &name);
        ~Linker();

        static std::string getError();

        template<typename T>
        T getSymbol(std::string symbol) {
            return reinterpret_cast<T>(getSymbol(std::move(symbol)));
        }
    private:
        void *getSymbol(const std::string &symbol);
#if WIN32
        static std::string getLastErrorAsString();
#endif
    };
}
