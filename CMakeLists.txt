cmake_minimum_required(VERSION 3.10)
project(LearnOpenGL LANGUAGES C CXX)

add_definitions(-DPROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)

find_package(glfw3 CONFIG QUIET)
if(NOT glfw3_FOUND)
    find_library(GLFW_LIBRARY NAMES glfw3 glfw PATHS "${CMAKE_SOURCE_DIR}/lib")
    if(NOT GLFW_LIBRARY)
        message(FATAL_ERROR "GLFW not found. On macOS, install it with: brew install glfw")
    endif()
endif()

find_library(ASSIMP_LIBRARY NAMES assimp libassimp.dll.a PATHS "${CMAKE_SOURCE_DIR}/lib")

if(NOT ASSIMP_LIBRARY)
    message(FATAL_ERROR "Assimp not found")
endif()


add_executable(app
    src/HelloWorld.cpp
    src/glad.c
    src/Shader.cpp
    src/stb_image.cpp
    src/Camera.cpp
    src/Model.cpp
    src/Mesh.cpp
    external/imgui-master/imgui.cpp
    external/imgui-master/imgui_draw.cpp
    external/imgui-master/imgui_tables.cpp
    external/imgui-master/imgui_widgets.cpp
    external/imgui-master/backends/imgui_impl_glfw.cpp
    external/imgui-master/backends/imgui_impl_opengl3.cpp
)

target_include_directories(app PRIVATE
    include
    include/glm
    external/imgui-master
    external/imgui-master/backends 
)

target_compile_definitions(app PRIVATE
    PROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)


target_link_libraries(app PRIVATE ${ASSIMP_LIBRARY})

if(glfw3_FOUND)
    target_link_libraries(app PRIVATE glfw OpenGL::GL)
else()
    target_link_libraries(app PRIVATE ${GLFW_LIBRARY} OpenGL::GL)
endif()

if(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    get_filename_component(MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)

    foreach(runtime_dll libgcc_s_seh-1.dll libstdc++-6.dll libwinpthread-1.dll)
        find_file(${runtime_dll}_PATH
            NAMES ${runtime_dll}
            PATHS "${MINGW_BIN_DIR}"
            NO_DEFAULT_PATH
        )

        if(${runtime_dll}_PATH)
            add_custom_command(TARGET app POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${${runtime_dll}_PATH}"
                    "$<TARGET_FILE_DIR:app>"
            )
        endif()
    endforeach()

    find_file(ASSIMP_RUNTIME_DLL
        NAMES libassimp-6.dll
        PATHS "${CMAKE_SOURCE_DIR}/lib" "${MINGW_BIN_DIR}"
        NO_DEFAULT_PATH
    )

    if(ASSIMP_RUNTIME_DLL)
        add_custom_command(TARGET app POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${ASSIMP_RUNTIME_DLL}"
                "$<TARGET_FILE_DIR:app>"
        )
    else()
        message(WARNING
            "libassimp-6.dll was not found. app.exe links successfully, but it will not start until "
            "a matching MinGW/UCRT libassimp-6.dll is placed in ${CMAKE_SOURCE_DIR}/lib or next to app.exe."
        )
    endif()
endif()
