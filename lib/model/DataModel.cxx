#ifndef _DATA_MODEL_CXX_
#define _DATA_MODEL_CXX_

#if defined(_WIN32) // Turn off deprecation warnings in Visual Studio
#pragma warning( disable : 4996 )
#endif

#include "DataModel.h"
#include "Types.h"

#include <itkAccumulateImageFilter.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkMetaDataObject.h>
#include <itkMultiThreader.h>
#include <itkPoint.h>

// Constants
template <class TImage>
const typename DataModel<TImage>::FilterType 
DataModel<TImage>::NO_FILTER_STRING("No filter");

template <class TImage>
const typename DataModel<TImage>::FilterType
DataModel<TImage>::FRANGI_FIBERNESS_FILTER_STRING("Frangi Fiberness Filter");

template <class TImage>
const typename DataModel<TImage>::FilterType
DataModel<TImage>::MULTISCALE_FIBERNESS_FILTER_STRING("Multiscale Fiberness");

template <class TImage>
const typename DataModel<TImage>::FilterType
DataModel<TImage>::MULTISCALE_FIBERNESS_THRESHOLD_FILTER_STRING("Multiscale Fiberness Threshold");

template <class TImage>
const typename DataModel<TImage>::FilterType
DataModel<TImage>::MULTISCALE_SKELETONIZATION_FILTER_STRING("Skeletonization");

template <class TImage>
const typename DataModel<TImage>::FilterType
DataModel<TImage>::JUNCTIONNESS_FILTER_STRING("Junctionness");

template <class TImage>
const typename DataModel<TImage>::FilterType
DataModel<TImage>::JUNCTIONNESS_LOCAL_MAX_FILTER_STRING("Junctionness Local Maxima");


