set(COMPILER_OPTIONS
        $<$<CXX_COMPILER_ID:MSVC>:
            /utf-8
            /MP
            /W4
            /wd4100 # unreferenced formal parameter
            /wd4457 # declaration of 'var' hides function parameter
            /wd4458 # declaration of 'var' hides class member
            /wd4459 # declaration of 'var' hides global declaration
            /wd4101 # 'var': unreferenced local variable
            /wd4388 # 'token' : signed/unsigned mismatch
            /wd4018 # '>': signed/unsigned mismatch
        >
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:
            -Wall
            -Wextra
            -Wpedantic
            -finput-charset=UTF-8
            -fexec-charset=UTF-8
        >
        $<$<CXX_COMPILER_ID:GNU>:
            -Wall
            -Wextra
            -Wpedantic
            -finput-charset=UTF-8
            -fexec-charset=UTF-8
        >
)

set(DEFINES
        $<$<CONFIG:Debug>:SERVER_DEBUG>
)

file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

if(ANDROID)
    add_library(DisasterServerCXX SHARED "Android.cpp" ${SOURCES})

    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    target_include_directories(DisasterServerCXX PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_options(DisasterServerCXX PRIVATE ${COMPILER_OPTIONS})
    target_compile_definitions(DisasterServerCXX PRIVATE ${DEFINES})
    target_link_libraries(DisasterServerCXX PRIVATE cJSON log enet m Threads::Threads)
else()
    if(DYLIB) # Build .dll or .so
        add_library(DisasterServerCXX SHARED ${SOURCES})

        if (WIN32)
            target_link_libraries(DisasterServerCXX PRIVATE cJSON enet ws2_32 winmm dbghelp)
        else() # UNIX
            set(THREADS_PREFER_PTHREAD_FLAG ON)
            find_package(Threads REQUIRED)
            target_link_libraries(DisasterServerCXX PRIVATE cJSON enet m Threads::Threads)
        endif()

        target_compile_options(DisasterServerCXX PRIVATE ${COMPILER_OPTIONS})
        target_compile_definitions(DisasterServerCXX PRIVATE ${DEFINES})
        target_include_directories(DisasterServerCXX PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

    else() # Build executable
        if(BUILD_UI)
           # set(UI_SOURCES
           #         "ui/Resources.c"
           #         "ui/Main.c"
           #         "ui/Image.c"
           #         "ui/Label.c"
           #         "ui/Button.c"
           #         "ui/TButton.c"
           #         "ui/MapList.c"
           #         "ui/Presets.c"
           #         "ui/MapListPreset.c"
           #         "ui/PingPreset.c"
           #         "ui/PlayerList.c"
           # )

           # if (WIN32)
           #     add_executable(DisasterServer
           #             ${SOURCES}
           #             ${UI_SOURCES}
           #     )

           #     target_link_libraries(DisasterServer PRIVATE
           #             ws2_32
           #             enet
           #             winmm
           #             dbghelp
           #             SDL3-static
           #             SDL3_image::SDL3_image
           #     )

           #     target_include_directories(DisasterServer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

           #     target_sources(DisasterServer PRIVATE "${CMAKE_SOURCE_DIR}/resources/icon.rc")

           # else() # UNIX
           #     add_executable(DisasterServer
           #             ${SOURCES}
           #             ${UI_SOURCES}
           #     )

           #     set(THREADS_PREFER_PTHREAD_FLAG ON)
           #     find_package(Threads REQUIRED)
           #     target_link_libraries(DisasterServer PRIVATE enet m Threads::Threads
           #             SDL3::SDL3-static
           #             SDL3_image::SDL3_image
           #     )
           # endif()

           # target_include_directories(DisasterServer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

        else()
            add_executable(DisasterServerCXX
                    ${SOURCES}
                    Main.cpp
            )

            if (WIN32)
                target_link_libraries(DisasterServerCXX PRIVATE cJSON enet ws2_32 winmm dbghelp)
            else() # UNIX
                set(THREADS_PREFER_PTHREAD_FLAG ON)
                find_package(Threads REQUIRED)
                target_link_libraries(DisasterServerCXX PRIVATE cJSON enet m Threads::Threads)
            endif()

            target_compile_options(DisasterServerCXX PRIVATE ${COMPILER_OPTIONS})
            target_compile_definitions(DisasterServerCXX PRIVATE ${DEFINES})
            target_include_directories(DisasterServerCXX PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

        endif()
    endif()
endif()


if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

unset(DYLIB CACHE)
unset(BUILD_UI CACHE)