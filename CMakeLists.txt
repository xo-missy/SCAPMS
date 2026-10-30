cmake_minimum_required(VERSION 3.16)
project(SCAPMS VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# Works with Qt 6 or Qt 5 (5.15+)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# ---- Qt-free core: utils + model + controller ----
file(GLOB CORE_SOURCES src/utils/*.cpp src/model/*.cpp src/controller/*.cpp)
add_library(scapms_core STATIC ${CORE_SOURCES})

# ---- GUI application ----
file(GLOB GUI_SOURCES src/gui/*.cpp)
add_executable(SCAPMS src/main.cpp ${GUI_SOURCES})
target_link_libraries(SCAPMS PRIVATE scapms_core Qt${QT_VERSION_MAJOR}::Widgets)

# ---- console tests for the core (no GUI needed) ----
add_executable(test_core tests/test_core.cpp)
target_link_libraries(test_core PRIVATE scapms_core)
enable_testing()
add_test(NAME core COMMAND test_core)

# sample data next to the executable so "data/sample_clinic.csv" is found automatically
file(COPY ${CMAKE_SOURCE_DIR}/data DESTINATION ${CMAKE_BINARY_DIR})