template <class TImage>
DataModel<TImage>
::DataModel() {
  this->fiberDiameter = 1.0f;
  this->filteredImageScale = 1.0;
  this->imageData = NULL;

  // Fiberness filters.
  this->hessianFilter = HessianFilterType::New();

  this->eigenAnalysisFilter = HessianEigenAnalysisFilterType::New();
  this->eigenAnalysisFilter->SetInput(this->hessianFilter->GetOutput());
  
  this->fibernessFilter = FibernessFilterType::New();
  this->fibernessFilter->SetInput(this->eigenAnalysisFilter->GetEigenValues());

  this->hessianToVesselnessFilter = HessianToObjectnessFilterType::New();
  this->hessianToVesselnessFilter->SetObjectDimension(1);
  this->hessianToVesselnessFilter->ScaleObjectnessMeasureOff();
  this->hessianToVesselnessFilter->SetBrightObject(true);

  this->multiscaleFibernessFilter = MultiScaleHessianMeasureImageType::New();
  this->multiscaleFibernessFilter->GenerateHessianOutputOff();
  this->multiscaleFibernessFilter->GenerateScalesOutputOff();
  this->multiscaleFibernessFilter->GenerateEigenvectorOutputOn();
  this->multiscaleFibernessFilter->SetSigmaMinimum(0.5);
  this->multiscaleFibernessFilter->SetSigmaMaximum(1.5);
  this->multiscaleFibernessFilter->SetSigmaStepMethodToEquispaced();
  this->multiscaleFibernessFilter->SetNumberOfSigmaSteps(11);
  this->multiscaleFibernessFilter->SetHessianToMeasureFilter(this->hessianToVesselnessFilter);

  this->minMaxFilter = MinMaxType::New();

  this->multiscaleFibernessThresholdFilter = ThresholdFilterType::New();
  this->multiscaleFibernessThresholdFilter->SetInput(this->multiscaleFibernessFilter->GetOutput());
  this->multiscaleFibernessThresholdFilter->SetInsideValue(0.0);
  this->multiscaleFibernessThresholdFilter->SetOutsideValue(1.0);
  this->multiscaleFibernessThresholdFilter->SetLowerThreshold(0.0);
  this->multiscaleFibernessThresholdFilter->SetUpperThreshold(0.0);

  this->skeletonizationFilter = SkeletonizationFilterType::New();
  this->skeletonizationFilter->SetInput(this->multiscaleFibernessThresholdFilter->GetOutput());

  this->fibernessConnectedComponentsFilter = ConnectedComponentFilterType::New();
  this->fibernessConnectedComponentsFilter->SetInput(this->multiscaleFibernessThresholdFilter->GetOutput());
  this->fibernessConnectedComponentsFilter->FullyConnectedOn();

  this->minMaxConnectedComponentsFilter = MinMaxConnectedComponentFilterType::New();

  this->junctionnessFilter = JunctionnessFilterType::New();
  this->junctionnessFilter->SetEigenVectorInput(this->eigenAnalysisFilter->GetEigenVectors());
  this->junctionnessFilter->SetVesselnessInput(this->fibernessFilter->GetOutput());

  this->junctionnessLocalMaxFilter = JunctionnessLocalMaxFilterType::New();
  this->junctionnessLocalMaxFilter->SetInput(this->junctionnessFilter->GetOutput());
  this->junctionnessLocalMaxFilter->FullyConnectedOn();

  this->inputImageITKToVTKFilter = new ITKImageToVTKImage<TImage>();
  this->filteredImageITKToVTKFilter = new ITKImageToVTKImage<TImage>();
  this->vectorImageITKToVTKFilter = new ITKImageToVTKImage<EigenVectorImageType>();
  this->progressCallback = NULL;

    // Hook up progress reporter for the filters.
  typename itk::MemberCommand< DataModel< TImage > >::Pointer hessianFilterProgressCommand 
    = itk::MemberCommand< DataModel< TImage > >::New();

  typename itk::MemberCommand< DataModel< TImage > >::Pointer eigenAnalysisFilterProgressCommand 
    = itk::MemberCommand< DataModel< TImage > >::New();

  typename itk::MemberCommand< DataModel< TImage > >::Pointer fibernessFilterProgressCommand 
    = itk::MemberCommand< DataModel< TImage > >::New();

  typename itk::MemberCommand< DataModel< TImage > >::Pointer multiscaleFibernessFilterProgressCommand 
    = itk::MemberCommand< DataModel< TImage > >::New();

  typename itk::MemberCommand< DataModel< TImage > >::Pointer fibernessThresholdFilterProgressCommand
    = itk::MemberCommand< DataModel< TImage > >::New();

  typename itk::MemberCommand< DataModel< TImage > >::Pointer skeletonizationFilterProgressCommand
    = itk::MemberCommand< DataModel< TImage > >::New();

  typename itk::MemberCommand< DataModel< TImage > >::Pointer fibernessConnectedComponentsFilterProgressCommand
    = itk::MemberCommand< DataModel< TImage > >::New();

  typename itk::MemberCommand< DataModel< TImage > >::Pointer junctionnessFilterProgressCommand 
    = itk::MemberCommand< DataModel< TImage > >::New();

  typename itk::MemberCommand< DataModel< TImage > >::Pointer junctionnessLocalMaxFilterProgressCommand
    = itk::MemberCommand< DataModel< TImage > >::New();

  // Set the callback function for each of the progress reporters.
  hessianFilterProgressCommand->SetCallbackFunction(this, &DataModel<TImage>::UpdateProgress);
  eigenAnalysisFilterProgressCommand->SetCallbackFunction(this, &DataModel<TImage>::UpdateProgress);
  fibernessFilterProgressCommand->SetCallbackFunction(this, &DataModel<TImage>::UpdateProgress);
  multiscaleFibernessFilterProgressCommand->SetCallbackFunction(this, &DataModel<TImage>::UpdateProgress);
  fibernessThresholdFilterProgressCommand->SetCallbackFunction(this, &DataModel<TImage>::UpdateProgress);
  skeletonizationFilterProgressCommand->SetCallbackFunction(this, &DataModel<TImage>::UpdateProgress);
  fibernessConnectedComponentsFilterProgressCommand->SetCallbackFunction(this, &DataModel<TImage>::UpdateProgress);
  junctionnessFilterProgressCommand->SetCallbackFunction(this, &DataModel<TImage>::UpdateProgress);
  junctionnessLocalMaxFilterProgressCommand->SetCallbackFunction(this, &DataModel<TImage>::UpdateProgress);

  // Attach a MetaDataObject with the name of the filter to each rilter.
  itk::EncapsulateMetaData<std::string >(hessianFilter->GetMetaDataDictionary(),
    "filterName", std::string("Hessian Filter"));
  itk::EncapsulateMetaData<std::string >(eigenAnalysisFilter->GetMetaDataDictionary(),
    "filterName", std::string("Eigen Analysis Filter"));
  itk::EncapsulateMetaData<std::string >(fibernessFilter->GetMetaDataDictionary(),
    "filterName", std::string("Fiberness Filter"));
  itk::EncapsulateMetaData<std::string >(multiscaleFibernessFilter->GetMetaDataDictionary(),
    "filterName", std::string("Multiscale Fiberness Filter"));
  itk::EncapsulateMetaData<std::string >(multiscaleFibernessThresholdFilter->GetMetaDataDictionary(),
    "filterName", std::string("Fiberness Threshold Filter"));
  itk::EncapsulateMetaData<std::string >(skeletonizationFilter->GetMetaDataDictionary(),
    "filterName", std::string("Skeletonization Filter"));
  itk::EncapsulateMetaData<std::string >(fibernessConnectedComponentsFilter->GetMetaDataDictionary(),
    "filterName", std::string("Fiberness Connected Components Filter"));
  itk::EncapsulateMetaData<std::string >(junctionnessFilter->GetMetaDataDictionary(),
    "filterName", std::string("Junctionness Filter"));
  itk::EncapsulateMetaData<std::string>(junctionnessLocalMaxFilter->GetMetaDataDictionary(),
    "filterName", std::string("Junctionness Local Max Filter"));

  this->hessianFilter->AddObserver(itk::ProgressEvent(), hessianFilterProgressCommand);
  this->eigenAnalysisFilter->AddObserver(itk::ProgressEvent(), eigenAnalysisFilterProgressCommand);
  this->fibernessFilter->AddObserver(itk::ProgressEvent(), fibernessFilterProgressCommand);
  this->multiscaleFibernessFilter->AddObserver(itk::ProgressEvent(), multiscaleFibernessFilterProgressCommand);
  this->multiscaleFibernessThresholdFilter->AddObserver(itk::ProgressEvent(), fibernessThresholdFilterProgressCommand);
  this->skeletonizationFilter->AddObserver(itk::ProgressEvent(), skeletonizationFilterProgressCommand);
  this->fibernessConnectedComponentsFilter->AddObserver(itk::ProgressEvent(), fibernessConnectedComponentsFilterProgressCommand);
  this->junctionnessFilter->AddObserver(itk::ProgressEvent(), junctionnessFilterProgressCommand);
  this->junctionnessLocalMaxFilter->AddObserver(itk::ProgressEvent(), junctionnessLocalMaxFilterProgressCommand);

  itk::MultiThreader::SetGlobalMaximumNumberOfThreads(2);
}


