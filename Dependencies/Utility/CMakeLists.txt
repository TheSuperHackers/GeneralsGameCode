set(UTILITY_SRC
    Allocator/SystemAllocator.h
    Utility/aligned_pod.h
    Utility/array_adapter.h
    Utility/atl_compat.h
    Utility/compat.h
    Utility/comsupp_compat.h
    Utility/endian_compat.h
    Utility/fstream_adapter.h
    Utility/hash_map_adapter.h
    Utility/interlocked_adapter.h
    Utility/intrin_compat.h
    Utility/iostream_adapter.h
    Utility/lazy_static.h
    Utility/mem_compat.h
    Utility/pseh_compat.h
    Utility/sstream_adapter.h
    Utility/stdio_adapter.h
    Utility/STLUtils.h
    Utility/string_compat.h
    Utility/stringex.h
    Utility/tchar_compat.h
    Utility/thread_compat.h
    Utility/time_compat.h
    Utility/utility_adapter.h
    Utility/wchar_compat.h
)

add_library(deps_utility INTERFACE)
target_include_directories(deps_utility INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(deps_utility PRIVATE ${UTILITY_SRC})

target_link_libraries(deps_utility INTERFACE
    deps_precompiled # Gives force included headers. Link early.
)
