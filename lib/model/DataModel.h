#ifndef _DATA_MODEL_H_
#define _DATA_MODEL_H_

#include <string>

#include <itkCommand.h>
#include <itkEventObject.h>
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkHessianRecursiveGaussianImageFilter.h>
#include <itkHessian3DToVesselnessMeasureImageFilter.h>
#include <itkMinimumMaximumImageCalculator.h>
#include <itkSymmetricSecondRankTensor.h>

class vtkAlgorithmOutput;

#include "ITKImageToVTKImage.h"

// This is the data model for the Fibrin Analysis library.
template <class TImage>
class DataModel {

  typedef void (*ProgressCallback)(float);
  typedef itk::ImageFileReader<TImage> FileReaderType;
  typedef itk::ImageFileWriter<TImage> FileWriterType;
  typedef itk::MinimumMaximumImageCalculator<TImage> MinMaxType;
  typedef itk::HessianRecursiveGaussianImageFilter<TImage> HessianFilterType;
  typedef itk::Hessian3DToVesselnessMeasureImageFilter<typename TImage::PixelType> VesselnessFilterType;
  
public:
  DataModel();
  virtual ~DataModel();

  void LoadImageFile(std::string fileName);
  void SaveImageFile(std::string fileName);

  void SetFiberDiameter(double radius);
  double GetFiberDiameter();

  void SetImageData(typename TImage::Pointer image);
  typename TImage::Pointer GetImageData();

  // Returns the VTK output port for the image data.
  vtkAlgorithmOutput* GetOutputPort();

  double GetFilteredDataMinimum();
  double GetFilteredDataMaximum();

  void GetDimensions(int dimensions[3]);

  void GetSpacing(double spacing[3]);

  void SetFilterToNone();
  void SetFilterToVesselness();

  void SetProgressCallback(ProgressCallback callback);

protected:
  double fiberDiameter;
  typename TImage::Pointer imageData;

  typename MinMaxType::Pointer minMaxFilter;
  typename HessianFilterType::Pointer hessianFilter;
  typename VesselnessFilterType::Pointer vesselnessFilter;
  ITKImageToVTKImage<TImage>* itkToVtkFilter;

  void UpdateProgress(itk::Object* object, const itk::EventObject& event);

  ProgressCallback progressCallback;

};

#include "DataModel.cxx"

#endif // _DATA_MODEL_H_