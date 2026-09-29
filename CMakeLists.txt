cmake_minimum_required(VERSION 3.16)
project(SherlogCostVideo LANGUAGES CXX C)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick)

qt_add_executable(SherlogCostVideo WIN32
    main.cpp
    costdatasource.h
    costdatasource.cpp
    triagefeedsource.h
    triagefeedsource.cpp
    referencecatalogsource.h
    referencecatalogsource.cpp
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(SherlogCostVideo PRIVATE QT_QML_DEBUG)
endif()

set_target_properties(SherlogCostVideo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "bin"
    ARCHIVE_OUTPUT_DIRECTORY "lib"
    LIBRARY_OUTPUT_DIRECTORY "lib"
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(SherlogCostVideo PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Qml
    Qt::Quick
)

qt_add_qml_module(SherlogCostVideo
    URI SherlogCostVideo
    VERSION 1.0
    QML_FILES
        "QML/CostChart.qml"
)


