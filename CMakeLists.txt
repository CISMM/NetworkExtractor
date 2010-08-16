PROJECT (NetworkExtractor)
CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
IF (COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
ENDIF (COMMAND cmake_policy)

# Set version information
SET (NETWORK_EXTRACTOR_MAJOR_NUMBER "1")
SET (NETWORK_EXTRACTOR_MINOR_NUMBER "0")
SET (NETWORK_EXTRACTOR_REVISION_NUMBER "0")
SET (NETWORK_EXTRACTOR_VERSION_STRING "${NETWORK_EXTRACTOR_MAJOR_NUMBER}.${NETWORK_EXTRACTOR_MINOR_NUMBER}.${NETWORK_EXTRACTOR_REVISION_NUMBER}")

SET (EXECUTABLE_OUTPUT_PATH "${NetworkExtractor_BINARY_DIR}/bin")
SET (LIBRARY_OUTPUT_PATH "${NetworkExtractor_BINARY_DIR}/lib")

OPTION (CMAKE_VERBOSE_MAKEFILE  "Enable/Disable verbose compiler output" ON)
OPTION (CMAKE_COLOR_MAKEFILE "Enable/Disable color cues when building" ON)
MARK_AS_ADVANCED (CLEAR CMAKE_VERBOSE_MAKEFILE CMAKE_COLOR_MAKEFILE)

#######################################
# Include ITK
#######################################

FIND_PACKAGE(ITK)
IF(ITK_FOUND)
  INCLUDE(${ITK_USE_FILE})
ELSE(ITK_FOUND)
  MESSAGE(FATAL_ERROR
     "Cannot build without ITK.  Please set ITK_DIR.")
ENDIF(ITK_FOUND)

#######################################
# Include VTK
#######################################

FIND_PACKAGE(VTK)
IF (VTK_FOUND)
  INCLUDE(${VTK_USE_FILE})
ELSE(VTK_FOUND)
  MESSAGE(FATAL_ERROR
     "Cannot build without VTK.  Please set VTK_DIR.")
ENDIF (VTK_FOUND)

#######################################
# Include Qt4
#######################################
FIND_PACKAGE(Qt4)
IF (QT_FOUND)
  INCLUDE(${QT_USE_FILE})
ELSE (QT_FOUND)
  MESSAGE(FATAL_ERROR
     "Cannot find Qt 4, which is required.")
ENDIF( QT_FOUND)

#######################################
# Personal filter library and support classes.
#
# NetworkExtractor is split up into several logical groups.
# apps      - a place for various applications related to the main application
#######################################
INCLUDE_DIRECTORIES (
    ${NetworkExtractor_SOURCE_DIR}/lib
    ${NetworkExtractor_SOURCE_DIR}/lib/filters
    ${NetworkExtractor_SOURCE_DIR}/lib/filters/itk
    ${NetworkExtractor_SOURCE_DIR}/lib/io
    ${NetworkExtractor_SOURCE_DIR}/lib/model
    ${NetworkExtractor_SOURCE_DIR}/lib/visualization
    ${NetworkExtractor_BINARY_DIR}/lib)
ADD_SUBDIRECTORY(apps)
ADD_SUBDIRECTORY(lib)

#######################################
# Testing components
#######################################
#OPTION (BUILD_IT_TESTS "Include NetworkExtractor unit tests" ON)
#IF (BUILD_IT_TESTS)
#    SUBDIRS(TestSuite Tests)
#ENDIF (BUILD_IT_TESTS)
