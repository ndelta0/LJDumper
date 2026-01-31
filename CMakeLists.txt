cmake_minimum_required(VERSION 3.28)
project(LJDumper)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED)

find_package(LabJackM REQUIRED)

add_executable(LJDumper main.cpp
        lj/ljManager.cpp
        lj/ljManager.hpp
        views/LjdMainWindow.cpp
        views/LjdMainWindow.hpp
        views/LjdMainWindow.ui
        lj/utils.cpp
        lj/utils.hpp
        widgets/DPinConfig.cpp
        widgets/DPinConfig.hpp
        widgets/DPinConfig.ui
        widgets/DFlowLayout.cpp
        widgets/DFlowLayout.hpp
        fileNameTemplateValidator.cpp
        fileNameTemplateValidator.hpp
        views/FileNameTemplatingDialog.cpp
        views/FileNameTemplatingDialog.hpp
        views/FileNameTemplatingDialog.ui
        views/ScanRateDialog.cpp
        views/ScanRateDialog.hpp
        views/ScanRateDialog.ui
        views/LjOutputControlWindow.cpp
        views/LjOutputControlWindow.hpp
        views/LjOutputControlWindow.ui
        widgets/WOutputPinRow.cpp
        widgets/WOutputPinRow.hpp
        widgets/WOutputPinRow.ui
        utils.hpp
        widgets/WClockConfig.cpp
        widgets/WClockConfig.hpp
        widgets/WClockConfig.ui
        widgets/Int64SpinBox.cpp
        widgets/Int64SpinBox.hpp
)
target_include_directories(LJDumper PUBLIC ${CMAKE_BINARY_DIR}/LJDumper_autogen/include ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(LJDumper
        Qt::Core
        Qt::Gui
        Qt::Widgets
        LabJackM::LabJackM
)

