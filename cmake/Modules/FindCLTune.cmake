set(CLTUNE_HINTS
    ${CLTUNE_ROOT}
    "C:/Program Files/cltune"
    "C:/Program Files (x86)/cltune"
    )

set(CLTUNE_PATHS
    /usr/local/cltune)

find_path(CLTUNE_INCLUDE_DIR
    NAMES cltune.h
    HINTS ${CLTUNE_HINTS}
    PATH_SUFFIXES include
    PATHS ${CLTUNE_PATHS}
    DOC "CLTune header file cltune.h")
mark_as_advanced(CLTUNE_INCLUDE_DIR)

find_library(CLTUNE_LIBRARY
    NAMES cltune cltune
    HINTS ${CLTUNE_HINTS}
    PATH_SUFFIXES lib
    PATHS ${CLTUNE_PATHS}
    DOC "CLTune library file")
mark_as_advanced(CLTUNE_LIBRARY)

# ==================================================================================================

# Notification messages
if(NOT CLTUNE_INCLUDE_DIR)
    message(STATUS "Could NOT find 'cltune.h', install CLTune or set CLTUNE_ROOT")
endif()
if(NOT CLTUNE_LIBRARY)
    message(STATUS "Could NOT find CLTune library, install it or set CLTUNE_ROOT")
endif()

# Determines whether or not CLTune was found
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CLTune DEFAULT_MSG CLTUNE_INCLUDE_DIR CLTUNE_LIBRARY)

# ==================================================================================================
