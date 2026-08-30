# Needed for packet stuff
include(TestBigEndian)
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

if(ANDROID)
	add_compile_definitions(SYS_ANDROID)
endif()

if((CMAKE_C_COMPILER_ID STREQUAL "Clang" OR CMAKE_C_COMPILER_ID STREQUAL "GNU"))
	set(CMAKE_C_FLAGS_DEBUG "-g")
	set(CMAKE_C_FLAGS_RELEASE "-O3")
	message("Clang/GCC flags applied")
endif()

set(SOURCES
	"Auth.c"
	"Config.c"
	"cJSON.c"
	"UTF8.c"
	"CMath.c"
	"DyList.c"
	"Log.c"
	"Lib.c"
	"Server.c"
	"Vote.c"
	"Maps.c"
	"Packet.c"
	"Player.c"
	"Palette.c"
	"Zone.c"

	"states/Lobby.c"
	"states/Mapvote.c"
	"states/CharSelect.c"
	"states/Game.c"
	"states/Results.c"
	"io/Time.c"

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
	"entities/WDLatern.c"
	"entities/MJLava.c"
	"entities/MJAss.c"
	"entities/MJJudger.c"

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
	"maps/WoodDream.c"
	"maps/Marijuna.c"
)

if(ANDROID)
	add_library(DisasterServer SHARED "Android.c" ${SOURCES})

	set(THREADS_PREFER_PTHREAD_FLAG ON)
	find_package(Threads REQUIRED)
	target_include_directories(DisasterServer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
	target_link_libraries(DisasterServer PRIVATE log enet m Threads::Threads)
else()
	if(DYLIB) # Build .dll or .so
		add_library(DisasterServer SHARED ${SOURCES})

		if (WIN32)
			target_link_libraries(DisasterServer PRIVATE enet ws2_32 winmm dbghelp)
		else() # UNIX
			set(THREADS_PREFER_PTHREAD_FLAG ON)
			find_package(Threads REQUIRED)
			target_link_libraries(DisasterServer PRIVATE enet m Threads::Threads)
		endif()

		target_include_directories(DisasterServer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

	else() # Build executable
		if(BUILD_UI)
			set(UI_SOURCES
				"ui/Resources.c"
				"ui/Main.c"
				"ui/Image.c"
				"ui/Label.c"
				"ui/Button.c"
				"ui/TButton.c"
				"ui/MapList.c"
			  	"ui/Presets.c"
				"ui/MapListPreset.c"
				"ui/PingPreset.c"
				"ui/PlayerList.c"
			)

			if (WIN32)
				add_executable(DisasterServer
					${SOURCES}
					${UI_SOURCES}
				)

				target_link_libraries(DisasterServer PRIVATE
						ws2_32
						enet
						winmm
						dbghelp
						SDL3-static
						SDL3_image::SDL3_image
				)

				target_include_directories(DisasterServer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

				target_sources(DisasterServer PRIVATE "${CMAKE_SOURCE_DIR}/resources/icon.rc")

			else() # UNIX
				add_executable(DisasterServer
					${SOURCES}
					${UI_SOURCES}
				)

				set(THREADS_PREFER_PTHREAD_FLAG ON)
				find_package(Threads REQUIRED)
				target_link_libraries(DisasterServer PRIVATE enet m Threads::Threads
						SDL3::SDL3-static
						SDL3_image::SDL3_image
				)
			endif()

			target_include_directories(DisasterServer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

		else()
			add_executable(DisasterServer
				${SOURCES}
				Main.c
			)

			if (WIN32)
				target_link_libraries(DisasterServer PRIVATE enet ws2_32 winmm dbghelp)
			else() # UNIX
				set(THREADS_PREFER_PTHREAD_FLAG ON)
				find_package(Threads REQUIRED)
				target_link_libraries(DisasterServer PRIVATE enet m Threads::Threads)
			endif()

			target_include_directories(DisasterServer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

		endif()
	endif()
endif()

if(MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

unset(DYLIB CACHE)
unset(BUILD_UI CACHE)
