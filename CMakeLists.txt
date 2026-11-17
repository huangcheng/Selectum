cmake_minimum_required(VERSION 3.19)
project(Selectum LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Test LinguistTools)

qt_standard_project_setup()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(selectum_abi STATIC
    src/abi/json.cpp
)
target_include_directories(selectum_abi
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(selectum_abi
    PUBLIC
        Qt::Core
)

add_library(selectum_core STATIC
    src/core/match_engine.cpp
    src/core/plugin_manifest.cpp
)
target_include_directories(selectum_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(selectum_core
    PUBLIC
        selectum_abi
        Qt::Core
)

enable_testing()
add_subdirectory(tests)

set(SELECTUM_APP_ICON_ICNS "${CMAKE_CURRENT_SOURCE_DIR}/assets/brand/Selectum.icns")
set(SELECTUM_APP_ICON_ICO "${CMAKE_CURRENT_SOURCE_DIR}/assets/brand/Selectum.ico")

qt_add_executable(Selectum
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    ${SELECTUM_APP_ICON_ICNS}
)

# Expose the PNG at :/icons/selectum.png for QIcon
set_source_files_properties(assets/brand/selectum-icon.png PROPERTIES
    QT_RESOURCE_ALIAS "icons/selectum.png"
)

qt_add_resources(Selectum "selectum_assets"
    PREFIX "/"
    FILES
        assets/brand/selectum-icon.png
)

qt_add_translations(
    TARGETS Selectum
    TS_FILES Selectum_zh_CN.ts
)

target_link_libraries(Selectum
    PRIVATE
        Qt::Core
        Qt::Widgets
)

set_target_properties(Selectum PROPERTIES
    MACOSX_BUNDLE_BUNDLE_NAME "Selectum"
    MACOSX_BUNDLE_GUI_IDENTIFIER "dev.selectum.app"
    MACOSX_BUNDLE_ICON_FILE "Selectum.icns"
    MACOSX_BUNDLE_INFO_STRING "Selectum"
    WIN32_EXECUTABLE TRUE
)

set_source_files_properties(${SELECTUM_APP_ICON_ICNS} PROPERTIES
    MACOSX_PACKAGE_LOCATION "Resources"
)

if(WIN32)
    set_target_properties(Selectum PROPERTIES
        RC_ICONS "${SELECTUM_APP_ICON_ICO}"
    )
endif()


install(TARGETS Selectum
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET Selectum
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
