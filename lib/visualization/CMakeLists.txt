SET(visualizationSrc
  ArrowVisualizationPipeline.cxx
  DVRIsosurfaceVisualizationPipeline.cxx
  ImagePlaneVisualizationPipeline.cxx
  IsosurfaceVisualizationPipeline.cxx
  OutlineVisualizationPipeline.cxx
  VisualizationPipeline.cxx
  Visualization.cxx
)

ADD_LIBRARY(faVisualization ${visualizationSrc})
