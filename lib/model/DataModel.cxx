#ifndef _DATA_MODEL_CXX_
#define _DATA_MODEL_CXX_

#if defined(_WIN32) // Turn off deprecation warnings in Visual Studio
#pragma warning( disable : 4996 )
#endif

#include <cstdlib>

#include "DataModel.h"
#include "Types.h"

#include <itkAccumulateImageFilter.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkMetaDataObject.h>
#include <itkMultiThreader.h>
#include <itkPoint.h>

// Constants
const DataModel::FilterType 
DataModel::NO_FILTER_STRING("No filter");

const DataModel::FilterType
DataModel::FRANGI_FIBERNESS_FILTER_STRING("Frangi Fiberness Filter");

const DataModel::FilterType
DataModel::MULTISCALE_FIBERNESS_FILTER_STRING("Multiscale Fiberness");

const DataModel::FilterType
DataModel::MULTISCALE_FIBERNESS_THRESHOLD_FILTER_STRING("Multiscale Fiberness Threshold");

const DataModel::FilterType
DataModel::MULTISCALE_SKELETONIZATION_FILTER_STRING("Skeletonization");

const DataModel::FilterType
DataModel::JUNCTIONNESS_FILTER_STRING("Junctionness");

const DataModel::FilterType
DataModel::JUNCTIONNESS_LOCAL_MAX_FILTER_STRING("Junctionness Local Maxima");


DataModel
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
  itk::MemberCommand< DataModel >::Pointer hessianFilterProgressCommand 
    = itk::MemberCommand< DataModel >::New();

  itk::MemberCommand< DataModel >::Pointer eigenAnalysisFilterProgressCommand 
    = itk::MemberCommand< DataModel >::New();

  itk::MemberCommand< DataModel >::Pointer fibernessFilterProgressCommand 
    = itk::MemberCommand< DataModel >::New();

  itk::MemberCommand< DataModel >::Pointer multiscaleFibernessFilterProgressCommand 
    = itk::MemberCommand< DataModel >::New();

  itk::MemberCommand< DataModel >::Pointer fibernessThresholdFilterProgressCommand
    = itk::MemberCommand< DataModel >::New();

  itk::MemberCommand< DataModel >::Pointer skeletonizationFilterProgressCommand
    = itk::MemberCommand< DataModel >::New();

  itk::MemberCommand< DataModel >::Pointer fibernessConnectedComponentsFilterProgressCommand
    = itk::MemberCommand< DataModel >::New();

  itk::MemberCommand< DataModel >::Pointer junctionnessFilterProgressCommand 
    = itk::MemberCommand< DataModel >::New();

  itk::MemberCommand< DataModel >::Pointer junctionnessLocalMaxFilterProgressCommand
    = itk::MemberCommand< DataModel >::New();

  // Set the callback function for each of the progress reporters.
  hessianFilterProgressCommand->SetCallbackFunction(this, &DataModel::UpdateProgress);
  eigenAnalysisFilterProgressCommand->SetCallbackFunction(this, &DataModel::UpdateProgress);
  fibernessFilterProgressCommand->SetCallbackFunction(this, &DataModel::UpdateProgress);
  multiscaleFibernessFilterProgressCommand->SetCallbackFunction(this, &DataModel::UpdateProgress);
  fibernessThresholdFilterProgressCommand->SetCallbackFunction(this, &DataModel::UpdateProgress);
  skeletonizationFilterProgressCommand->SetCallbackFunction(this, &DataModel::UpdateProgress);
  fibernessConnectedComponentsFilterProgressCommand->SetCallbackFunction(this, &DataModel::UpdateProgress);
  junctionnessFilterProgressCommand->SetCallbackFunction(this, &DataModel::UpdateProgress);
  junctionnessLocalMaxFilterProgressCommand->SetCallbackFunction(this, &DataModel::UpdateProgress);

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

  // ITK will detect the number of cores on the system and set it by default.
  // Here we can override that setting if the proper environment variable is
  // set.
  char *var = getenv("FIBRIN_ANALYSIS_THREADS");
  if (var) {
    int numberOfThreads = atoi(var);
    if (numberOfThreads > 0)
      this->SetNumberOfThreads(numberOfThreads);
  }
}


