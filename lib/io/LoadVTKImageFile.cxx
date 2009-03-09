#include "LoadVTKImageFile.h"


LoadVTKImageFile::LoadVTKImageFile() {
  imageReader = UShort3DImageReaderType::New();
}


LoadVTKImageFile::~LoadVTKImageFile() {
}


UShort3DImageType::Pointer LoadVTKImageFile::LoadFile(std::string& fileName) {
  imageReader->SetFileName(fileName.c_str());
  imageReader->Update();
  return imageReader->GetOutput();
}
