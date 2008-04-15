PROJECT (gui)

SET (Gui_SRCS
    FibrinAnalysisGUI.h FibrinAnalysisGUI.cxx
)

# A library, so others can use the GUI tools
ADD_LIBRARY(FibrinAnalysisGui ${Gui_SRCS})
TARGET_LINK_LIBRARIES(FibrinAnalysisGui )
