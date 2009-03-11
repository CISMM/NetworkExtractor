#ifndef _LOAD_VTK_IMAGE_FILE_H_
#define _LOAD_VTK_IMAGE_FILE_H_

#include <string>

#include <itkVTKImageIO.h>
#include <itkImageFileReader.h>
#include <itkDefaultConvertPixelTraits.h>

template <class TOutputImage,
          class ConvertPixelTraits=itk::DefaultConvertPixelTraits< 
                   ITK_TYPENAME TOutputImage::IOPixelType > >
class LoadVTKImageFile {

  typedef itk::ImageFileReader<typename TOutputImage> ImageReaderType;

public:
  LoadVTKImageFile();
  virtual ~LoadVTKImageFile();

  typename TOutputImage::Pointer LoadFile(std::string& fileName);

protected:
  typename ImageReaderType::Pointer imageReader;

};

#include "LoadVTKImageFile.cxx"

#endif // _LOAD_VTK_IMAGE_FILE_H_