#ifndef _LOAD_VTK_IMAGE_FILE_H_
#define _LOAD_VTK_IMAGE_FILE_H_

#include <string>

#include <itkVTKImageIO.h>
#include <itkImageFileReader.h>

#include "Types.h"

class LoadVTKImageFile {

public:
  LoadVTKImageFile();
  virtual ~LoadVTKImageFile();

  UShort3DImageType::Pointer LoadFile(std::string& fileName);

protected:
  UShort3DImageReaderType::Pointer imageReader;

};

#endif // _LOAD_VTK_IMAGE_FILE_H_