template <class TImage>
DataModel<TImage>
::~DataModel() {
  delete this->inputImageITKToVTKFilter;
  delete this->filteredImageITKToVTKFilter;
  delete this->vectorImageITKToVTKFilter;
}


template <class TImage>
void
DataModel<TImage>
::LoadImageFile(std::string fileName) {
  typename ScalarFileReaderType::Pointer reader = ScalarFileReaderType::New();
  reader->SetFileName(fileName.c_str());
  reader->Update();
  this->SetImageData(reader->GetOutput());

  // Connect this image data to the various pipelines.
  this->minMaxFilter->SetImage(this->imageData);
  this->minMaxFilter->Compute();
}


template <class TImage>
void
DataModel<TImage>
::SaveFilteredImageFile(std::string fileName, std::string filterName, float scale) {
  // Check for TIFF output and perform casting as needed.
  size_t tiffLocation = fileName.rfind(".tif");
  if (tiffLocation == fileName.size()-4) {
    typename ScaleFilterType::Pointer scaler = ScaleFilterType::New();
    scaler->SetShift(0.0f);
    scaler->SetScale(scale);

    if (filterName == NO_FILTER_STRING) {
      scaler->SetInput(this->imageData);
    } else if (filterName == FRANGI_FIBERNESS_FILTER_STRING) {
      scaler->SetInput(this->fibernessFilter->GetOutput());
    } else if (filterName == MULTISCALE_FIBERNESS_FILTER_STRING) {
      scaler->SetInput(this->multiscaleFibernessFilter->GetOutput());
    } else if (filterName == MULTISCALE_FIBERNESS_THRESHOLD_FILTER_STRING) {
      scaler->SetInput(this->multiscaleFibernessThresholdFilter->GetOutput());
    } else if (filterName == MULTISCALE_SKELETONIZATION_FILTER_STRING) {
      scaler->SetInput(this->skeletonizationFilter->GetOutput());
    } else if (filterName == JUNCTIONNESS_FILTER_STRING) {
      scaler->SetInput(this->junctionnessFilter->GetOutput());
    } else if (filterName == JUNCTIONNESS_LOCAL_MAX_FILTER_STRING) {
      scaler->SetInput(this->junctionnessLocalMaxFilter->GetOutput());
    } else {
      return;
    }

    typename UShort3DCastType::Pointer caster = UShort3DCastType::New();
    caster->SetInput(scaler->GetOutput());

    typename TIFFWriterType::Pointer writer = TIFFWriterType::New();
    writer->SetInput(caster->GetOutput());
    writer->SetFileName(fileName.c_str());
    writer->Update();

  } else { // Nothing special to do for VTK output files.

    typename ScalarFileWriterType::Pointer writer = ScalarFileWriterType::New();
    if (filterName == NO_FILTER_STRING)
      writer->SetInput(this->imageData);
    else if (filterName == FRANGI_FIBERNESS_FILTER_STRING)
      writer->SetInput(this->fibernessFilter->GetOutput());
    else if (filterName == MULTISCALE_FIBERNESS_FILTER_STRING)
      writer->SetInput(this->multiscaleFibernessFilter->GetOutput());
    else if (filterName == MULTISCALE_FIBERNESS_THRESHOLD_FILTER_STRING)
      writer->SetInput(this->multiscaleFibernessThresholdFilter->GetOutput());
    else if (filterName == MULTISCALE_SKELETONIZATION_FILTER_STRING)
      writer->SetInput(this->skeletonizationFilter->GetOutput());
    else if (filterName == JUNCTIONNESS_FILTER_STRING)
      writer->SetInput(this->junctionnessFilter->GetOutput());
    else if (filterName == JUNCTIONNESS_LOCAL_MAX_FILTER_STRING)
      writer->SetInput(this->junctionnessLocalMaxFilter->GetOutput());
    else
      return;

    writer->SetFileName(fileName.c_str());
    writer->Update();
  }
}


