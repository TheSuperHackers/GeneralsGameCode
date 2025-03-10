add_library(profile STATIC)

target_include_directories(profile INTERFACE
    .
)

target_include_directories(profile PRIVATE
    ../../Include
)

target_sources(profile PRIVATE
    profile_funclevel.cpp
	profile_funclevel.h
	profile_highlevel.cpp
	profile_highlevel.h
	profile_result.cpp
	profile_result.h
	_pch.cpp
	_pch.h
	internal.h
	internal_cmd.h
	internal_funclevel.h
	internal_highlevel.h
	internal_result.h
	profile.cpp
	profile.h
	profile_cmd.cpp
	profile_doc.h
)

set_target_properties(profile PROPERTIES OUTPUT_NAME $<IF:$<CONFIG:Debug>,profiledebug,profile>)
