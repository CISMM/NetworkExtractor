SET(filtersSrc
  itk/itkEigenValues3DToFrangiVesselnessMeasureImageFilter.cxx
  itk/itkEigenValues3DToSatoVesselnessMeasureImageFilter.cxx
  itk/itkEigenVectors3DToJunctionnessMeasureImageFilter.cxx
  itk/itkHessian3DEigenAnalysisImageFilter.cxx
  itk/itkMultiScaleHessianBasedMeasureImageFilter2.cxx
)

ADD_LIBRARY(faFilter ${filtersSrc})