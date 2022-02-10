cmake_minimum_required (VERSION 3.11)
project(spi_analyzer)

# custom CMake Modules are located in the cmake directory.
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include(ExternalAnalyzerSDK)

set(SOURCES 
src/SpiExAnalyzer.cpp
src/SpiExAnalyzer.h
src/SpiExAnalyzerResults.cpp
src/SpiExAnalyzerResults.h
src/SpiExAnalyzerSettings.cpp
src/SpiExAnalyzerSettings.h
src/SpiExSimulationDataGenerator.cpp
src/SpiExSimulationDataGenerator.h
)

add_analyzer_plugin(spi_analyzer SOURCES ${SOURCES})
