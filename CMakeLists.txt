PROJECT (FibrinAnalysis)

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

INCLUDE_DIRECTORIES(${ITK_INCLUDE_DIRS})
LINK_DIRECTORIES(${ITK_LIBRARY_DIRS})

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

INCLUDE_DIRECTORIES(${VTK_INCLUDE_DIRS})
LINK_DIRECTORIES(${VTK_LIBRARY_DIRS})


###################################
# nanoCommon stuff, including 
# wxWidgets and extensions
###################################
#SET (nanoCommonPath ../nanoCommon)
#INCLUDE (${nanoCommonPath}/cpp/wxvtk/LocalUsewxWidgets.cmake)
#INCLUDE (${CMAKE_ROOT}/Modules/UsewxWidgets.cmake)
#INCLUDE_DIRECTORIES(
#    ${wxWidgets_INCLUDES}
#    ${nanoCommonPath}/cpp/wx
#    ${nanoCommonPath}/cpp/wxvtk
#)
#LINK_DIRECTORIES(${wxWidgets_LIBRARY_DIRS})

# We need to include the binary directory here because nanoCommon
# is outside the build tree.
#ADD_SUBDIRECTORY(${nanoCommonPath}/cpp "${FibrinAnalysis_BINARY_DIR}/nanoCommon")

#######################################
# Personal filter library and support classes.
#
# GenericApplication is split up into several logical groups.
# apps      - a place for various applications related to the main application
#######################################
INCLUDE_DIRECTORIES (
    ${FibrinAnalysis_SOURCE_DIR}/apps)
#ADD_SUBDIRECTORY(apps)

#######################################
# Windowing components
#######################################
#INCLUDE_DIRECTORIES (
#    ${FibrinAnalysis_SOURCE_DIR}/wxGui
#)
#ADD_SUBDIRECTORY(wxGui)
ADD_SUBDIRECTORY(qtGui)

#######################################
# Testing components
#######################################
#OPTION (BUILD_IT_TESTS "Include FibrinAnalysis unit tests" ON)
#IF (BUILD_IT_TESTS)
#    SUBDIRS(TestSuite Tests)
#ENDIF (BUILD_IT_TESTS)
