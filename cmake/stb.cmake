# Fetch the stb library for writing JPEG and PNG screenshots.

find_package(Stb CONFIG QUIET)

if(NOT Stb_FOUND)
	include(FetchContent)
	FetchContent_Declare(
		stb
		GIT_REPOSITORY https://github.com/nothings/stb.git
		GIT_TAG        5c205738c191bcb0abc65c4febfa9bd25ff35234
	)

	FetchContent_MakeAvailable(stb)

	set(Stb_INCLUDE_DIR ${stb_SOURCE_DIR})
endif()

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${Stb_INCLUDE_DIR})