template <class TImage>
void
DataModel<TImage>
::SaveFiberOrientationImageFile(std::string fileName) {
  typename VectorFileWriterType::Pointer writer = VectorFileWriterType::New();
  writer->SetInput(this->multiscaleFibernessFilter->GetEigenvectorOutput());
  writer->SetFileName(fileName.c_str());
  writer->Update();
  writer->Write();
}


template <class TImage>
void
DataModel<TImage>
::SaveFiberOrientationDataFile(std::string fileName) {
  typename InputImageType::Pointer skeletonImage = this->skeletonizationFilter->GetOutput();
  itk::ImageRegionIteratorWithIndex<InputImageType> sit(skeletonImage,skeletonImage->GetLargestPossibleRegion());

  const EigenVectorImageType* eigenvectorImage = this->multiscaleFibernessFilter->GetEigenvectorOutput();
  itk::ImageRegionConstIteratorWithIndex<EigenVectorImageType> eit(eigenvectorImage,eigenvectorImage->GetLargestPossibleRegion());

  sit.GoToBegin();
  eit.GoToBegin();

  // Open file.
  FILE *fp = fopen(fileName.c_str(), "w");
  fprintf(fp, "xPosition,yPosition,zPosition,xDirection,yDirection,zDirection\n");

  while(!sit.IsAtEnd()) {
    if (sit.Value() > 0.0) {
      itk::Index<3> index = eit.GetIndex();
      itk::Point<float, 3> point;
      imageData->TransformIndexToPhysicalPoint(index, point);
      EigenVectorType eigenvector = eit.Value();
      fprintf(fp, "%f,%f,%f,%f,%f,%f\n", point[0], point[1], point[2], eigenvector[0], eigenvector[1], eigenvector[2]);

    }

    ++sit;
    ++eit;
  }

  fclose(fp);
}


