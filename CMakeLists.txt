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
)

target_include_directories(app PRIVATE
    include
    include/glm
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
