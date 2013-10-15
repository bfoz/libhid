cmake_minimum_required (VERSION 2.8.1)

############################################################
# Project
############################################################
project (libhid)
set (name hid)

############################################################
# Sources
############################################################
set (file_inc_common 
    include/device.h
    include/element.h
    include/enumerator.h
    include/filter.h
    include/hid.h
    ) 
set (file_src_win
    win32/device.cc
    win32/enumerator.cc
    win32/hid.cc
    )
set (file_inc_win
    win32/auto_free.h
    win32/button.h
    win32/collection.h
    win32/common.h
    win32/device.h
    win32/element.h
    win32/enumerator.h
    win32/value.h
    )
set (file_src_macos 
    macos/device.cc
    macos/element.cc
    macos/enumerator.cc
    macos/hid.cc
    macos/util.cc
    )
set (file_inc_macos
    macos/common.h
    macos/device.h
    macos/element.h
    macos/enumerator.h
    macos/util.h
    ) 
set (file_src_linux
    )
set (file_inc_linux
    ) 

set (file_inc ${file_inc_common})
if (WIN32)
    set (file_src ${file_src} ${file_src_win})
    set (file_inc ${file_inc} ${file_inc_win})
elseif (APPLE)
    set (file_src ${file_src} ${file_src_macos})
    set (file_inc ${file_inc} ${file_inc_macos})
elseif (UNIX)
    set (file_src ${file_src} ${file_src_linux})
    set (file_inc ${file_inc} ${file_inc_linux})
endif ()

if (WIN32)
    source_group ("common" FILES ${file_inc_common})
endif ()

############################################################
# Build
############################################################
set (dir_inc
    include)
set (dir_lib
    )

include_directories (${dir_inc})

link_directories (${dir_lib})

add_definitions (-DUNICODE)
set (CMAKE_MODULE_LINKER_FLAGS "-static-libstdc++")

############################################################
# Target
############################################################
set (target_static lib${name}-static)

add_library (${target_static} STATIC ${file_src} ${file_inc})

if (WIN32)
    set (file_name_static lib${name})
else ()
    set (file_name_static ${name})
endif ()

set_target_properties (${target_static} PROPERTIES
    OUTPUT_NAME ${file_name_static}
    OUTPUT_NAME_DEBUG ${file_name_static}d
    )

#target_link_libraries (${target_shared} )

############################################################
# Install
############################################################
if (WIN32)
    set (CMAKE_INSTALL_LIBDIR lib)
    set (CMAKE_INSTALL_INCLUDEDIR include)
else (WIN32)
    #include (GNUInstallDirs)
    if (NOT DEFINED CMAKE_INSTALL_LIBDIR)
        set (CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_PREFIX}/lib)
    endif ()
    if (NOT DEFINED CMAKE_INSTALL_INCLUDEDIR)
        set (CMAKE_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_PREFIX}/include)
    endif ()
endif (WIN32)

install (TARGETS ${target_static} DESTINATION ${CMAKE_INSTALL_LIBDIR})