template <class TImage>
void
DataModel<TImage>
::SetFiberDiameter(double diameter) {
  if (diameter != this->fiberDiameter) {
    this->fiberDiameter = diameter;

    // Set sigma on the Hessian filter.
    this->hessianFilter->SetSigma(0.5*diameter);
  }
}


template <class TImage>
void
DataModel<TImage>
::SetMultiscaleFibernessAlphaCoefficient(double alpha) {
  if (alpha != this->fibernessFilter->GetAlpha())
    this->fibernessFilter->SetAlpha(alpha);
  if (alpha != this->hessianToVesselnessFilter->GetAlpha()) {
    this->hessianToVesselnessFilter->SetAlpha(alpha);
    this->multiscaleFibernessFilter->Modified();
  }
}


template <class TImage>
double
DataModel<TImage>
::GetMultiscaleFibernessAlphaCoefficient() {
  return this->fibernessFilter->GetAlpha();
}


template <class TImage>
void
DataModel<TImage>
::SetMultiscaleFibernessBetaCoefficient(double beta) {
  if (beta != this->fibernessFilter->GetBeta())
    this->fibernessFilter->SetBeta(beta);
  if (beta != this->hessianToVesselnessFilter->GetBeta()) {
    this->hessianToVesselnessFilter->SetBeta(beta);
    this->multiscaleFibernessFilter->Modified();
  }
}


template <class TImage>
double
DataModel<TImage>
::GetMultiscaleFibernessBetaCoefficient() {
  return this->fibernessFilter->GetBeta();
}


template <class TImage>
void
DataModel<TImage>
::SetMultiscaleFibernessGammaCoefficient(double gamma) {
  if (gamma != this->fibernessFilter->GetGamma())
    this->fibernessFilter->SetGamma(gamma);
  if (gamma != this->hessianToVesselnessFilter->GetGamma()) {
    this->hessianToVesselnessFilter->SetGamma(gamma);
    this->multiscaleFibernessFilter->Modified();
  }
}


template <class TImage>
double
DataModel<TImage>
::GetMultiscaleFibernessGammaCoefficient() {
  return this->fibernessFilter->GetGamma();
}


template <class TImage>
void
DataModel<TImage>
::SetMultiscaleFibernessMinimumScale(double minimum) {
  if (minimum != this->multiscaleFibernessFilter->GetSigmaMinimum()) {
    this->multiscaleFibernessFilter->SetSigmaMinimum(minimum);
  }
}


template <class TImage>
double
DataModel<TImage>
::GetMultiscaleFibernessMinimumScale() {
  return this->multiscaleFibernessFilter->GetSigmaMinimum();
}


template <class TImage>
void
DataModel<TImage>
::SetMultiscaleFibernessMaximumScale(double maximum) {
  if (maximum != this->multiscaleFibernessFilter->GetSigmaMaximum()) {
    this->multiscaleFibernessFilter->SetSigmaMaximum(maximum);
  }
}


template <class TImage>
double
DataModel<TImage>
::GetMultiscaleFibernessMaximumScale() {
  return this->multiscaleFibernessFilter->GetSigmaMaximum();
}


template <class TImage>
void
DataModel<TImage>
::SetMultiscaleFibernessNumberOfScales(int numberOfScales) {
  this->multiscaleFibernessFilter->SetNumberOfSigmaSteps(numberOfScales);
}


template <class TImage>
int
DataModel<TImage>
::GetMultiscaleFibernessNumberOfScales() {
  return this->multiscaleFibernessFilter->GetNumberOfSigmaSteps();
}


template <class TImage>
void
DataModel<TImage>
::SetMultiscaleFibernessThreshold(double threshold) {
  this->multiscaleFibernessThresholdFilter->SetLowerThreshold(0.0);
  this->multiscaleFibernessThresholdFilter->SetUpperThreshold(threshold);
}


template <class TImage>
double
DataModel<TImage>
::GetMultiscaleFibernessThreshold() {
  return this->multiscaleFibernessThresholdFilter->GetUpperThreshold();
}


template <class TImage>
double
DataModel<TImage>
::GetFiberDiameter() {
  return this->fiberDiameter;
}


