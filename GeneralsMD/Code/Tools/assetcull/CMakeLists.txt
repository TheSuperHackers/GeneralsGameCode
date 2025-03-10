set(ASSETCULL_SRC
    "assetcull.cpp"
)

add_executable(assetcull WIN32)

target_sources(assetcull PRIVATE ${ASSETCULL_SRC})

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_link_options(assetcull PRIVATE /subsystem:console)
endif()
