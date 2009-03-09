#ifndef _TYPES_H_
#define _TYPES_H_

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkVTKImageExport.h"

typedef unsigned short UShortPixelType;
static const unsigned int     Dimension3 = 3;
typedef itk::Image<UShortPixelType, Dimension3> UShort3DImageType;
typedef itk::ImageFileReader<UShort3DImageType> UShort3DImageReaderType;
typedef itk::VTKImageExport<UShort3DImageType> UShort3DExporterType;

#endif // _TYPES_H_