template <class TImage>
void
DataModel<TImage>
::SetJunctionProbeDiameter(double diameter) {
  // Set probe radius on the junctionness filter.
  if (0.5*diameter != this->junctionnessFilter->GetRadius()) {
    this->junctionnessFilter->SetRadius(0.5*diameter);
  }
}


template <class TImage>
double
DataModel<TImage>
::GetJunctionProbeDiameter() {
  return 2.0*this->junctionnessFilter->GetRadius();
}


template <class TImage>
void
DataModel<TImage>
::SetJunctionFibernessThreshold(double threshold) {
  if (threshold != this->junctionnessFilter->GetVesselnessThreshold()) {
    this->junctionnessFilter->SetVesselnessThreshold(threshold);
  }
}


template <class TImage>
double
DataModel<TImage>
::GetJunctionFibernessThreshold() {
  return this->junctionnessFilter->GetVesselnessThreshold();
}


// Deprecated method
template <class TImage>
void
DataModel<TImage>
::SetJunctionnessLocalMaxHeight(double height) {
  //if (height != this->junctionnessLocalMaxFilter->GetHeight()) {
  //  this->junctionnessLocalMaxFilter->SetHeight(height);
  //}
}


// Deprecated method
template <class TImage>
double
DataModel<TImage>
::GetJunctionnessLocalMaxHeight() {
  //return this->junctionnessLocalMaxFilter->GetHeight();
  return 0;
}


template <class TImage>
void 
DataModel<TImage>
::SetImageData(typename TImage::Pointer image) {
  this->imageData = image;

  this->hessianFilter->SetInput(this->imageData);
  this->multiscaleFibernessFilter->SetInput(this->imageData);

  // Set image data.
  this->inputImageITKToVTKFilter->SetInput(this->imageData);

  // Set filtered image data to input image initially.
  this->filteredImageITKToVTKFilter->SetInput(this->imageData);
}


template <class TImage>
typename TImage::Pointer
DataModel<TImage>
::GetImageData() {
  return this->imageData;
}


template <class TImage>
vtkAlgorithmOutput*
DataModel<TImage>
::GetImageOutputPort() {
  return this->inputImageITKToVTKFilter->GetOutputPort();
}


template <class TImage>
vtkAlgorithmOutput*
DataModel<TImage>
::GetFilteredImageOutputPort() {
  return this->filteredImageITKToVTKFilter->GetOutputPort();
}


template <class TImage>
vtkAlgorithmOutput*
DataModel<TImage>
::GetVectorOutputPort() {
  return this->vectorImageITKToVTKFilter->GetOutputPort();
}


template <class TImage>
double
DataModel<TImage>
::GetFilteredDataMinimum() {
  return minMaxFilter->GetMinimum();
}


template <class TImage>
double
DataModel<TImage>
::GetFilteredDataMaximum() {
  return minMaxFilter->GetMaximum();
}


template <class TImage>
void
DataModel<TImage>
::GetDimensions(int dimensions[3]) {
  if (!this->GetImageData()) {
    dimensions[0] = 0;
    dimensions[1] = 0;
    dimensions[2] = 0;
    return;
  }

  typename UShort3DImageType::RegionType region 
      = this->GetImageData()->GetLargestPossibleRegion();
  itk::Size<3> size = region.GetSize();

  dimensions[0] = size[0];
  dimensions[1] = size[1];
  dimensions[2] = size[2];
}


template <class TImage>
void
DataModel<TImage>
::GetSpacing(double spacing[3]) {
  if (!this->GetImageData()) {
    spacing[0] = 0;
    spacing[1] = 0;
    spacing[2] = 0;
    return;
  }

  itk::Vector<double> thisSpacing = this->GetImageData()->GetSpacing();
  spacing[0] = thisSpacing[0];
  spacing[1] = thisSpacing[1];
  spacing[2] = thisSpacing[2];
}


template <class TImage>
void
DataModel<TImage>
::SetFilterToNone() {

  if (this->imageData) {
    this->minMaxFilter->SetImage(this->imageData);
    this->minMaxFilter->Compute();
    
    this->filteredImageITKToVTKFilter->SetInput(this->imageData);
  }
}


