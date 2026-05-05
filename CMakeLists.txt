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
target_include_directories(LJDumper PUBLIC ${CMAKE_BINARY_DIR}/LJDumper_autogen/include ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(LJDumper
        Qt::Core
        Qt::Gui
        Qt::Widgets
        LabJackM::LabJackM
)

if(MSVC)
    target_compile_options(LJDumper PRIVATE /Zc:preprocessor /wd4229)
endif()

install(
        TARGETS LJDumper
        RUNTIME DESTINATION bin
)

qt_generate_deploy_app_script(
        TARGET LJDumper
        OUTPUT_SCRIPT qt_deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
)

install(SCRIPT ${qt_deploy_script})

set(LABJACK_BASIC_INSTALLER_URL
        "https://files.labjack.com/installers/LJM/Windows/x86_64/beta/LabJackBasic_2025-02-12.exe"
        CACHE STRING
        "URL of the LabJack Basic installer"
)

set(LABJACK_BASIC_INSTALLER_PATH
        "${CMAKE_SOURCE_DIR}/installer/packages/com.labjack.ljm/data/LabJackBasic_2025-02-12.exe"
)

if(WIN32 AND NOT EXISTS "${LABJACK_BASIC_INSTALLER_PATH}")
    message(STATUS "Downloading LabJack Basic installer...")

    file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/installer/packages/com.labjack.ljm/data")

    file(DOWNLOAD
            "${LABJACK_BASIC_INSTALLER_URL}"
            "${LABJACK_BASIC_INSTALLER_PATH}"
            SHOW_PROGRESS
            TLS_VERIFY ON
            STATUS LABJACK_BASIC_DOWNLOAD_STATUS
    )

    list(GET LABJACK_BASIC_DOWNLOAD_STATUS 0 LABJACK_BASIC_DOWNLOAD_RESULT)
    list(GET LABJACK_BASIC_DOWNLOAD_STATUS 1 LABJACK_BASIC_DOWNLOAD_MESSAGE)

    if(NOT LABJACK_BASIC_DOWNLOAD_RESULT EQUAL 0)
        file(REMOVE "${LABJACK_BASIC_INSTALLER_PATH}")
        message(FATAL_ERROR
                "Failed to download LabJack Basic installer: ${LABJACK_BASIC_DOWNLOAD_MESSAGE}"
        )
    endif()
endif()

find_program(IFW_BINARYCREATOR_EXECUTABLE
        NAMES binarycreator binarycreator.exe
        HINTS
        "$ENV{QTIFWDIR}/bin"
        "D:/Qt/Tools/QtInstallerFramework/4.8/bin"
        "D:/Qt/Tools/QtInstallerFramework/4.9/bin"
        "D:/Qt/Tools/QtInstallerFramework/4.10/bin"
        "D:/Qt/Tools/QtInstallerFramework/4.11/bin"
)

if(IFW_BINARYCREATOR_EXECUTABLE)
    add_custom_target(package-ifw
            COMMAND ${CMAKE_COMMAND}
            --install "${CMAKE_BINARY_DIR}"
            --config $<CONFIG>
            --prefix "${CMAKE_BINARY_DIR}/installer-root"

            COMMAND ${CMAKE_COMMAND} -E rm -rf
            "${CMAKE_SOURCE_DIR}/installer/packages/pl.knowledge.ljdumper/data"

            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_BINARY_DIR}/installer-root"
            "${CMAKE_SOURCE_DIR}/installer/packages/pl.knowledge.ljdumper/data"

            COMMAND "${IFW_BINARYCREATOR_EXECUTABLE}"
            -c "${CMAKE_SOURCE_DIR}/installer/config/config.xml"
            -p "${CMAKE_SOURCE_DIR}/installer/packages"
            "${CMAKE_BINARY_DIR}/LJDumperSetup.exe"

            DEPENDS LJDumper
            COMMENT "Creating Qt Installer Framework installer"
    )
else()
    message(STATUS "Qt Installer Framework binarycreator not found; package-ifw target disabled.")
endif()