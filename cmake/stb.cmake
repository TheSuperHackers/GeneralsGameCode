# Fetch the stb library for writing JPEG and PNG screenshots.

# vcpkg provides stb through a find module (FindStb.cmake), not a config package,
# so the search must not be restricted to CONFIG mode.
find_package(Stb QUIET)

if(NOT Stb_FOUND)
	include(FetchContent)
	FetchContent_Declare(
		stb
		GIT_REPOSITORY https://github.com/TheSuperHackers/stb.git
		GIT_TAG        5c205738c191bcb0abc65c4febfa9bd25ff35234
	)

	FetchContent_MakeAvailable(stb)

	set(Stb_INCLUDE_DIR ${stb_SOURCE_DIR})
endif()

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${Stb_INCLUDE_DIR})