template <class TImage>
void
DataModel<TImage>
::SetFilterToFrangiFiberness() {

  if (!this->imageData)
    return;

  this->fibernessFilter->Update();
  this->minMaxFilter->SetImage(this->fibernessFilter->GetOutput());
  this->minMaxFilter->Compute();
  this->filteredImageITKToVTKFilter->SetInput(this->fibernessFilter->GetOutput());
}


template <class TImage>
void
DataModel<TImage>
::SetFilterToMultiscaleFiberness() {

  if (!this->imageData)
    return;

  this->multiscaleFibernessFilter->Update();
  this->minMaxFilter->SetImage(this->multiscaleFibernessFilter->GetOutput());
  this->minMaxFilter->Compute();
  this->filteredImageITKToVTKFilter->SetInput(this->multiscaleFibernessFilter->GetOutput());
}


template <class TImage>
void
DataModel<TImage>
::SetFilterToMultiscaleFibernessThreshold() {

  if (!this->imageData)
    return;

  this->multiscaleFibernessThresholdFilter->Update();
  this->minMaxFilter->SetImage(this->multiscaleFibernessThresholdFilter->GetOutput());
  this->minMaxFilter->Compute();
  this->filteredImageITKToVTKFilter->SetInput(this->multiscaleFibernessThresholdFilter->GetOutput());
}


template <class TImage>
void
DataModel<TImage>
::SetFilterToMultiscaleSkeletonization() {

  if (!this->imageData)
    return;

  this->skeletonizationFilter->Update();
  this->filteredImageITKToVTKFilter->SetInput(this->skeletonizationFilter->GetOutput());
}


template <class TImage>
void
DataModel<TImage>
::SetFilterToJunctionness() {

  if (!this->imageData)
    return;

  this->junctionnessFilter->Update();
  this->minMaxFilter->SetImage(this->junctionnessFilter->GetOutput());
  this->minMaxFilter->Compute();
  this->filteredImageITKToVTKFilter->SetInput(this->junctionnessFilter->GetOutput());
  this->vectorImageITKToVTKFilter->SetInput(
    this->junctionnessFilter->GetEigenVectorInput());
}


template <class TImage>
void
DataModel<TImage>
::SetFilterToJunctionnessLocalMax() {

  if (!this->imageData)
    return;
  
  this->junctionnessLocalMaxFilter->Update();
  this->minMaxFilter->SetImage(this->junctionnessFilter->GetOutput());
  this->minMaxFilter->Compute();
  this->filteredImageITKToVTKFilter->SetInput(this->junctionnessLocalMaxFilter->GetOutput());
}


template <class TImage>
void
DataModel<TImage>
::SetVoxelSpacing(int dimension, double spacing) {
  if (!this->imageData)
    return;

  typename TImage::SpacingType currentSpacing = this->imageData->GetSpacing();
  currentSpacing[dimension] = spacing;
  this->imageData->SetSpacing(currentSpacing);

  this->inputImageITKToVTKFilter->GetOutputPort()->GetProducer()->Modified();
  this->filteredImageITKToVTKFilter->GetOutputPort()->GetProducer()->Modified();
  this->vectorImageITKToVTKFilter->GetOutputPort()->GetProducer()->Modified();
}


template <class TImage>
void
DataModel<TImage>
::SetVoxelXSpacing(double spacing) {
  this->SetVoxelSpacing(0, spacing); 
}


template <class TImage>
void
DataModel<TImage>
::SetVoxelYSpacing(double spacing) {
  this->SetVoxelSpacing(1, spacing); 
}


template <class TImage>
void
DataModel<TImage>
::SetVoxelZSpacing(double spacing) {
  this->SetVoxelSpacing(2, spacing); 
}


template <class TImage>
void
DataModel<TImage>
::SetFilteredImageScaleFactor(double scale) {
  this->filteredImageScale = scale;
}


template <class TImage>
double
DataModel<TImage>
::GetFilteredImageScaleFactor() {
  return this->filteredImageScale;
}


