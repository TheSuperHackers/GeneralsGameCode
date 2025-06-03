# Add resources (optional)
if (MSVC_IDE)
    add_library(resources INTERFACE
        visualstudio/asciistring.natvis
        visualstudio/stllist.natvis
        visualstudio/stlvector.natvis
        visualstudio/unicodestring.natvis
    )
endif()
