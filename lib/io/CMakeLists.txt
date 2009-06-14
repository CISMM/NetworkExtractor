SET(ioSrc
  ITKImageToVTKImage.cxx
  XMLHelper.cxx
)

INCLUDE_DIRECTORIES(
  ${VTK_DIR}/Utilities/vtklibxml2
)

ADD_LIBRARY(faIO ${ioSrc})