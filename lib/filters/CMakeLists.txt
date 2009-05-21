SET(filtersSrc
  itk/itkEigenValues3DToSatoVesselnessMeasureImageFilter.cxx
  itk/itkEigenVectors3DToJunctionnessMeasureImageFilter.cxx
  itk/itkHessian3DEigenAnalysisImageFilter.cxx
)

ADD_LIBRARY(faFilter ${filtersSrc})