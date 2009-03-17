SET(filtersSrc
  IntensityThresholdThinningFilter.cxx
  itk/itkEigenValues3DToVesselnessMeasureImageFilter.cxx
  itk/itkHessian3DEigenAnalysisImageFilter.cxx
)

ADD_LIBRARY(faFilter ${filtersSrc})