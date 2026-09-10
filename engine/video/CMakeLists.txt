cmake_minimum_required(VERSION 3.28)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(generals_video LANGUAGES CXX)
endif()

add_library(generals_video STATIC)

target_sources(generals_video
    PUBLIC
        FILE_SET CXX_MODULES
        BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}"
        FILES
            frame/Frame.cppm
            capture/AVIWriter.cppm
            capture/ImageWriter.cppm
            decode/Decoder.cppm
            presentation/Presenter.cppm
            playback/Playback.cppm
)

if(WIN32)
    target_sources(generals_video
        PUBLIC
            FILE_SET CXX_MODULES
            BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}"
            FILES
                decode/FFmpegDecoder.cppm
                playback/FFmpegPlayer.cppm
                playback/Runtime.cppm
    )
    target_include_directories(generals_video PUBLIC ${FFMPEG_INCLUDE_DIRS})
    target_link_directories(generals_video PUBLIC ${FFMPEG_LIBRARY_DIRS})
    target_link_libraries(generals_video PUBLIC ${FFMPEG_LIBRARIES})
endif()

target_compile_features(generals_video PUBLIC cxx_std_23)

if(TARGET stb)
    target_link_libraries(generals_video PRIVATE stb)
else()
    find_path(STB_INCLUDE_DIR stb_image_write.h REQUIRED)
    target_include_directories(generals_video PRIVATE "${STB_INCLUDE_DIR}")
endif()

option(RTS_BUILD_VIDEO_TESTS "Build video unit tests." OFF)

if(RTS_BUILD_VIDEO_TESTS)
    include(CTest)
    find_package(Boost CONFIG REQUIRED)

    add_executable(generals_video_avi_writer_test)
    target_sources(generals_video_avi_writer_test PRIVATE FILE_SET CXX_MODULES
        BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}" FILES capture/AVIWriter.test.cppm)
        target_compile_features(generals_video_avi_writer_test PRIVATE cxx_std_23)
    target_include_directories(generals_video_avi_writer_test PRIVATE ${Boost_INCLUDE_DIRS})
    target_link_libraries(generals_video_avi_writer_test PRIVATE generals_video)
    add_test(NAME generals_video_avi_writer_test COMMAND generals_video_avi_writer_test)

    add_executable(generals_video_image_writer_test)
    target_sources(generals_video_image_writer_test PRIVATE FILE_SET CXX_MODULES
        BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}" FILES capture/ImageWriter.test.cppm)
        target_compile_features(generals_video_image_writer_test PRIVATE cxx_std_23)
    target_include_directories(generals_video_image_writer_test PRIVATE ${Boost_INCLUDE_DIRS})
    target_link_libraries(generals_video_image_writer_test PRIVATE generals_video)
    if(TARGET stb)
        target_link_libraries(generals_video_image_writer_test PRIVATE stb)
    else()
        target_include_directories(generals_video_image_writer_test PRIVATE "${STB_INCLUDE_DIR}")
    endif()
    add_test(NAME generals_video_image_writer_test COMMAND generals_video_image_writer_test)

    add_executable(generals_video_playback_test)
    target_sources(generals_video_playback_test
        PRIVATE
            FILE_SET CXX_MODULES
            BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}"
            FILES
                playback/Playback.test.cppm
    )
        target_compile_features(generals_video_playback_test PRIVATE cxx_std_23)
    target_include_directories(generals_video_playback_test PRIVATE ${Boost_INCLUDE_DIRS})
    target_link_libraries(generals_video_playback_test PRIVATE generals_video)
    add_test(NAME generals_video_playback_test COMMAND generals_video_playback_test)

    if(WIN32)
        add_executable(generals_video_runtime_test)
        target_sources(generals_video_runtime_test
            PRIVATE
                FILE_SET CXX_MODULES
                BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}"
                FILES
                    playback/Runtime.test.cppm
        )
        target_compile_features(generals_video_runtime_test PRIVATE cxx_std_23)
        target_include_directories(generals_video_runtime_test PRIVATE ${Boost_INCLUDE_DIRS})
        target_link_libraries(generals_video_runtime_test PRIVATE generals_video)
        add_test(NAME generals_video_runtime_test COMMAND generals_video_runtime_test)
        set_tests_properties(generals_video_runtime_test PROPERTIES
            ENVIRONMENT "PATH=${FFMPEG_RUNTIME_DIRECTORY};${FFMPEG_DEBUG_RUNTIME_DIRECTORY};$ENV{PATH}"
        )
    endif()

    add_executable(generals_video_presentation_test)
    target_sources(generals_video_presentation_test
        PRIVATE
            FILE_SET CXX_MODULES
            BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}"
            FILES
                presentation/Presenter.test.cppm
    )
        target_compile_features(generals_video_presentation_test PRIVATE cxx_std_23)
    target_include_directories(generals_video_presentation_test PRIVATE ${Boost_INCLUDE_DIRS})
    target_link_libraries(generals_video_presentation_test PRIVATE generals_video)
    add_test(NAME generals_video_presentation_test COMMAND generals_video_presentation_test)
endif()
