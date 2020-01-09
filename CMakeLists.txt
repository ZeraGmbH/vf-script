cmake_minimum_required(VERSION 3.1)

#set project name and language
project(vf-script LANGUAGES CXX)

#Automoc and RCC ON. Needed in QT projects
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
#C++ 11 Flag# spawn out some info on configuration
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#Set project Version
set(VfScript_VERSION_MAJOR "0")
set(VfScript_VERSION_MINOR "0")
set(VfScript_VERSION_PATCH "1")

set(PROJECT_VERSION "${VfScript_VERSION_MAJOR}.${VfScript_VERSION_MINOR}.${VfScript_VERSION_PATCH}")

#Adding cmake functions like feature_summary()
include(FeatureSummary)
include(GNUInstallDirs)

#Find dependecies
find_package(Qt5 COMPONENTS Core Qml Quick CONFIG REQUIRED)
find_package(VfHelpers REQUIRED)
find_package(VfEvent REQUIRED)
find_package(VfComponent REQUIRED)
find_package(VfStorageHash REQUIRED)
find_package(VfQml REQUIRED)

#sum up project Files 
file(GLOB SOURCES 
    ${PROJECT_SOURCE_DIR}/*.cpp
    )

file(GLOB HEADER
    ${PROJECT_SOURCE_DIR}/*.h
    )

file(GLOB RESOURCES 
    ${PROJECT_SOURCE_DIR}/*.qrc
    )

#create library 
add_library(VfScript SHARED
    ${SOURCES}
    ${HEADER}
    ${RESOURCES}
    )


#link dependencies
target_link_libraries(VfScript 
    PRIVATE
    Qt5::Core
    Qt5::Qml
    Qt5::Quick
    VeinMeta::VfHelpers
    VeinMeta::VfEvent
    VeinMeta::VfComponent 
    VeinMeta::VfStorageHash
    VeinMeta::VfQml
    )

# announce headers - target perspective
target_include_directories(VfScript
    PUBLIC
    $<INSTALL_INTERFACE:include/VfScript>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}

    )

#set target Version
set_target_properties(VfScript PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(VfScript PROPERTIES SOVERSION ${VfScript_VERSION_MAJOR})


#generate export header
include(GenerateExportHeader)
generate_export_header(VfScript)

#install export header
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/vfscript_export.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/VfScript
    )



# install library
install(TARGETS VfScript
    EXPORT VfScriptExport
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

# configure MarkdownQtConfigVersion.cmake
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    VfScriptConfigVersion.cmake
    VERSION ${PACKAGE_VERSION}
    COMPATIBILITY SameMinorVersion
    )

# configure *Config.cmake
configure_file(VfScriptConfig.cmake.in VfScriptConfig.cmake @ONLY)

# install *Config(Version).cmake
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/VfScriptConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/VfScriptConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/VfScript
    )

# install targets cmake-files
install(EXPORT VfScriptExport
    FILE VfScriptTargets.cmake
    NAMESPACE VeinMeta::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/VfScript
    )

# install public headers
install(
    FILES ${HEADER}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/VfScript
    )

# spawn out some info on configuration
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)



