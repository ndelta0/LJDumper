# FindLabJackM.cmake
#
# Finds the LabJackM (LJM) library
#
# Provides:
#   LabJackM_FOUND
#   LabJackM_VERSION
#   LabJackM_INCLUDE_DIRS
#   LabJackM_LIBRARIES
#   LabJackM::LabJackM (imported target)
#
# Expected layout (default /usr/local):
#   include/LabJackM.h
#   include/LJM_StreamUtilities.h
#   include/LJM_Utilities.h
#   lib/libLabJackM.so
#   share/LabJack/ljm.version

include(FindPackageHandleStandardArgs)

# --- Headers ---
find_path(LabJackM_INCLUDE_DIR
        NAMES
        LabJackM.h
        LJM_StreamUtilities.h
        LJM_Utilities.h
        PATHS
        /usr/local/include
        /usr/include
)

# --- Library ---
find_library(LabJackM_LIBRARY
        NAMES LabJackM
        PATHS
        /usr/local/lib
        /usr/lib
)

# --- Version ---
find_file(LabJackM_VERSION_FILE
        NAMES ljm.version
        PATHS
        /usr/local/share/LabJack
        /usr/share/LabJack
)

if (LabJackM_VERSION_FILE)
    file(READ "${LabJackM_VERSION_FILE}" _LJM_VERSION_CONTENT)
    string(STRIP "${_LJM_VERSION_CONTENT}" LabJackM_VERSION)
endif ()

# --- Standard handling ---
find_package_handle_standard_args(LabJackM
        REQUIRED_VARS
        LabJackM_LIBRARY
        LabJackM_INCLUDE_DIR
        VERSION_VAR
        LabJackM_VERSION
)

if (LabJackM_FOUND)
    set(LabJackM_LIBRARIES ${LabJackM_LIBRARY})
    set(LabJackM_INCLUDE_DIRS ${LabJackM_INCLUDE_DIR})

    if (NOT TARGET LabJackM::LabJackM)
        add_library(LabJackM::LabJackM SHARED IMPORTED)
        set_target_properties(LabJackM::LabJackM PROPERTIES
                IMPORTED_LOCATION "${LabJackM_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${LabJackM_INCLUDE_DIR}"
        )
    endif ()
endif ()

mark_as_advanced(
        LabJackM_INCLUDE_DIR
        LabJackM_LIBRARY
        LabJackM_VERSION_FILE
)
