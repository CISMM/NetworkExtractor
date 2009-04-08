#ifndef _DATA_MODEL_H_
#define _DATA_MODEL_H_

#include <string>

#include <itkCommand.h>
#include <itkEigenValues3DToVesselnessMeasureImageFilter.h>
#include <itkEigenVectors3DToJunctionnessMeasureImageFilter.h>
#include <itkEventObject.h>
#include <itkFixedArray.h>
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkValuedRegionalMaximaImageFilter.h>
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
  typedef itk::MinimumMaximumImageCalculator<TImage> MinMaxType;
  typedef itk::SymmetricSecondRankTensor<ComponentType, ::itk::GetImageDimension<TImage>::ImageDimension> HessianType;
  typedef itk::Image<HessianType, ::itk::GetImageDimension<TImage>::ImageDimension> HessianImageType;
  typedef itk::HessianRecursiveGaussianImageFilter<TImage, HessianImageType> HessianFilterType;

  typedef itk::FixedArray<ComponentType, 3>       EigenValueType;
  typedef itk::Vector<ComponentType, 3>           EigenVectorType;
  typedef typename itk::Image<EigenValueType, 3>  EigenValueImageType;
  typedef typename itk::Image<EigenVectorType, 3> EigenVectorImageType;
  typedef itk::Hessian3DEigenAnalysisImageFilter<HessianImageType, EigenValueImageType, EigenVectorImageType> HessianEigenAnalysisFilterType;
  typedef itk::EigenValues3DToVesselnessMeasureImageFilter<EigenValueImageType, TImage> VesselnessFilterType;
  typedef itk::EigenVectors3DToJunctionnessImageFilter<EigenVectorImageType, TImage> JunctionnessFilterType;
  typedef itk::ValuedRegionalMaximaImageFilter<TImage, TImage> JunctionnessLocalMaxFilterType;

  typedef itk::ImageFileReader<TImage> ScalarFileReaderType;
  typedef itk::ImageFileWriter<TImage> ScalarFileWriterType;
  typedef itk::ImageFileWriter<EigenVectorImageType> VectorFileWriterType;

public:
  typedef std::string FilterType;
  static const FilterType NO_FILTER_STRING;
  static const FilterType VESSELNESS_FILTER_STRING;
  static const FilterType JUNCTIONNESS_FILTER_STRING;
  static const FilterType JUNCTIONNESS_LOCAL_MAX_FILTER_STRING;

  DataModel();
  virtual ~DataModel();

  void LoadImageFile(std::string fileName);
  void SaveImageFile(std::string fileName, std::string filterName);

  void SaveFiberOrientationImageFile(std::string fileName);

  void SetFiberDiameter(double diameter);
  double GetFiberDiameter();

  void SetJunctionProbeDiameter(double diameter);
  double GetJunctionProbeDiameter();

  void SetJunctionVesselnessThreshold(double threshold);
  double GetJunctionVesselnessThreshold();

  void SetJunctionnessLocalMaxHeight(double height);
  double GetJunctionnessLocalMaxHeight();

  void SetImageData(typename TImage::Pointer image);
  typename TImage::Pointer GetImageData();

  // Returns the VTK output port for the scalar image data.
  vtkAlgorithmOutput* GetOutputPort();

  // Returns the VTK output port for the vector image data from the
  // eigen vectors oriented along the cylinder axes of the fibers.
  vtkAlgorithmOutput* GetVectorOutputPort();

  double GetFilteredDataMinimum();
  double GetFilteredDataMaximum();

  void GetDimensions(int dimensions[3]);

  void GetSpacing(double spacing[3]);

  void SetFilterToNone();
  void SetFilterToVesselness();
  void SetFilterToJunctionness();
  void SetFilterToJunctionnessLocalMax();

  void SetProgressCallback(ProgressCallback callback);

protected:
  double fiberDiameter;
  typename TImage::Pointer imageData;

  typename MinMaxType::Pointer minMaxFilter;
  typename HessianEigenAnalysisFilterType::Pointer eigenAnalysisFilter;

  typename HessianFilterType::Pointer hessianFilter;
  typename VesselnessFilterType::Pointer vesselnessFilter;
  typename JunctionnessFilterType::Pointer junctionnessFilter;
  typename JunctionnessLocalMaxFilterType::Pointer junctionnessLocalMaxFilter;
  ITKImageToVTKImage<TImage>* scalarImageITKToVTKFilter;
  ITKImageToVTKImage<EigenVectorImageType>* vectorImageITKToVTKFilter;

  void UpdateProgress(itk::Object* object, const itk::EventObject& event);

  ProgressCallback progressCallback;

};

#include "DataModel.cxx"

#endif // _DATA_MODEL_H_