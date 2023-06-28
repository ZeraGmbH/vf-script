cmake_minimum_required(VERSION 3.1)

project(vf-script LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(VfScript_VERSION_MAJOR "0")
set(VfScript_VERSION_MINOR "0")
set(VfScript_VERSION_PATCH "1")
set(PROJECT_VERSION "${VfScript_VERSION_MAJOR}.${VfScript_VERSION_MINOR}.${VfScript_VERSION_PATCH}")

include(FeatureSummary)
include(GNUInstallDirs)
include(GenerateExportHeader)
include(CMakePackageConfigHelpers)

find_package(Qt5 COMPONENTS Core Qml Quick CONFIG REQUIRED)
find_package(VeinFramework REQUIRED)
find_package(VfQml REQUIRED)

file(GLOB SOURCES 
    ${PROJECT_SOURCE_DIR}/*.cpp
    )

file(GLOB HEADER
    ${PROJECT_SOURCE_DIR}/*.h
    )

add_library(VfScript SHARED
    ${SOURCES}
    ${HEADER}
    )

target_link_libraries(VfScript 
    PRIVATE
    Qt5::Core
    Qt5::Qml
    Qt5::Quick
    Zera::VfHelpers
    Zera::VfEvent
    Zera::VfComponent
    Zera::VfStorageHash
    VeinMeta::VfQml
    )

target_include_directories(VfScript
    PUBLIC
    $<INSTALL_INTERFACE:include/VfScript>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    )

set_target_properties(VfScript PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(VfScript PROPERTIES SOVERSION ${VfScript_VERSION_MAJOR})

generate_export_header(VfScript)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/vfscript_export.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/VfScript
    )

install(TARGETS VfScript
    EXPORT VfScriptExport
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

write_basic_package_version_file(
    VfScriptConfigVersion.cmake
    VERSION ${PACKAGE_VERSION}
    COMPATIBILITY SameMajorVersion
    )

configure_file(VfScriptConfig.cmake.in VfScriptConfig.cmake @ONLY)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/VfScriptConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/VfScriptConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/VfScript
    )

install(EXPORT VfScriptExport
    FILE VfScriptTargets.cmake
    NAMESPACE VeinMeta::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/VfScript
    )

install(
    FILES ${HEADER}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/VfScript
    )

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
