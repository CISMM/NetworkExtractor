SET(ioSrc
  LoadVTKImageFile.cxx
  ITKImageToVTKImage.cxx
)

ADD_LIBRARY(faIO ${ioSrc})