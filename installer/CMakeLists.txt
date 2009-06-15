IF ( WIN32 )
  INCLUDE( "${FibrinAnalysis_SOURCE_DIR}/installer/Win32/CMake/Win32Package.txt" )
ENDIF ( WIN32 )

IF ( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
  INCLUDE( "${FibrinAnalysis_SOURCE_DIR}/installer/MacOSX/CMake/MacOSXPackage.txt" )
ENDIF ( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )