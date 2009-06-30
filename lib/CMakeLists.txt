ADD_SUBDIRECTORY(filters)
ADD_SUBDIRECTORY(io)
ADD_SUBDIRECTORY(model)
ADD_SUBDIRECTORY(visualization)

CONFIGURE_FILE( "${FibrinAnalysis_SOURCE_DIR}/lib/Version.h.in"
  "${FibrinAnalysis_BINARY_DIR}/lib/Version.h" )