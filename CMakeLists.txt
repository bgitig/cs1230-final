cmake_minimum_required(VERSION 3.16)

# Sets project name
project(projects_realtime LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Sets C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Specifies required Qt components
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Gui)
find_package(Qt6 REQUIRED COMPONENTS OpenGL)
find_package(Qt6 REQUIRED COMPONENTS OpenGLWidgets)
find_package(Qt6 REQUIRED COMPONENTS Xml)

# Allows you to include files from within those directories, without prefixing their filepaths
include_directories(src)

# Specifies .cpp and .h files to be passed to the compiler
add_executable(${PROJECT_NAME}
    src/main.cpp

    src/realtime.cpp
    src/mainwindow.cpp
    src/settings.cpp
    src/utils/scenefilereader.cpp
    src/utils/sceneparser.cpp

    src/mainwindow.h
    src/realtime.h
    src/settings.h
    src/utils/scenedata.h
    src/utils/scenefilereader.h
    src/utils/sceneparser.h
    src/utils/shaderloader.h
    src/utils/aspectratiowidget/aspectratiowidget.hpp
    src/utils/camera.h src/utils/camera.cpp
    src/utils/shape.h src/utils/shape.cpp
    src/utils/sphere.h src/utils/sphere.cpp
    src/utils/cube.h src/utils/cube.cpp
    src/utils/cone.h src/utils/cone.cpp
    src/utils/cylinder.h src/utils/cylinder.cpp
    src/terrain.h src/terrain.cpp
    src/mouse.h src/mouse.cpp
    src/skybox.h src/skybox.cpp
    src/sky.qrc

    src/objloader.h src/objloader.cpp

    src/turtle.h src/turtle.cpp
    src/treebase.h src/treebase.cpp
    src/treemanager.h src/treemanager.cpp
    src/bumpmapping.h src/bumpmapping.cpp
    src/rockgenerator.h src/rockgenerator.cpp
    src/flag.h src/flag.cpp
    src/particles.h src/particles.cpp


)

# GLM: this creates its library and allows you to `#include "glm/..."`
add_subdirectory(glm)

# GLEW: this creates its library and allows you to `#include "GL/glew.h"`
add_library(StaticGLEW STATIC glew/src/glew.c
    src/utils/camera.h src/utils/camera.cpp
)
include_directories(${PROJECT_NAME} PRIVATE glew/include)

# Specifies libraries to be linked (Qt components, glew, etc)
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt::Core
    Qt::Gui
    Qt::OpenGL
    Qt::OpenGLWidgets
    Qt::Xml
    StaticGLEW
)

# Specifies other files
qt6_add_resources(${PROJECT_NAME} "Resources"
    PREFIX
        "/"
    FILES
        resources/shaders/default.frag
        resources/shaders/default.vert
        resources/shaders/shadows.frag
        resources/shaders/shadows.vert
        resources/shaders/terrain.frag
        resources/shaders/terrain.vert
        resources/shaders/bump.frag
        resources/shaders/bump.vert

        resources/shaders/tree_wind.frag
        resources/shaders/tree_wind.vert

resources/shaders/bloom.vert
resources/shaders/bloom.frag
resources/shaders/godrays.vert
resources/shaders/godrays.frag
resources/shaders/fog.vert
resources/shaders/fog.frag
)

qt6_add_resources(${PROJECT_NAME} "svg"
    PREFIX
        "/"
    FILES
        svg/rock.svg
svg/tree.svg
svg/cam.svg

svg/light.svg

svg/rake.svg
)

qt_add_resources(sky.qrc)
qt_add_resources(base.qrc)

# GLEW: this provides support for Windows (including 64-bit)
if (WIN32)
  add_compile_definitions(GLEW_STATIC)
  target_link_libraries(${PROJECT_NAME} PRIVATE
    opengl32
    glu32
  )
endif()

# Set this flag to silence warnings on Windows
if (MSVC OR MSYS OR MINGW)
  set(CMAKE_CXX_FLAGS "-Wno-volatile")
endif()
# Set this flag to silence warnings on MacOS
if (APPLE)
  set(CMAKE_CXX_FLAGS "-Wno-deprecated-volatile")
endif()
