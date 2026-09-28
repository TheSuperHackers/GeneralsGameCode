FetchContent_Declare(
    dx8
    GIT_REPOSITORY https://github.com/TheSuperHackers/min-dx8-sdk.git
    GIT_TAG        7bddff8c01f5fb931c3cb73d4aa8e66d303d97bc
)

FetchContent_MakeAvailable(dx8)

# When NO_D3DX is enabled, remove d3dx8d from d3d8lib link libraries
if(MINGW AND DEFINED MINGW_NO_D3DX AND MINGW_NO_D3DX)
    # Get current link libraries from d3d8lib
    get_target_property(DX8_LINK_LIBS d3d8lib INTERFACE_LINK_LIBRARIES)
    
    if(DX8_LINK_LIBS)
        # Remove d3dx8d from the list
        list(REMOVE_ITEM DX8_LINK_LIBS d3dx8d)
        
        # Set the updated list back
        set_target_properties(d3d8lib PROPERTIES
            INTERFACE_LINK_LIBRARIES "${DX8_LINK_LIBS}"
        )
        
        message(STATUS "Removed d3dx8d from d3d8lib link libraries (NO_D3DX)")
    endif()
endif()
