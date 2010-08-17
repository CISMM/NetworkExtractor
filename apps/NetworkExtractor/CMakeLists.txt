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
  NetworkExtractor.cxx
)

# Use the include path and library for Qt that is used by VTK.
INCLUDE_DIRECTORIES(
  ${QT_INCLUDE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

SET(UIS
  NetworkExtractor.ui
)
SET (NetworkExtractorHeaders
  NetworkExtractor.h
)
QT4_WRAP_UI(UIHeaders ${UIS})
QT4_WRAP_CPP(MOCSrcs ${NetworkExtractorHeaders} )
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

# Workaround for rc.exe in Visual Studio 2005 being unable to handle many /I arguments.
IF ( WIN32 )
  SET( WinIcon icon.res )
  ADD_CUSTOM_COMMAND( OUTPUT icon.res COMMAND rc.exe /fo icon.res ${CMAKE_CURRENT_SOURCE_DIR}/installer/Win32/icon.rc )
ENDIF ( WIN32 )

ADD_EXECUTABLE( NetworkExtractor 
  ${WinIcon}
  ${QtGuiSrcs}
  ${UISrcs}
  ${MOCSrcs}
)

TARGET_LINK_LIBRARIES( NetworkExtractor
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
  faModel
  faVisualization
)

# Set package properties
SET( CPACK_PACKAGE_NAME "Network-Extractor" )
SET( CPACK_PACKAGE_VENDOR "University of North Carolina CISMM" )
SET( CPACK_PACKAGE_EXECUTABLES "NetworkExtractor;Network Extractor ${NetworkExtractor_VERSION_STRING}" )
SET( CPACK_PACKAGE_DESCRIPTION_SUMMARY "Network Extractor" )
SET( CPACK_PACKAGE_VERSION "${NetworkExtractor_VERSION_STRING}" )
SET( CPACK_PACKAGE_VERSION_MAJOR "${NetworkExtractor_MAJOR_NUMBER}" )
SET( CPACK_PACKAGE_VERSION_MINOR "${NetworkExtractor_MINOR_NUMBER}" )
SET( CPACK_PACKAGE_VERSION_PATCH "${NetworkExtractor_REVISION_NUMBER}" )
SET( CPACK_PACKAGE_INSTALL_DIRECTORY "Network Extractor ${NetworkExtractor_VERSION_STRING}" )
SET( CPACK_PACKAGE_INSTALL_REGISTRY_KEY "Network Extractor ${NetworkExtractor_VERSION_STRING}" )
SET( CPACK_RESOURCE_FILE_LICENSE "${NetworkExtractor_SOURCE_DIR}/apps/NetworkExtractor/installer/licenses/NetworkExtractor-license.txt" )
SET( CPACK_RESOURCE_FILE_README  "${NetworkExtractor_SOURCE_DIR}/apps/NetworkExtractor/installer/licenses/README.txt" )

INCLUDE( ${NetworkExtractor_SOURCE_DIR}/apps/NetworkExtractor/installer/CMakeLists.txt )

INSTALL( TARGETS NetworkExtractor
  RUNTIME DESTINATION bin
)

# This must always be last!
INCLUDE( CPack )

# This includes Microsoft runtime libraries when run in Windows dev environment.
INCLUDE( InstallRequiredSystemLibraries )
