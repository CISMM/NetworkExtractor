PROJECT (FibrinAnalysis)
CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

# Set version information
SET (FIBRIN_ANALYSIS_MAJOR_NUMBER "1")
SET (FIBRIN_ANALYSIS_MINOR_NUMBER "0")
SET (FIBRIN_ANALYSIS_REVISION_NUMBER "1")
SET (FIBRIN_ANALYSIS_VERSION_STRING "${FIBRIN_ANALYSIS_MAJOR_NUMBER}.${FIBRIN_ANALYSIS_MINOR_NUMBER}.${FIBRIN_ANALYSIS_REVISION_NUMBER}")

SET (EXECUTABLE_OUTPUT_PATH "${FibrinAnalysis_BINARY_DIR}/bin")
SET (LIBRARY_OUTPUT_PATH "${FibrinAnalysis_BINARY_DIR}/lib")

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
# Personal filter library and support classes.
#
# FibrinAnalysis is split up into several logical groups.
# apps      - a place for various applications related to the main application
#######################################
INCLUDE_DIRECTORIES (
    ${FibrinAnalysis_SOURCE_DIR}/lib
    ${FibrinAnalysis_SOURCE_DIR}/lib/filters
    ${FibrinAnalysis_SOURCE_DIR}/lib/filters/itk
    ${FibrinAnalysis_SOURCE_DIR}/lib/io
    ${FibrinAnalysis_SOURCE_DIR}/lib/model
    ${FibrinAnalysis_SOURCE_DIR}/lib/visualization
    ${FibrinAnalysis_BINARY_DIR}/lib)
ADD_SUBDIRECTORY(apps)
ADD_SUBDIRECTORY(lib)

#######################################
# Testing components
#######################################
#OPTION (BUILD_IT_TESTS "Include FibrinAnalysis unit tests" ON)
#IF (BUILD_IT_TESTS)
#    SUBDIRS(TestSuite Tests)
#ENDIF (BUILD_IT_TESTS)
