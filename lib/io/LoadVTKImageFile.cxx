#ifndef _LOAD_VTK_IMAGE_FILE_CXX_
#define _LOAD_VTK_IMAGE_FILE_CXX_

#include "LoadVTKImageFile.h"

template <class TOutputImage, class ConvertPixelTraits>
LoadVTKImageFile<TOutputImage, ConvertPixelTraits>
::LoadVTKImageFile() {

  imageReader = ImageReaderType::New();
}


template <class TOutputImage, class ConvertPixelTraits>
LoadVTKImageFile<TOutputImage, ConvertPixelTraits>
::~LoadVTKImageFile() {
}


template <class TOutputImage, class ConvertPixelTraits>
typename TOutputImage::Pointer 
LoadVTKImageFile<TOutputImage, ConvertPixelTraits>
::LoadFile(std::string& fileName) {

  imageReader->SetFileName(fileName.c_str());
  imageReader->Update();
  return imageReader->GetOutput();
}

#endif // _LOAD_VTK_IMAGE_FILE_CXX_