DataModel
::~DataModel() {
  delete this->inputImageITKToVTKFilter;
  delete this->filteredImageITKToVTKFilter;
  delete this->vectorImageITKToVTKFilter;
}


void
DataModel
::LoadImageFile(std::string fileName) {
  this->imageFileName = fileName;
  ScalarFileReaderType::Pointer reader = ScalarFileReaderType::New();
  reader->SetFileName(fileName.c_str());
  reader->Update();
  this->SetImageData(reader->GetOutput());

  // Connect this image data to the various pipelines.
  this->minMaxFilter->SetImage(this->imageData);
  this->minMaxFilter->Compute();

  this->inputImageITKToVTKFilter->Modified();
  this->inputImageITKToVTKFilter->Update();
  this->filteredImageITKToVTKFilter->Modified();
  this->filteredImageITKToVTKFilter->Update();
}


void
DataModel
::SaveFilteredImageFile(std::string fileName, std::string filterName, float scale) {
  // Check for TIFF output and perform casting as needed.
  size_t tiffLocation = fileName.rfind(".tif");
  if (tiffLocation == fileName.size()-4) {
    ScaleFilterType::Pointer scaler = ScaleFilterType::New();
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

    UShort3DCastType::Pointer caster = UShort3DCastType::New();
    caster->SetInput(scaler->GetOutput());

    TIFFWriterType::Pointer writer = TIFFWriterType::New();
    writer->SetInput(caster->GetOutput());
    writer->SetFileName(fileName.c_str());
    writer->Update();

  } else { // Nothing special to do for VTK output files.

    ScalarFileWriterType::Pointer writer = ScalarFileWriterType::New();
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


std::string
DataModel
::GetImageFileName() {
  return this->imageFileName;
}


void
DataModel
::SetNumberOfThreads(int threads) {
  itk::MultiThreader::SetGlobalDefaultNumberOfThreads(threads);
  itk::MultiThreader::SetGlobalMaximumNumberOfThreads(threads);
}


int
DataModel
::GetNumberOfThreads() {
  return itk::MultiThreader::GetGlobalMaximumNumberOfThreads();
}


void
DataModel
::SaveFiberOrientationImageFile(std::string fileName) {
  VectorFileWriterType::Pointer writer = VectorFileWriterType::New();
  writer->SetInput(this->multiscaleFibernessFilter->GetEigenvectorOutput());
  writer->SetFileName(fileName.c_str());
  writer->Update();
  writer->Write();
}


void
DataModel
::SaveFiberOrientationDataFile(std::string fileName) {
  InputImageType::Pointer skeletonImage = this->skeletonizationFilter->GetOutput();
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


void
DataModel
::SetFiberDiameter(double diameter) {
  if (diameter != this->fiberDiameter) {
    this->fiberDiameter = diameter;

    // Set sigma on the Hessian filter.
    this->hessianFilter->SetSigma(0.5*diameter);
  }
}


void
DataModel
::SetMultiscaleFibernessAlphaCoefficient(double alpha) {
  if (alpha != this->fibernessFilter->GetAlpha())
    this->fibernessFilter->SetAlpha(alpha);
  if (alpha != this->hessianToVesselnessFilter->GetAlpha()) {
    this->hessianToVesselnessFilter->SetAlpha(alpha);
    this->multiscaleFibernessFilter->Modified();
  }
}


double
DataModel
::GetMultiscaleFibernessAlphaCoefficient() {
  return this->fibernessFilter->GetAlpha();
}


void
DataModel
::SetMultiscaleFibernessBetaCoefficient(double beta) {
  if (beta != this->fibernessFilter->GetBeta())
    this->fibernessFilter->SetBeta(beta);
  if (beta != this->hessianToVesselnessFilter->GetBeta()) {
    this->hessianToVesselnessFilter->SetBeta(beta);
    this->multiscaleFibernessFilter->Modified();
  }
}


double
DataModel
::GetMultiscaleFibernessBetaCoefficient() {
  return this->fibernessFilter->GetBeta();
}


void
DataModel
::SetMultiscaleFibernessGammaCoefficient(double gamma) {
  if (gamma != this->fibernessFilter->GetGamma())
    this->fibernessFilter->SetGamma(gamma);
  if (gamma != this->hessianToVesselnessFilter->GetGamma()) {
    this->hessianToVesselnessFilter->SetGamma(gamma);
    this->multiscaleFibernessFilter->Modified();
  }
}


double
DataModel
::GetMultiscaleFibernessGammaCoefficient() {
  return this->fibernessFilter->GetGamma();
}


void
DataModel
::SetMultiscaleFibernessMinimumScale(double minimum) {
  if (minimum != this->multiscaleFibernessFilter->GetSigmaMinimum()) {
    this->multiscaleFibernessFilter->SetSigmaMinimum(minimum);
  }
}


double
DataModel
::GetMultiscaleFibernessMinimumScale() {
  return this->multiscaleFibernessFilter->GetSigmaMinimum();
}


void
DataModel
::SetMultiscaleFibernessMaximumScale(double maximum) {
  if (maximum != this->multiscaleFibernessFilter->GetSigmaMaximum()) {
    this->multiscaleFibernessFilter->SetSigmaMaximum(maximum);
  }
}


double
DataModel
::GetMultiscaleFibernessMaximumScale() {
  return this->multiscaleFibernessFilter->GetSigmaMaximum();
}


void
DataModel
::SetMultiscaleFibernessNumberOfScales(int numberOfScales) {
  this->multiscaleFibernessFilter->SetNumberOfSigmaSteps(numberOfScales);
}


int
DataModel
::GetMultiscaleFibernessNumberOfScales() {
  return this->multiscaleFibernessFilter->GetNumberOfSigmaSteps();
}


void
DataModel
::SetMultiscaleFibernessThreshold(double threshold) {
  this->multiscaleFibernessThresholdFilter->SetLowerThreshold(0.0);
  this->multiscaleFibernessThresholdFilter->SetUpperThreshold(threshold);
}


double
DataModel
::GetMultiscaleFibernessThreshold() {
  return this->multiscaleFibernessThresholdFilter->GetUpperThreshold();
}


double
DataModel
::GetFiberDiameter() {
  return this->fiberDiameter;
}


void
DataModel
::SetJunctionProbeDiameter(double diameter) {
  // Set probe radius on the junctionness filter.
  if (0.5*diameter != this->junctionnessFilter->GetRadius()) {
    this->junctionnessFilter->SetRadius(0.5*diameter);
  }
}


double
DataModel
::GetJunctionProbeDiameter() {
  return 2.0*this->junctionnessFilter->GetRadius();
}


void
DataModel
::SetJunctionFibernessThreshold(double threshold) {
  if (threshold != this->junctionnessFilter->GetVesselnessThreshold()) {
    this->junctionnessFilter->SetVesselnessThreshold(threshold);
  }
}


double
DataModel
::GetJunctionFibernessThreshold() {
  return this->junctionnessFilter->GetVesselnessThreshold();
}


// Deprecated method
void
DataModel
::SetJunctionnessLocalMaxHeight(double height) {
  //if (height != this->junctionnessLocalMaxFilter->GetHeight()) {
  //  this->junctionnessLocalMaxFilter->SetHeight(height);
  //}
}


// Deprecated method
double
DataModel
::GetJunctionnessLocalMaxHeight() {
  //return this->junctionnessLocalMaxFilter->GetHeight();
  return 0;
}


void 
DataModel
::SetImageData(TImage::Pointer image) {
  this->imageData = image;

  this->hessianFilter->SetInput(this->imageData);
  this->multiscaleFibernessFilter->SetInput(this->imageData);

  // Set image data.
  this->inputImageITKToVTKFilter->SetInput(this->imageData);

  // Set filtered image data to input image initially.
  this->filteredImageITKToVTKFilter->SetInput(this->imageData);
}


DataModel::TImage::Pointer
DataModel
::GetImageData() {
  return this->imageData;
}


vtkAlgorithmOutput*
DataModel
::GetImageOutputPort() {
  return this->inputImageITKToVTKFilter->GetOutputPort();
}


vtkAlgorithmOutput*
DataModel
::GetFilteredImageOutputPort() {
  return this->filteredImageITKToVTKFilter->GetOutputPort();
}


vtkAlgorithmOutput*
DataModel
::GetVectorOutputPort() {
  return this->vectorImageITKToVTKFilter->GetOutputPort();
}


double
DataModel
::GetFilteredDataMinimum() {
  return minMaxFilter->GetMinimum();
}


double
DataModel
::GetFilteredDataMaximum() {
  return minMaxFilter->GetMaximum();
}


void
DataModel
::GetDimensions(int dimensions[3]) {
  if (!this->GetImageData()) {
    dimensions[0] = 0;
    dimensions[1] = 0;
    dimensions[2] = 0;
    return;
  }

  UShort3DImageType::RegionType region 
      = this->GetImageData()->GetLargestPossibleRegion();
  itk::Size<3> size = region.GetSize();

  dimensions[0] = size[0];
  dimensions[1] = size[1];
  dimensions[2] = size[2];
}


void
DataModel
::SetFilterToNone() {

  if (this->imageData) {
    this->minMaxFilter->SetImage(this->imageData);
    this->minMaxFilter->Compute();
    
    this->filteredImageITKToVTKFilter->SetInput(this->imageData);
  }
}


void
DataModel
::SetFilterToFrangiFiberness() {

  if (!this->imageData)
    return;

  this->fibernessFilter->Update();
  this->minMaxFilter->SetImage(this->fibernessFilter->GetOutput());
  this->minMaxFilter->Compute();
  this->filteredImageITKToVTKFilter->SetInput(this->fibernessFilter->GetOutput());
}


void
DataModel
::SetFilterToMultiscaleFiberness() {

  if (!this->imageData)
    return;

  this->multiscaleFibernessFilter->Update();
  this->minMaxFilter->SetImage(this->multiscaleFibernessFilter->GetOutput());
  this->minMaxFilter->Compute();
  this->filteredImageITKToVTKFilter->SetInput(this->multiscaleFibernessFilter->GetOutput());
}


void
DataModel
::SetFilterToMultiscaleFibernessThreshold() {

  if (!this->imageData)
    return;

  this->multiscaleFibernessThresholdFilter->Update();
  this->minMaxFilter->SetImage(this->multiscaleFibernessThresholdFilter->GetOutput());
  this->minMaxFilter->Compute();
  this->filteredImageITKToVTKFilter->SetInput(this->multiscaleFibernessThresholdFilter->GetOutput());
}


void
DataModel
::SetFilterToMultiscaleSkeletonization() {

  if (!this->imageData)
    return;

  this->skeletonizationFilter->Update();
  this->filteredImageITKToVTKFilter->SetInput(this->skeletonizationFilter->GetOutput());
}


void
DataModel
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


void
DataModel
::SetFilterToJunctionnessLocalMax() {

  if (!this->imageData)
    return;
  
  this->junctionnessLocalMaxFilter->Update();
  this->minMaxFilter->SetImage(this->junctionnessFilter->GetOutput());
  this->minMaxFilter->Compute();
  this->filteredImageITKToVTKFilter->SetInput(this->junctionnessLocalMaxFilter->GetOutput());
}


void
DataModel
::SetVoxelSpacing(double spacing[3]) {
  if (!this->imageData)
    return;

  this->imageData->SetSpacing(spacing);

  this->inputImageITKToVTKFilter->GetOutputPort()->GetProducer()->Modified();
  this->filteredImageITKToVTKFilter->GetOutputPort()->GetProducer()->Modified();
  this->vectorImageITKToVTKFilter->GetOutputPort()->GetProducer()->Modified();
}


void
DataModel
::SetVoxelSpacing(int dimension, double spacing) {
  if (!this->imageData)
    return;

  TImage::SpacingType currentSpacing = this->imageData->GetSpacing();
  currentSpacing[dimension] = spacing;
  this->imageData->SetSpacing(currentSpacing);

  this->inputImageITKToVTKFilter->GetOutputPort()->GetProducer()->Modified();
  this->filteredImageITKToVTKFilter->GetOutputPort()->GetProducer()->Modified();
  this->vectorImageITKToVTKFilter->GetOutputPort()->GetProducer()->Modified();
}


void
DataModel
::SetVoxelXSpacing(double spacing) {
  this->SetVoxelSpacing(0, spacing); 
}


void
DataModel
::SetVoxelYSpacing(double spacing) {
  this->SetVoxelSpacing(1, spacing); 
}


void
DataModel
::SetVoxelZSpacing(double spacing) {
  this->SetVoxelSpacing(2, spacing); 
}


void
DataModel
::GetVoxelSpacing(double spacing[3]) {
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


void
DataModel
::SetFilteredImageScaleFactor(double scale) {
  this->filteredImageScale = scale;
}


double
DataModel
::GetFilteredImageScaleFactor() {
  return this->filteredImageScale;
}


void
DataModel
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


void
DataModel
::ComputeVolumeFractionEstimateVsZData(double threshold, std::string fileName) {
  FILE *fp = fopen(fileName.c_str(), "w");
  fprintf(fp, "z\tvolumeFraction\n");

  this->multiscaleFibernessThresholdFilter->SetLowerThreshold(0);
  this->multiscaleFibernessThresholdFilter->SetUpperThreshold(threshold);
  this->multiscaleFibernessThresholdFilter->Modified();

  itk::AccumulateImageFilter< TImage, TImage >::Pointer accumulateFilter1 =
    itk::AccumulateImageFilter< TImage, TImage >::New();
  accumulateFilter1->SetInput(this->multiscaleFibernessThresholdFilter->GetOutput());
  accumulateFilter1->SetAccumulateDimension(0);

  itk::AccumulateImageFilter<TImage, TImage>::Pointer accumulateFilter2 =
    itk::AccumulateImageFilter<TImage, TImage>::New();
  accumulateFilter2->SetInput(accumulateFilter1->GetOutput());
  accumulateFilter2->SetAccumulateDimension(1);
  accumulateFilter2->Update();

  TImage::RegionType accumulatedRegion 
    = accumulateFilter2->GetOutput()->GetLargestPossibleRegion();
  itk::Size<3> accumulatedRegionSize = accumulatedRegion.GetSize();

  printf("Reduced dimensions: %ld, %ld, %ld\n",
    accumulatedRegionSize[0], accumulatedRegionSize[1], accumulatedRegionSize[2]);

  // Compute number of pixels above the fiberness threshold value.
  int dims[3];
  this->GetDimensions(dims);

  TImage::PixelType pixelsPerSlice 
    = static_cast< TImage::PixelType >(dims[0]*dims[1]);
  printf("Pixels per slice: %f\n", pixelsPerSlice);

  TImage::Pointer image = accumulateFilter2->GetOutput();
  for (int i = 0; i < dims[2]; i++) {
    TImage::IndexType index;
    index[0] = 0;
    index[1] = 0;
    index[2] = i;
    TImage::PixelType sum = image->GetPixel(index) / pixelsPerSlice;
    fprintf(fp, "%d\t%f\n", i, sum);
  }

  fclose(fp);
}


void
DataModel
::SetProgressCallback(ProgressCallback callback) {
  this->progressCallback = callback;
}


void
DataModel
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
