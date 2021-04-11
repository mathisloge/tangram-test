cmake_minimum_required(VERSION 3.18)
project(tangram-test)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/Modules/")

include(Modules/CopyDllsForDebug.cmake)

find_package(imgui CONFIG REQUIRED)
find_package(Corrade REQUIRED Utility Main)
find_package(Magnum REQUIRED GL MeshTools Sdl2Application)
find_package(MagnumIntegration REQUIRED ImGui)
find_package(tangram CONFIG REQUIRED)
find_path(MAPBOX_VARIANT_INCLUDE_DIRS "mapbox/optional.hpp")


add_executable(tangram-test main.cpp)

target_include_directories(tangram-test PRIVATE 
    ${MAPBOX_VARIANT_INCLUDE_DIRS}
)
target_link_libraries(tangram-test PRIVATE 
    imgui::imgui
    Magnum::Application
    Magnum::GL
    Magnum::Magnum
    MagnumIntegration::ImGui
    tangram::tangram
)

set(DIRS 
    "D:/dev/vcpkg/installed/x64-windows/bin" 
    "D:/dev/vcpkg/installed/x64-windows/debug/bin"
    "D:/dev/install/bin"
)
copy_dlls_for_debug(TARGETS tangram-test LIBS "" DIRS ${DIRS})
