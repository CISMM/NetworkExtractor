#ifndef _DATA_MODEL_H_
#define _DATA_MODEL_H_

#include <string>

#include <itkCommand.h>
#include <itkEigenValues3DToVesselnessMeasureImageFilter.h>
#include <itkEventObject.h>
#include <itkFixedArray.h>
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkHessian3DEigenAnalysisImageFilter.h>
#include <itkHessianRecursiveGaussianImageFilter.h>
#include <itkMatrix.h>
#include <itkMinimumMaximumImageCalculator.h>
#include <itkSymmetricSecondRankTensor.h>

class vtkAlgorithmOutput;

#include "ITKImageToVTKImage.h"

// This is the data model for the Fibrin Analysis library.
template <class TImage>
class DataModel {

  typedef void (*ProgressCallback)(float, const char *);
  typedef float ComponentType;
  typedef itk::ImageFileReader<TImage> FileReaderType;
  typedef itk::ImageFileWriter<TImage> FileWriterType;
  typedef itk::MinimumMaximumImageCalculator<TImage> MinMaxType;
  typedef itk::SymmetricSecondRankTensor<ComponentType, ::itk::GetImageDimension<TImage>::ImageDimension> HessianType;
  typedef itk::Image<HessianType, ::itk::GetImageDimension<TImage>::ImageDimension> HessianImageType;
  typedef itk::HessianRecursiveGaussianImageFilter<TImage, HessianImageType> HessianFilterType;

  typedef itk::FixedArray<ComponentType, 3> EigenValueType;
  typedef itk::Matrix<ComponentType, 3, 3>  EigenVectorType;
  typedef typename itk::Image<EigenValueType, 3>  EigenValueImageType;
  typedef typename itk::Image<EigenVectorType, 3> EigenVectorImageType;
  typedef itk::Hessian3DEigenAnalysisImageFilter<HessianImageType, EigenValueImageType, EigenVectorImageType> HessianEigenAnalysisFilterType;
  typedef itk::EigenValues3DToVesselnessMeasureImageFilter<EigenValueImageType, TImage> VesselnessFilterType;

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
  typename HessianEigenAnalysisFilterType::Pointer eigenAnalysisFilter;

  typename HessianFilterType::Pointer hessianFilter;
  typename VesselnessFilterType::Pointer vesselnessFilter;
  ITKImageToVTKImage<TImage>* itkToVtkFilter;

  void UpdateProgress(itk::Object* object, const itk::EventObject& event);

  ProgressCallback progressCallback;

};

#include "DataModel.cxx"

#endif // _DATA_MODEL_H_