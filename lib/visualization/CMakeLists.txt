SET(visualizationSrc
  IsosurfaceVisualizationPipeline.cxx
  OutlineVisualizationPipeline.cxx
  VisualizationPipeline.cxx
)

ADD_LIBRARY(faVisualization ${visualizationSrc})