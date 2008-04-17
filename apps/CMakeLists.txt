###################################
# FibrinAnalysis GUI executables.
#
# Note: It is *absolutely* necessary to include "WIN32" on the
# line below. (e.g. ADD_EXECUTABLE(FibrinAnalysis WIN32 FibrinAnalysis.cxx))
# Failing to do that does not set up a wx application using the 
# IMPLEMENT_APP() macro.
###################################
IF (WIN32)
    SET (GUI_EXE WIN32)
    # This seems to be necessary for debug builds...for some reason, though, we
    # can't just use CMAKE_DEBUG_EXE_LINKER_FLAGS
    # SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:msvcrt")
ENDIF (WIN32)

INCLUDE_DIRECTORIES(../wxGui)

ADD_EXECUTABLE(FibrinAnalysis ${GUI_EXE} MACOSX_BUNDLE FibrinAnalysisApp.cxx)
TARGET_LINK_LIBRARIES(FibrinAnalysis FibrinAnalysisGui nanoWx nanoVTKView vtksys vtkCommon vtkFiltering vtkGraphics vtkHybrid vtkImaging vtkRendering vtkVolumeRendering itksys ITKCommon ITKBasicFilters ITKIO)
