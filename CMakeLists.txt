cmake_minimum_required(VERSION 3.28)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
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
        widgets
        REQUIRED)

find_package(LabJackM REQUIRED)

add_executable(LJDumper
        src/main.cpp
        src/lj/ljManager.cpp
        src/lj/ljManager.hpp
        src/views/LjdMainWindow.cpp
        src/views/LjdMainWindow.hpp
        src/views/LjdMainWindow.ui
        src/lj/utils.cpp
        src/lj/utils.hpp
        src/widgets/DPinConfig.cpp
        src/widgets/DPinConfig.hpp
        src/widgets/DPinConfig.ui
        src/widgets/DAnalogPinConfig.cpp
        src/widgets/DAnalogPinConfig.hpp
        src/widgets/DAnalogPinConfig.ui
        src/widgets/DFlowLayout.cpp
        src/widgets/DFlowLayout.hpp
        src/fileNameTemplateValidator.cpp
        src/fileNameTemplateValidator.hpp
        src/views/FileNameTemplatingDialog.cpp
        src/views/FileNameTemplatingDialog.hpp
        src/views/FileNameTemplatingDialog.ui
        src/views/ScanRateDialog.cpp
        src/views/ScanRateDialog.hpp
        src/views/ScanRateDialog.ui
        src/views/LjOutputControlWindow.cpp
        src/views/LjOutputControlWindow.hpp
        src/views/LjOutputControlWindow.ui
        src/widgets/WOutputPinRow.cpp
        src/widgets/WOutputPinRow.hpp
        src/widgets/WOutputPinRow.ui
        src/utils.hpp
        src/widgets/WClockConfig.cpp
        src/widgets/WClockConfig.hpp
        src/widgets/WClockConfig.ui
        src/widgets/Int64SpinBox.cpp
        src/widgets/Int64SpinBox.hpp
)
target_include_directories(LJDumper PUBLIC ${CMAKE_BINARY_DIR}/LJDumper_autogen/include ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(LJDumper
        Qt::Core
        Qt::Gui
        Qt::Widgets
        LabJackM::LabJackM
)

if(MSVC)
    target_compile_options(LJDumper PRIVATE /Zc:preprocessor /wd4229)
endif()