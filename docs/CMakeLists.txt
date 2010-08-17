# Configure the documentation guide with the library/program version number
CONFIGURE_FILE( "${NetworkExtractor_SOURCE_DIR}/docs/Configuration.in.tex"
  "${NetworkExtractor_BINARY_DIR}/docs/Configuration.tex" @ONLY )

# This is a simple file copy
CONFIGURE_FILE( "${NetworkExtractor_SOURCE_DIR}/docs/NetworkExtractor-UserGuide.tex"
  "${NetworkExtractor_BINARY_DIR}/docs/NetworkExtractor-${NetworkExtractor_VERSION_STRING}-UserGuide.tex"
  COPYONLY )
