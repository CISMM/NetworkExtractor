#ifndef _DATA_MODEL_H_
#define _DATA_MODEL_H_

#include <string>

#include <itkBinaryThresholdImageFilter.h>
#include <itkBinaryThinningImageFilter.h>
#include <itkCastImageFilter.h>
#include <itkCommand.h>
#include <itkConnectedComponentImageFilter.h>
#include <itkEigenValues3DToFrangiVesselnessMeasureImageFilter.h>
#include <itkEigenVectors3DToJunctionnessMeasureImageFilter.h>
#include <itkEventObject.h>
#include <itkFixedArray.h>
#include <itkHessian3DEigenAnalysisImageFilter.h>
#include <itkHessianRecursiveGaussianImageFilter.h>
#include <itkHessianToObjectnessMeasureImageFilter.h>
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkMatrix.h>
#include <itkMinimumMaximumImageCalculator.h>
#include <itkMultiScaleHessianBasedMeasureImageFilter2.h>
#include <itkShiftScaleImageFilter.h>
#include <itkSymmetricSecondRankTensor.h>
#include <itkValuedRegionalMaximaImageFilter.h>

#include <vtkAlgorithmOutput.h>

#include "ITKImageToVTKImage.h"
#include "Types.h"

// This is the data model for the Fibrin Analysis library.
class DataModel {

  typedef Float3DImageType TImage;
  typedef TImage InputImageType;

  typedef void (*ProgressCallback)(float, const char *);
  typedef float ComponentType;
  typedef itk::MinimumMaximumImageCalculator<TImage> MinMaxType;
  typedef itk::SymmetricSecondRankTensor<ComponentType, ::itk::GetImageDimension<TImage>::ImageDimension> HessianType;
  typedef itk::Image<HessianType, ::itk::GetImageDimension<TImage>::ImageDimension> HessianImageType;
  typedef itk::HessianRecursiveGaussianImageFilter<TImage, HessianImageType> HessianFilterType;

  typedef itk::FixedArray<ComponentType, 3>       EigenValueType;
  typedef itk::Vector<ComponentType, 3>           EigenVectorType;
  typedef itk::Image<EigenValueType, 3>  EigenValueImageType;
  typedef itk::Image<EigenVectorType, 3> EigenVectorImageType;
  typedef itk::MultiScaleHessianBasedMeasureImageFilter2<TImage, HessianImageType, EigenVectorImageType> MultiScaleHessianMeasureImageType;
  typedef itk::HessianToObjectnessMeasureImageFilter<HessianImageType, TImage> HessianToObjectnessFilterType;
  typedef itk::Hessian3DEigenAnalysisImageFilter<HessianImageType, EigenValueImageType, EigenVectorImageType> HessianEigenAnalysisFilterType;
  typedef itk::EigenValues3DToFrangiVesselnessMeasureImageFilter<EigenValueImageType, TImage> FibernessFilterType;
  typedef itk::ShiftScaleImageFilter<TImage, TImage> ScaleFilterType;
  typedef itk::EigenVectors3DToJunctionnessImageFilter<EigenVectorImageType, TImage> JunctionnessFilterType;
  typedef itk::ValuedRegionalMaximaImageFilter<TImage, TImage> JunctionnessLocalMaxFilterType;
  typedef itk::BinaryThresholdImageFilter<TImage, TImage> ThresholdFilterType;
  typedef itk::BinaryThinningImageFilter<TImage, TImage> SkeletonizationFilterType;
  typedef SkeletonizationFilterType::Pointer SkeletonizationFilterTypePointer;

  typedef int ConnectedComponentOutputType;
  typedef itk::Image<ConnectedComponentOutputType, 3> ConnectedComponentOutputImageType;
  typedef itk::ConnectedComponentImageFilter<TImage, ConnectedComponentOutputImageType> ConnectedComponentFilterType;
  typedef itk::MinimumMaximumImageCalculator<ConnectedComponentOutputImageType> MinMaxConnectedComponentFilterType;

  typedef itk::ImageFileReader<TImage> ScalarFileReaderType;
  typedef itk::ImageFileWriter<TImage> ScalarFileWriterType;
  typedef itk::ImageFileWriter<EigenVectorImageType> VectorFileWriterType;

  // For writing out to TIFF images.
  typedef itk::Image<unsigned short, 3> TIFFOutputImageType;
  typedef itk::CastImageFilter<TImage,TIFFOutputImageType> UShort3DCastType;
  typedef itk::ImageFileWriter<TIFFOutputImageType> TIFFWriterType;

public:
  typedef std::string FilterType;
  static const FilterType NO_FILTER_STRING;
  static const FilterType FRANGI_FIBERNESS_FILTER_STRING;
  static const FilterType MULTISCALE_FIBERNESS_FILTER_STRING;
  static const FilterType MULTISCALE_FIBERNESS_THRESHOLD_FILTER_STRING;
  static const FilterType MULTISCALE_SKELETONIZATION_FILTER_STRING;
  static const FilterType JUNCTIONNESS_FILTER_STRING;
  static const FilterType JUNCTIONNESS_LOCAL_MAX_FILTER_STRING;

