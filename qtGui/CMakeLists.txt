INCLUDE(${VTK_USE_FILE})

# use what QVTK built with
SET(QT_MOC_EXECUTABLE ${VTK_QT_MOC_EXECUTABLE} CACHE FILEPATH "")
SET(QT_UIC_EXECUTABLE ${VTK_QT_UIC_EXECUTABLE} CACHE FILEPATH "")
SET(QT_QMAKE_EXECUTABLE ${VTK_QT_QMAKE_EXECUTABLE} CACHE FILEPATH "")
SET(DESIRED_QT_VERSION ${VTK_DESIRED_QT_VERSION} CACHE FILEPATH "")
FIND_PACKAGE(Qt)
IF(QT_USE_FILE)
  INCLUDE(${QT_USE_FILE})
ELSE(QT_USE_FILE)
  SET(QT_LIBRARIES   ${QT_QT_LIBRARY})
ENDIF(QT_USE_FILE)

SET (QtGuiSrcs
  Main.cxx
  FibrinAppQt.cxx
  Visualization.cxx
)

# Use the include path and library for Qt that is used by VTK.
INCLUDE_DIRECTORIES(
  ${QT_INCLUDE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

SET(UIS
  FibrinAppQt.ui
)
SET (FibrinAppHeaders
  FibrinAppQt.h
)
QT4_WRAP_UI(UIHeaders ${UIS})
QT4_WRAP_CPP(MOCSrcs ${FibrinAppHeaders} )
ADD_DEFINITIONS(-DQT_GUI_LIBS -DQT_CORE_LIB -DQT3_SUPPORT)
SET_SOURCE_FILES_PROPERTIES(${QtGuiSrcs} PROPERTIES
                          OBJECT_DEPENDS "${UIHeaders}")


# It's nice to have the ui in the windows project file...just double click on it
# and designer comes up on that ui file :)
IF (${CMAKE_BUILD_TOOL} MATCHES "msdev")
  SET (QtGuiSrcs ${QtGuiSrcs} ${UIS})
ENDIF (${CMAKE_BUILD_TOOL} MATCHES "msdev")
IF (${CMAKE_BUILD_TOOL} MATCHES "devenv")
  SET (QtGuiSrcs ${QtGuiSrcs} ${UIS})
ENDIF (${CMAKE_BUILD_TOOL} MATCHES "devenv")

ADD_EXECUTABLE( FibrinAnalysis ${QtGuiSrcs} ${UISrcs} ${MOCSrcs})

TARGET_LINK_LIBRARIES( FibrinAnalysis
  QVTK
  ${QT_LIBRARIES}
  vtksys
  vtkCommon
  vtkFiltering
  vtkGraphics
  vtkHybrid
  vtkImaging
  vtkRendering
  vtkVolumeRendering
  itksys
  ITKCommon
  ITKBasicFilters
  ITKIO
  faFilter
  faIO
  faVisualization
)

# Set package properties
SET( CPACK_PACKAGE_NAME "Fibrin-Analysis" )
SET( CPACK_PACKAGE_VENDOR "University of North Carolina CISMM" )
SET( CPACK_PACKAGE_EXECUTABLES "FibrinAnalysis;Fibrin Analysis 1.0.0" )
SET( CPACK_PACKAGE_DESCRIPTION_SUMMARY "Fibrin Analysis" )
SET( CPACK_PACKAGE_VERSION "1.0.0" )
SET( CPACK_PACKAGE_VERSION_MAJOR "1" )
SET( CPACK_PACKAGE_VERSION_MINOR "0" )
SET( CPACK_PACKAGE_VERSION_PATCH "0" )
SET( CPACK_PACKAGE_INSTALL_DIRECTORY "Fibrin Analysis 1.0.0" )
SET( CPACK_PACKAGE_INSTALL_REGISTRY_KEY "Fibrin Analysis 1.0.0" )
SET( CPACK_RESOURCE_FILE_LICENSE "${FibrinAnalysis_SOURCE_DIR}/installer/licenses/FibrinAnalysis-license.txt" )
SET( CPACK_RESOURCE_FILE_README  "${FibrinAnalysis_SOURCE_DIR}/installer/licenses/README.txt" )

INCLUDE( ../installer/CMakeLists.txt )

INSTALL( TARGETS FibrinAnalysis
  RUNTIME DESTINATION bin
)

# This must always be last!
INCLUDE( CPack )

# This includes Microsoft runtime libraries when run in Windows dev environment.
INCLUDE( InstallRequiredSystemLibraries )
