cmake_minimum_required(VERSION 3.8)

option(DYLIB "Builds dynamic library" OFF)
option(BUILD_UI "Builds UI using SDL" ON)

# Needed for packet stuff
include (TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)

if(IS_BIG_ENDIAN)
	add_compile_definitions(SYS_BIG_ENDIAN)
endif()

if(DYLIB)
	add_compile_definitions(SYS_DYLIB)
endif()

if(BUILD_UI)
	add_compile_definitions(SYS_USE_SDL2)
endif()

if((CMAKE_C_COMPILER_ID STREQUAL "Clang" OR CMAKE_C_COMPILER_ID STREQUAL "GNU"))
	set(CMAKE_C_FLAGS_DEBUG "-g")
	set(CMAKE_C_FLAGS_RELEASE "-O3")
	message("Clang/GCC flags applied")
endif()

set(SOURCES
	"Log.c"
	"Lib.c"
	"Config.c"
	"DyList.c"
	"cJSON.c"
	"CMath.c"
	"Server.c"
	"Vote.c"
	"Lobby.c"
	"Maps.c"
	"Mapvote.c"
	"CharSelect.c"
	"Player.c"
	"Game.c"

	"io/TcpListener.c"
	"io/Packet.c"
	"io/Time.c"
	"io/UdpListener.c"  

	"entities/Ring.c"
	"entities/CreamRing.c"
	"entities/TailsProjectile.c"
	"entities/EggmanTracker.c"
	"entities/BlackRing.c"
	"entities/ExellerClone.c"
	"entities/RMZSlug.c"
	"entities/RMZShard.c"
	"entities/Act9Wall.c" 
	"entities/NotPerfect.c"
	"entities/PFLift.c" 
	"entities/HDDoor.c" 
	"entities/SpkieController.c"
	"entities/LCEye.c"
	"entities/LCChain.c"
	"entities/NAPSnowball.c"
	"entities/NAPIce.c"
	"entities/HillThunder.c"
	"entities/VVLava.c" 
	"entities/YouCantRun.c"
	"entities/TCAcid.c"
	"entities/KAFSpeedBox.c"
	"entities/TailsDoll.c"
	"entities/DTBall.c" 
	"entities/DTStalactits.c" 
	"entities/Dummy.c"

	"maps/RavineMist.c"
	"maps/Act9.c"
	"maps/NotPerfect.c" 
	"maps/PricelessFreedom.c"
	"maps/HauntingDream.c"
    "maps/DotDotDot.c"
	"maps/LimpCity.c"
	"maps/NastyParadise.c"
	"maps/Hill.c" 
	"maps/VolcanoValley.c"
	"maps/YouCantRun.c"
	"maps/TortureCave.c" 
	"maps/KindAndFair.c"
	"maps/DarkTower.c"
	"maps/FartZone.c" 
	"maps/MajinForest.c" 
	"maps/HideAndSeek.c"
)

if(DYLIB) # Build .dll or .so
	add_library(DisasterServer SHARED ${SOURCES})
	
	if (WIN32)
		target_link_libraries(DisasterServer PRIVATE ws2_32)
	else() # UNIX
		set(THREADS_PREFER_PTHREAD_FLAG ON)
		find_package(Threads REQUIRED)
		target_link_libraries(DisasterServer PRIVATE m Threads::Threads)
	endif()

else() # Build executable
	add_executable(DisasterServer Main.c ${SOURCES})

	if(BUILD_UI)
		if (WIN32)
			target_link_libraries(DisasterServer PRIVATE ws2_32 ${SDL2_INCLUDE_DIRS})
		else() # UNIX
			set(THREADS_PREFER_PTHREAD_FLAG ON)
			find_package(Threads REQUIRED)
			target_link_libraries(DisasterServer PRIVATE m Threads::Threads ${SDL2_INCLUDE_DIRS})
		endif()

	else()
		if (WIN32)
			target_link_libraries(DisasterServer PRIVATE ws2_32)
		else() # UNIX
			set(THREADS_PREFER_PTHREAD_FLAG ON)
			find_package(Threads REQUIRED)
			target_link_libraries(DisasterServer PRIVATE m Threads::Threads)
		endif()

	endif()
endif()

if(MSVC) # Turn off retarded microsofts shit
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

unset(DYLIB CACHE)
unset(BUILD_UI CACHE)