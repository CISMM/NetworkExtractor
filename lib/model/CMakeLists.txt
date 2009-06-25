SET(modelSrc
  DataModel.cxx
)

ADD_LIBRARY(faModel ${modelSrc})

TARGET_LINK_LIBRARIES( faModel
  vtksys
  vtkCommon
  vtkFiltering
  vtkGraphics
  vtkHybrid
  vtkImaging
  vtkRendering
  vtkVolumeRendering
  itksys
  ITKCommon
  ITKBasicFilters
  ITKIO
)
