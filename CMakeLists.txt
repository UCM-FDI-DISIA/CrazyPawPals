cmake_minimum_required(VERSION 3.6)


project(CrazyPawPals C CXX)
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_definitions(-D_CRTDBG_MAP_ALLOC)
    endif()

    set(USER_MSVC ((MSVC OR MSVC_IDE) AND (NOT (CMAKE_CXX_COMPILER_ID STREQUAL "Clang"))))
    if (${USER_MSVC})
        message(STATUS "MSVC detected")
        add_compile_options(
            /W4
            /Zi

            /nologo
            #-D DBG_COLLISIONS=true
        )
    else()
        message(STATUS "GCC/Clang detected")
        add_compile_options(
            -Wall
            -Wextra
            -Wpedantic
            # -Werror

            -Wno-c++98-compat
            -Wno-c++98-compat-pedantic
            -Wno-reserved-identifier

            -Wno-shadow-field
            -Wno-shadow-field-in-constructor

            -Wno-c++20-extensions
        )
    endif()

    set(
        CrazyPawPals_PROJECT_DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}/project
    )
    set(
        CrazyPawPals_SRC_DIRECTORY
        ${CrazyPawPals_PROJECT_DIRECTORY}/src
    )

    set(
        CrazyPawPals_LIB_DIRECTORY
        ${CrazyPawPals_PROJECT_DIRECTORY}/lib
    )

    file(
        GLOB_RECURSE
        CrazyPawPals_SOURCES
        CMAKE_CONFIGURE_DEPENDS
        ${CrazyPawPals_SRC_DIRECTORY}/*.cpp
    )

    set(
        SDL2_IMAGE_DIRECTORY
        ${CrazyPawPals_LIB_DIRECTORY}/SDL2_image-2.8.4
    )
    set(
        SDL2_MIXER_DIRECTORY
        ${CrazyPawPals_LIB_DIRECTORY}/SDL2_mixer-2.8.0
    )
    set(
        SDL2_NET_DIRECTORY
        ${CrazyPawPals_LIB_DIRECTORY}/SDL2_net-2.2.0
    )
    set(
        SDL2_TTF_DIRECTORY
        ${CrazyPawPals_LIB_DIRECTORY}/SDL2_ttf-2.24.0
    )
    set(
        SDL2_DIRECTORY
        ${CrazyPawPals_LIB_DIRECTORY}/SDL2-2.30.11
    )


    if ((CMAKE_BUILD_TYPE STREQUAL "Release"))
        if (${USER_MSVC})
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
        else()
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mwindows -Wl,--subsystem,windows")
        endif()
    endif()
    add_executable(
        ${PROJECT_NAME}
        # WIN32
        ${CrazyPawPals_SRC_DIRECTORY}/main.cpp
        ${CrazyPawPals_SOURCES}
    )
    if ((CMAKE_BUILD_TYPE STREQUAL "Release"))
        set_target_properties(
            ${PROJECT_NAME}
            PROPERTIES
            WIN32_EXECUTABLE TRUE
        )
    endif()
    

    if (NOT $ENV{DBG_NETWORK})
        set(DBG_NETWORK false)
        set(DBG_NETWORK_HOST false)
        set(DBG_NETWORK_CLIENT false)
    else()
        set(DBG_NETWORK $ENV{DBG_NETWORK})
        set(DBG_NETWORK_HOST $ENV{DBG_NETWORK_HOST})
        set(DBG_NETWORK_CLIENT $ENV{DBG_NETWORK_CLIENT})
    endif()
    target_compile_options(
        ${PROJECT_NAME}
        PRIVATE
        -D RIGIDBODY_COMPONENT_DEGEN_RESTITUTION_OFF
        -DDBG_COLLISIONS=false
        #-DGENERATE_LOG
        -DDBG_NETWORK=${DBG_NETWORK}
        -DDBG_NETWORK_HOST=${DBG_NETWORK_HOST}
        -DDBG_NETWORK_CLIENT=${DBG_NETWORK_CLIENT}
        
        -DDBG_NETWORK_HOST_IP="localhost"
        -DDBG_NETWORK_HOST_PORT=1234
    )
    
    target_include_directories(
        ${PROJECT_NAME}
        PUBLIC
        ${CrazyPawPals_SRC_DIRECTORY}
    )
    target_include_directories(
        ${PROJECT_NAME}
        SYSTEM
        PUBLIC
        ${SDL2_DIRECTORY}/include
        ${SDL2_IMAGE_DIRECTORY}/include
        ${SDL2_MIXER_DIRECTORY}/include
        ${SDL2_NET_DIRECTORY}/include
        ${SDL2_TTF_DIRECTORY}/include
    )

    target_link_directories(
        ${PROJECT_NAME}
        PRIVATE
        ${SDL2_DIRECTORY}/lib/x64
        ${SDL2_IMAGE_DIRECTORY}/lib/x64
        ${SDL2_MIXER_DIRECTORY}/lib/x64
        ${SDL2_NET_DIRECTORY}/lib/x64
        ${SDL2_TTF_DIRECTORY}/lib/x64
    )
    target_link_libraries(
        ${PROJECT_NAME}
        PRIVATE
        SDL2
        SDL2main
        SDL2_image
        SDL2_ttf
        SDL2_net
        SDL2_mixer
    )
    
    set(
        CrazyPawPals_EXE_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}/bin
    )
    set_target_properties(
        ${PROJECT_NAME}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CrazyPawPals_EXE_DIRECTORY}/$<0:>
        ARCHIVE_OUTPUT_DIRECTORY ${CrazyPawPals_EXE_DIRECTORY}/$<0:>
        LIBRARY_OUTPUT_DIRECTORY ${CrazyPawPals_EXE_DIRECTORY}/$<0:>
    )
    
    add_custom_target(
        copy_resources_bin
        COMMAND ${CMAKE_COMMAND}
            -D V_PROJECT_DIRECTORY=${CrazyPawPals_PROJECT_DIRECTORY} -D V_EXECUTABLE_DIRECTORY=${CrazyPawPals_EXE_DIRECTORY}
            -P ${CMAKE_CURRENT_LIST_DIR}/copy-resources.cmake
    )
    add_dependencies(CrazyPawPals copy_resources_bin)

