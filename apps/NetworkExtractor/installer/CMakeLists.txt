IF ( WIN32 )
  INCLUDE( "${FibrinAnalysis_SOURCE_DIR}/apps/FibrinAnalysis/installer/Win32/CMake/Win32Package.txt" )
ENDIF ( WIN32 )

IF ( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
  INCLUDE( "${FibrinAnalysis_SOURCE_DIR}/apps/FibrinAnalysis/installer/MacOSX/CMake/MacOSXPackage.txt" )
ENDIF ( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )