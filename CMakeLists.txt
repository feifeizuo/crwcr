cmake_minimum_required(VERSION 3.8.0)

project(CRWCR)

option(USE_CUDA "use cuda" OFF)

if(USE_CUDA)
    enable_language("CUDA")
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)
# Add a compiler flag
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Make this a GUI application on Windows
if(WIN32)
  set(CMAKE_WIN32_EXECUTABLE ON)
endif()

# Find Qt's  library
find_package(Qt5Widgets CONFIG REQUIRED)
find_package(Qt5Core QUIET REQUIRED)
find_package(Qt5Gui QUIET REQUIRED)

include(FindOpenMP)
if(OPENMP_FOUND)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
else(OPENMP_FOUND)
  message("ERROR: OpenMP could not be found.")
endif(OPENMP_FOUND)

qt5_add_resources(QRCS src/resource.qrc)

set(SOURCE_FILES
    src/main.cpp
    src/mainwindow.cpp
    src/crwcralgorithm.cpp
    src/imagecanvas.cpp
    src/toolpanel.cpp
	src/toolForm.ui
    src/pointlistgeometry.cpp
    src/twolabelseed.cpp
)

set(HEADER_FILES
    src/mainwindow.h
    src/crwcralgorithm.h
    src/singleton.h
    src/imagecanvas.h
    src/toolpanel.h
    src/pointlistgeometry.h
    src/twolabelseed.h
)

if(USE_CUDA)
    set(SOLVER_SOURCE_FILES
        src/crwcrgpusolver.h
        src/crwcrkernel.cuh
        src/crwcrgpusolver.cu
    )
	ADD_DEFINITIONS(-DUSE_GPU)
else()
    set(SOLVER_SOURCE_FILES
        src/crwcrsolver.h
        src/crwcrsolver.cpp
    )
endif()

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES} ${SOLVER_SOURCE_FILES} ${QRCS})
target_link_libraries(${PROJECT_NAME} Qt5::Widgets opengl32.lib)