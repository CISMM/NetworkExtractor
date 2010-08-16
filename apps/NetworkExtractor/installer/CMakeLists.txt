IF ( WIN32 )
  INCLUDE( "${NetworkExtractor_SOURCE_DIR}/apps/NetworkExtractor/installer/Win32/CMake/Win32Package.txt" )
ENDIF ( WIN32 )

IF ( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
  INCLUDE( "${NetworkExtractor_SOURCE_DIR}/apps/NetworkExtractor/installer/MacOSX/CMake/MacOSXPackage.txt" )
ENDIF ( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )