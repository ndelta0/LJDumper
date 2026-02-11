# FindLabJackM.cmake
#
# Provides:
#   LabJackM_FOUND
#   LabJackM_VERSION
#   LabJackM_INCLUDE_DIRS
#   LabJackM_LIBRARIES
#   LabJackM::LabJackM

include(FindPackageHandleStandardArgs)

# ------------------------------------------------------------
# Platform-specific roots
# ------------------------------------------------------------
if (WIN32)
    set(_LJM_ROOTS
            "$ENV{ProgramFiles\(x86\)}/LabJack/Drivers"
            "$ENV{ProgramFiles}/LabJack/Drivers"
    )
else()
    set(_LJM_ROOTS
            /usr/local
            /usr
    )
endif()

# ------------------------------------------------------------
# Headers
# ------------------------------------------------------------
if (WIN32)
    find_path(LabJackM_INCLUDE_DIR
            NAMES LabJackM.h
            PATHS ${_LJM_ROOTS}
            NO_DEFAULT_PATH
    )
else()
    find_path(LabJackM_INCLUDE_DIR
            NAMES
            LabJackM.h
            PATHS ${_LJM_ROOTS}
            PATH_SUFFIXES include
    )
endif()

# ------------------------------------------------------------
# Library
# ------------------------------------------------------------
if (WIN32)
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        # 64-bit
        find_library(LabJackM_LIBRARY
                NAMES LabJackM
                PATHS ${_LJM_ROOTS}
                PATH_SUFFIXES 64bit
                NO_DEFAULT_PATH
        )
    else()
        # 32-bit
        find_library(LabJackM_LIBRARY
                NAMES LabJackM
                PATHS ${_LJM_ROOTS}
                NO_DEFAULT_PATH
        )
    endif()

    # DLL location (for runtime)
    find_file(LabJackM_DLL
            NAMES LabJack.LJM.dll
            PATHS ${_LJM_ROOTS}
            NO_DEFAULT_PATH
    )
else()
    find_library(LabJackM_LIBRARY
            NAMES LabJackM
            PATHS ${_LJM_ROOTS}
            PATH_SUFFIXES lib
    )
endif()

# ------------------------------------------------------------
# Version (Linux only – Windows drivers don’t ship one cleanly)
# ------------------------------------------------------------
if (NOT WIN32)
    find_file(LabJackM_VERSION_FILE
            NAMES ljm.version
            PATHS ${_LJM_ROOTS}
            PATH_SUFFIXES share/LabJack
    )

    if (LabJackM_VERSION_FILE)
        file(READ "${LabJackM_VERSION_FILE}" _LJM_VERSION_CONTENT)
        string(STRIP "${_LJM_VERSION_CONTENT}" LabJackM_VERSION)
    endif()
endif()

# ------------------------------------------------------------
# Package handling
# ------------------------------------------------------------
find_package_handle_standard_args(LabJackM
        REQUIRED_VARS
        LabJackM_LIBRARY
        LabJackM_INCLUDE_DIR
        VERSION_VAR
        LabJackM_VERSION
)

# ------------------------------------------------------------
# Imported target
# ------------------------------------------------------------
if (LabJackM_FOUND AND NOT TARGET LabJackM::LabJackM)
    if (WIN32)
        add_library(LabJackM::LabJackM SHARED IMPORTED)
        set_target_properties(LabJackM::LabJackM PROPERTIES
                IMPORTED_IMPLIB "${LabJackM_LIBRARY}"
                IMPORTED_LOCATION "${LabJackM_DLL}"
                INTERFACE_INCLUDE_DIRECTORIES "${LabJackM_INCLUDE_DIR}"
        )
    else()
        add_library(LabJackM::LabJackM SHARED IMPORTED)
        set_target_properties(LabJackM::LabJackM PROPERTIES
                IMPORTED_LOCATION "${LabJackM_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${LabJackM_INCLUDE_DIR}"
        )
    endif()
endif()

set(LabJackM_LIBRARIES ${LabJackM_LIBRARY})
set(LabJackM_INCLUDE_DIRS ${LabJackM_INCLUDE_DIR})

mark_as_advanced(
        LabJackM_LIBRARY
        LabJackM_INCLUDE_DIR
        LabJackM_VERSION_FILE
        LabJackM_DLL
)
