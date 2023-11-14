cmake_minimum_required(VERSION 3.20)

project(GxX)

set(TARGET_NAME gx-x)

file(GLOB_RECURSE TARGET_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)

if (APPLE)
    list(APPEND TARGET_SRC
            ${CMAKE_CURRENT_SOURCE_DIR}/src/app_native_macos.mm
            )
endif (APPLE)

add_library(${TARGET_NAME}
        ${TARGET_SRC}
        )

target_link_libraries(${TARGET_NAME} PUBLIC gx)

target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if (ANDROID)
    target_include_directories(${TARGET_NAME} PRIVATE
            ${ANDROID_NDK}/sources/android/native_app_glue)
    target_link_libraries(${TARGET_NAME} PUBLIC
            native_app_glue)
endif ()

if (ANDROID)
    target_include_directories(PUBLIC
            ${ANDROID_NDK}/sources/android/native_app_glue)
elseif (IOS)
    target_link_libraries(${TARGET_NAME} PUBLIC "-framework OpenGLES  -framework Metal -framework UIKit -framework CoreGraphics -framework QuartzCore")
elseif (APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(METAL_LIBRARY Metal)
    find_library(QUARTZCORE_LIBRARY QuartzCore)
    mark_as_advanced(COCOA_LIBRARY)
    mark_as_advanced(METAL_LIBRARY)
    mark_as_advanced(QUARTZCORE_LIBRARY)
    target_link_libraries(${TARGET_NAME} PUBLIC ${COCOA_LIBRARY} ${METAL_LIBRARY} ${QUARTZCORE_LIBRARY})
endif ()

# link x11
if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    find_package(X11 REQUIRED)
    if (NOT X11_FOUND)
        message(FATAL_ERROR "Could not find X11 library!")
    endif ()
    if (NOT X11_Xcursor_FOUND)
        message(FATAL_ERROR "Could not find Xcursor library!")
    endif()
    target_link_libraries(${TARGET_NAME} PRIVATE ${X11_LIBRARIES} ${X11_Xcursor_LIB})
endif ()