template <class TImage>
void
DataModel<TImage>
::ComputeConnectedComponentsVsThresholdData(double minThreshold, double maxThreshold,
                                             double thresholdIncrement, std::string fileName) {
  FILE *fp = fopen(fileName.c_str(), "w");
  fprintf(fp, "thresholdValue\tconnectedComponents\n");

  for (double thresholdValue = minThreshold; thresholdValue <= maxThreshold; thresholdValue += thresholdIncrement) {
    
    // Compute number of connected components.
    this->multiscaleFibernessThresholdFilter->SetLowerThreshold(0);
    this->multiscaleFibernessThresholdFilter->SetUpperThreshold(thresholdValue);
    this->multiscaleFibernessThresholdFilter->Modified();

    try {
      this->fibernessConnectedComponentsFilter->Update();
    } catch (...) {
      std::cout << "Exception caught in DataModel::ComputeConnectedComponentsVsThresholdData" << std::endl;
    }
    
    this->minMaxConnectedComponentsFilter->SetImage(this->fibernessConnectedComponentsFilter->GetOutput());
    this->minMaxConnectedComponentsFilter->Compute();
    int connectedComponents = this->minMaxConnectedComponentsFilter->GetMaximum();

    printf("%f\t%d\n", thresholdValue, connectedComponents);
    fprintf(fp, "%f\t%d\n", thresholdValue, connectedComponents);
  }

  fclose(fp);
}


template <class TImage>
void
DataModel<TImage>
::ComputeVolumeFractionEstimateVsZData(double threshold, std::string fileName) {
  FILE *fp = fopen(fileName.c_str(), "w");
  fprintf(fp, "z\tvolumeFraction\n");

  this->multiscaleFibernessThresholdFilter->SetLowerThreshold(0);
  this->multiscaleFibernessThresholdFilter->SetUpperThreshold(threshold);
  this->multiscaleFibernessThresholdFilter->Modified();

  typename itk::AccumulateImageFilter< TImage, TImage >::Pointer accumulateFilter1 =
    itk::AccumulateImageFilter< TImage, TImage >::New();
  accumulateFilter1->SetInput(this->multiscaleFibernessThresholdFilter->GetOutput());
  accumulateFilter1->SetAccumulateDimension(0);

  typename itk::AccumulateImageFilter<TImage, TImage>::Pointer accumulateFilter2 =
    itk::AccumulateImageFilter<TImage, TImage>::New();
  accumulateFilter2->SetInput(accumulateFilter1->GetOutput());
  accumulateFilter2->SetAccumulateDimension(1);
  accumulateFilter2->Update();

  typename TImage::RegionType accumulatedRegion 
    = accumulateFilter2->GetOutput()->GetLargestPossibleRegion();
  itk::Size<3> accumulatedRegionSize = accumulatedRegion.GetSize();

  printf("Reduced dimensions: %ld, %ld, %ld\n",
    accumulatedRegionSize[0], accumulatedRegionSize[1], accumulatedRegionSize[2]);

  // Compute number of pixels above the fiberness threshold value.
  int dims[3];
  this->GetDimensions(dims);

  typename TImage::PixelType pixelsPerSlice 
    = static_cast< typename TImage::PixelType >(dims[0]*dims[1]);
  printf("Pixels per slice: %f\n", pixelsPerSlice);

  typename TImage::Pointer image = accumulateFilter2->GetOutput();
  for (int i = 0; i < dims[2]; i++) {
    typename TImage::IndexType index;
    index[0] = 0;
    index[1] = 0;
    index[2] = i;
    typename TImage::PixelType sum = image->GetPixel(index) / pixelsPerSlice;
    fprintf(fp, "%d\t%f\n", i, sum);
  }

  fclose(fp);
}


template <class TImage>
void
DataModel<TImage>
::SetProgressCallback(ProgressCallback callback) {
  this->progressCallback = callback;
}


template <class TImage>
void
DataModel<TImage>
::UpdateProgress(itk::Object* object, const itk::EventObject& event) {
  itk::ProcessObject* processObject = dynamic_cast<itk::ProcessObject*>(object);
  if (this->progressCallback) {
    std::string filterName;
    itk::ExposeMetaData(object->GetMetaDataDictionary(), "filterName", filterName);
    std::string message("Running ");
    message.append(filterName);
    this->progressCallback(processObject->GetProgress(), message.c_str());
    std::cout << message << ": " 
      << processObject->GetProgress() << std::endl;
  }
}


#endif // _DATA_MODEL_CXX_
