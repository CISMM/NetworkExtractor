#ifndef _TYPES_H_
#define _TYPES_H_

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkVTKImageExport.h"

static const unsigned int     Dimension3 = 3;

typedef unsigned short UShortPixelType;
typedef itk::Image<UShortPixelType, Dimension3> UShort3DImageType;
typedef itk::ImageFileReader<UShort3DImageType> UShort3DImageReaderType;
typedef itk::VTKImageExport<UShort3DImageType> UShort3DExporterType;

typedef float FloatPixelType;
typedef itk::Image<FloatPixelType, Dimension3> Float3DImageType;
typedef itk::ImageFileReader<Float3DImageType> Float3DImageReaderType;
typedef itk::VTKImageExport<Float3DImageType> Float3DExporterType;

#endif // _TYPES_H_