  DataModel();
  virtual ~DataModel();

  void LoadImageFile(std::string fileName);
  void SaveFilteredImageFile(std::string fileName, std::string filterName, float scale);

  std::string GetImageFileName();

  void SetNumberOfThreads(int threads);
  int  GetNumberOfThreads();

  void SaveFiberOrientationImageFile(std::string fileName);

  void SaveFiberOrientationDataFile(std::string fileName);

  void SetFiberDiameter(double diameter);
  double GetFiberDiameter();

  void SetMultiscaleFibernessAlphaCoefficient(double alpha);
  double GetMultiscaleFibernessAlphaCoefficient();

  void SetMultiscaleFibernessBetaCoefficient(double beta);
  double GetMultiscaleFibernessBetaCoefficient();

  void SetMultiscaleFibernessGammaCoefficient(double gamma);
  double GetMultiscaleFibernessGammaCoefficient();

  void SetMultiscaleFibernessMinimumScale(double minimum);
  double GetMultiscaleFibernessMinimumScale();

  void SetMultiscaleFibernessMaximumScale(double maximum);
  double GetMultiscaleFibernessMaximumScale();

  void SetMultiscaleFibernessNumberOfScales(int numberOfScales);
  int GetMultiscaleFibernessNumberOfScales();

  void SetMultiscaleFibernessThreshold(double threshold);
  double GetMultiscaleFibernessThreshold();

  void SetJunctionProbeDiameter(double diameter);
  double GetJunctionProbeDiameter();

  void SetJunctionFibernessThreshold(double threshold);
  double GetJunctionFibernessThreshold();

  void SetJunctionnessLocalMaxHeight(double height);
  double GetJunctionnessLocalMaxHeight();

  void SetImageData(TImage::Pointer image);
  TImage::Pointer GetImageData();

  // Returns the VTK output port for the original scalar image data.
  vtkAlgorithmOutput* GetImageOutputPort();

  // Returns the VTK output port for the filtered scalar image data.
  vtkAlgorithmOutput* GetFilteredImageOutputPort();

  // Returns the VTK output port for the vector image data from the
  // eigen vectors oriented along the cylinder axes of the fibers.
  vtkAlgorithmOutput* GetVectorOutputPort();

  double GetFilteredDataMinimum();
  double GetFilteredDataMaximum();

  void GetDimensions(int dimensions[3]);

  void SetFilterToNone();
  void SetFilterToFrangiFiberness();
  void SetFilterToMultiscaleFiberness();
  void SetFilterToMultiscaleFibernessThreshold();
  void SetFilterToMultiscaleSkeletonization();
  void SetFilterToJunctionness();
  void SetFilterToJunctionnessLocalMax();

  void SetVoxelSpacing(double spacing[3]);
  void SetVoxelSpacing(int dimension, double spacing);
  void SetVoxelXSpacing(double spacing);
  void SetVoxelYSpacing(double spacing);
  void SetVoxelZSpacing(double spacing);
  void GetVoxelSpacing(double spacing[3]);

  void SetFilteredImageScaleFactor(double scale);
  double GetFilteredImageScaleFactor();

  void ComputeConnectedComponentsVsThresholdData(double minThreshold, double maxThreshold,
    double thresholdIncrement, std::string fileName);
  void ComputeVolumeFractionEstimateVsZData(double threshold, std::string fileName);

  void SetProgressCallback(ProgressCallback callback);

protected:
  std::string imageFileName;
  double fiberDiameter;
  double filteredImageScale;
  TImage::Pointer imageData;

  MinMaxType::Pointer minMaxFilter;
  HessianEigenAnalysisFilterType::Pointer eigenAnalysisFilter;

  HessianFilterType::Pointer hessianFilter;
  FibernessFilterType::Pointer fibernessFilter;
  HessianToObjectnessFilterType::Pointer hessianToVesselnessFilter;
  MultiScaleHessianMeasureImageType::Pointer multiscaleFibernessFilter;
  ThresholdFilterType::Pointer multiscaleFibernessThresholdFilter;
  SkeletonizationFilterTypePointer skeletonizationFilter;
  ConnectedComponentFilterType::Pointer fibernessConnectedComponentsFilter;
  MinMaxConnectedComponentFilterType::Pointer minMaxConnectedComponentsFilter;
  JunctionnessFilterType::Pointer junctionnessFilter;
  JunctionnessLocalMaxFilterType::Pointer junctionnessLocalMaxFilter;

  ITKImageToVTKImage<TImage>* inputImageITKToVTKFilter;
  ITKImageToVTKImage<TImage>* filteredImageITKToVTKFilter;
  ITKImageToVTKImage<EigenVectorImageType>* vectorImageITKToVTKFilter;

  void UpdateProgress(itk::Object* object, const itk::EventObject& event);

  ProgressCallback progressCallback;

};

// _DATA_MODEL_H_
#endif
