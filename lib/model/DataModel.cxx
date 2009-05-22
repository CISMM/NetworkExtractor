#ifndef _DATA_MODEL_CXX_
#define _DATA_MODEL_CXX_

#include "DataModel.h"

#include <itkMetaDataObject.h>
#include <itkMultiThreader.h>
#include <itkAccumulateImageFilter.h>


// Constants
template <class TImage>
const typename DataModel<TImage>::FilterType typename DataModel<TImage>::NO_FILTER_STRING = std::string("No filter");

template <class TImage>
const typename DataModel<TImage>::FilterType typename DataModel<TImage>::FIBERNESS_FILTER_STRING 
  = std::string("Fiberness");

template <class TImage>
const typename DataModel<TImage>::FilterType typename DataModel<TImage>::FIBERNESS_THRESHOLD_FILTER_STRING 
  = std::string("Fiberness Threshold");

template <class TImage>
const typename DataModel<TImage>::FilterType typename DataModel<TImage>::JUNCTIONNESS_FILTER_STRING 
  = std::string("Junctionness");

template <class TImage>
const typename DataModel<TImage>::FilterType typename DataModel<TImage>::JUNCTIONNESS_LOCAL_MAX_FILTER_STRING
  = std::string("Junctionness Local Maxima");


template <class TImage>
DataModel<TImage>
::DataModel() {
  this->fiberDiameter = 1.0f;
  this->imageData = NULL;

  this->minMaxFilter = MinMaxType::New();

  // Fiberness filters.
  this->hessianFilter = HessianFilterType::New();
  //this->hessianFilter->ReleaseDataFlagOn();

  this->eigenAnalysisFilter = HessianEigenAnalysisFilterType::New();
  //this->eigenAnalysisFilter->ReleaseDataFlagOn();
  this->eigenAnalysisFilter->SetInput(this->hessianFilter->GetOutput());
  
  this->fibernessFilter = FibernessFilterType::New();
  this->fibernessFilter->SetInput(this->eigenAnalysisFilter->GetEigenValues());

  this->fibernessThresholdFilter = ThresholdFilterType::New();
  this->fibernessThresholdFilter->SetInput(this->fibernessFilter->GetOutput());
  this->fibernessThresholdFilter->SetInsideValue(0.0);
  this->fibernessThresholdFilter->SetOutsideValue(1.0);

  this->fibernessConnectedComponentsFilter = ConnectedComponentFilterType::New();
  this->fibernessConnectedComponentsFilter->SetInput(this->fibernessThresholdFilter->GetOutput());
  this->fibernessConnectedComponentsFilter->FullyConnectedOn();

  this->minMaxConnectedComponentsFilter = MinMaxConnectedComponentFilterType::New();

  this->junctionnessFilter = JunctionnessFilterType::New();
  this->junctionnessFilter->SetEigenVectorInput(this->eigenAnalysisFilter->GetEigenVectors());
  this->junctionnessFilter->SetVesselnessInput(this->fibernessFilter->GetOutput());

  this->junctionnessLocalMaxFilter = JunctionnessLocalMaxFilterType::New();
  this->junctionnessLocalMaxFilter->SetInput(this->junctionnessFilter->GetOutput());
  this->junctionnessLocalMaxFilter->FullyConnectedOn();

  this->scalarImageITKToVTKFilter = new ITKImageToVTKImage<TImage>();
  this->vectorImageITKToVTKFilter = new ITKImageToVTKImage<EigenVectorImageType>();
  this->progressCallback = NULL;

    // Hook up progress reporter for the filters.
  itk::MemberCommand<DataModel<TImage>>::Pointer hessianFilterProgressCommand 
    = itk::MemberCommand<DataModel<TImage>>::New();

  itk::MemberCommand<DataModel<TImage>>::Pointer eigenAnalysisFilterProgressCommand 
    = itk::MemberCommand<DataModel<TImage>>::New();

  itk::MemberCommand<DataModel<TImage>>::Pointer fibernessFilterProgressCommand 
    = itk::MemberCommand<DataModel<TImage>>::New();

  itk::MemberCommand<DataModel<TImage>>::Pointer fibernessThresholdFilterProgressCommand
    = itk::MemberCommand<DataModel<TImage>>::New();

  itk::MemberCommand<DataModel<TImage>>::Pointer fibernessConnectedComponentsFilterProgressCommand
    = itk::MemberCommand<DataModel<TImage>>::New();

  itk::MemberCommand<DataModel<TImage>>::Pointer junctionnessFilterProgressCommand 
    = itk::MemberCommand<DataModel<TImage>>::New();

  itk::MemberCommand<DataModel<TImage>>::Pointer junctionnessLocalMaxFilterProgressCommand
    = itk::MemberCommand<DataModel<TImage>>::New();

  // Set the callback function for each of the progress reporters.
  hessianFilterProgressCommand->SetCallbackFunction(this, &DataModel<TImage>::UpdateProgress);
  eigenAnalysisFilterProgressCommand->SetCallbackFunction(this, &DataModel<TImage>::UpdateProgress);
  fibernessFilterProgressCommand->SetCallbackFunction(this, &DataModel<TImage>::UpdateProgress);
  fibernessThresholdFilterProgressCommand->SetCallbackFunction(this, &DataModel<TImage>::UpdateProgress);
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
  itk::EncapsulateMetaData<std::string >(fibernessThresholdFilter->GetMetaDataDictionary(),
    "filterName", std::string("Fiberness Threshold Filter"));
  itk::EncapsulateMetaData<std::string >(fibernessConnectedComponentsFilter->GetMetaDataDictionary(),
    "filterName", std::string("Fiberness Connected Components Filter"));
  itk::EncapsulateMetaData<std::string >(junctionnessFilter->GetMetaDataDictionary(),
    "filterName", std::string("Junctionness Filter"));
  itk::EncapsulateMetaData<std::string>(junctionnessLocalMaxFilter->GetMetaDataDictionary(),
    "filterName", std::string("Junctionness Local Max Filter"));

  this->hessianFilter->AddObserver(itk::ProgressEvent(), hessianFilterProgressCommand);
  this->eigenAnalysisFilter->AddObserver(itk::ProgressEvent(), eigenAnalysisFilterProgressCommand);
  this->fibernessFilter->AddObserver(itk::ProgressEvent(), fibernessFilterProgressCommand);
  this->fibernessThresholdFilter->AddObserver(itk::ProgressEvent(), fibernessThresholdFilterProgressCommand);
  this->fibernessConnectedComponentsFilter->AddObserver(itk::ProgressEvent(), fibernessConnectedComponentsFilterProgressCommand);
  this->junctionnessFilter->AddObserver(itk::ProgressEvent(), junctionnessFilterProgressCommand);
  this->junctionnessLocalMaxFilter->AddObserver(itk::ProgressEvent(), junctionnessLocalMaxFilterProgressCommand);

  itk::MultiThreader::SetGlobalMaximumNumberOfThreads(2);
}


template <class TImage>
DataModel<TImage>
::~DataModel() {
  delete this->scalarImageITKToVTKFilter;
  delete this->vectorImageITKToVTKFilter;
}


template <class TImage>
void
DataModel<TImage>
::LoadImageFile(std::string fileName) {
  ScalarFileReaderType::Pointer reader = ScalarFileReaderType::New();
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
::SaveImageFile(std::string fileName, std::string filterName) {
  // Check for TIFF output and perform casting as needed.
  size_t tiffLocation = fileName.rfind(".tif");
  if (tiffLocation == fileName.size()-4) {
    UShort3DCastType::Pointer caster = UShort3DCastType::New();
    TIFFWriterType::Pointer writer = TIFFWriterType::New();
   
    if (filterName == NO_FILTER_STRING)
      caster->SetInput(this->imageData);
    else if (filterName == FIBERNESS_FILTER_STRING)
      caster->SetInput(this->fibernessFilter->GetOutput());
    else if (filterName == FIBERNESS_THRESHOLD_FILTER_STRING)
      caster->SetInput(this->fibernessThresholdFilter->GetOutput());
    else if (filterName == JUNCTIONNESS_FILTER_STRING)
      caster->SetInput(this->junctionnessFilter->GetOutput());
    else if (filterName == JUNCTIONNESS_LOCAL_MAX_FILTER_STRING)
      caster->SetInput(this->junctionnessLocalMaxFilter->GetOutput());
    else
      return;

    writer->SetInput(caster->GetOutput());
    writer->SetFileName(fileName.c_str());
    writer->Update();

  } else { // Nothing special to do for VTK output files.

    ScalarFileWriterType::Pointer writer = ScalarFileWriterType::New();
    if (filterName == NO_FILTER_STRING)
      writer->SetInput(this->imageData);
    else if (filterName == FIBERNESS_FILTER_STRING)
      writer->SetInput(this->fibernessFilter->GetOutput());
    else if (filterName == FIBERNESS_THRESHOLD_FILTER_STRING)
      writer->SetInput(this->fibernessThresholdFilter->GetOutput());
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
  VectorFileWriterType::Pointer writer = VectorFileWriterType::New();
  writer->SetInput(junctionnessFilter->GetEigenVectorInput());
  writer->SetFileName(fileName.c_str());
  writer->Update();
  writer->Write();
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
::SetFibernessAlphaCoefficient(double alpha) {
  if (alpha != this->fibernessFilter->GetAlpha())
    this->fibernessFilter->SetAlpha(alpha);
}


template <class TImage>
double
DataModel<TImage>
::GetFibernessAlphaCoefficient() {
  return this->fibernessFilter->GetAlpha();
}


template <class TImage>
void
DataModel<TImage>
::SetFibernessBetaCoefficient(double beta) {
  if (beta != this->fibernessFilter->GetBeta())
    this->fibernessFilter->SetBeta(beta);
}


template <class TImage>
double
DataModel<TImage>
::GetFibernessBetaCoefficient() {
  return this->fibernessFilter->GetBeta();
}


template <class TImage>
void
DataModel<TImage>
::SetFibernessCCoefficient(double c) {
  if (c != this->fibernessFilter->GetC())
    this->fibernessFilter->SetC(c);
}


template <class TImage>
double
DataModel<TImage>
::GetFibernessCCoefficient() {
  return this->fibernessFilter->GetC();
}


template <class TImage>
void
DataModel<TImage>
::SetFibernessThreshold(double threshold) {
  this->fibernessThresholdFilter->SetLowerThreshold(0.0);
  this->fibernessThresholdFilter->SetUpperThreshold(threshold);
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

  // Set display volume to image data by default.
  this->scalarImageITKToVTKFilter->SetInput(this->imageData);
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
::GetOutputPort() {
  return this->scalarImageITKToVTKFilter->GetOutputPort();
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

  UShort3DImageType::RegionType region 
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
    
    this->scalarImageITKToVTKFilter->SetInput(this->imageData);
  }
}


template <class TImage>
void
DataModel<TImage>
::SetFilterToFiberness() {

  if (!this->imageData)
    return;

  this->fibernessFilter->Update();
  this->minMaxFilter->SetImage(this->fibernessFilter->GetOutput());
  this->minMaxFilter->Compute();
  this->scalarImageITKToVTKFilter->SetInput(this->fibernessFilter->GetOutput());
}


template <class TImage>
void
DataModel<TImage>
::SetFilterToFibernessThreshold() {

  if (!this->imageData)
    return;

  this->fibernessThresholdFilter->Update();
  this->minMaxFilter->SetImage(this->fibernessThresholdFilter->GetOutput());
  this->minMaxFilter->Compute();
  this->scalarImageITKToVTKFilter->SetInput(this->fibernessThresholdFilter->GetOutput());
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
  this->scalarImageITKToVTKFilter->SetInput(this->junctionnessFilter->GetOutput());
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
  this->scalarImageITKToVTKFilter->SetInput(this->junctionnessLocalMaxFilter->GetOutput());
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
    this->fibernessThresholdFilter->SetLowerThreshold(0);
    this->fibernessThresholdFilter->SetUpperThreshold(thresholdValue);
    this->fibernessThresholdFilter->Modified();

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

  this->fibernessThresholdFilter->SetLowerThreshold(0);
  this->fibernessThresholdFilter->SetUpperThreshold(threshold);
  this->fibernessThresholdFilter->Modified();

  itk::AccumulateImageFilter<TImage, TImage>::Pointer accumulateFilter1 =
    itk::AccumulateImageFilter<TImage, TImage>::New();
  accumulateFilter1->SetInput(this->fibernessThresholdFilter->GetOutput());
  accumulateFilter1->SetAccumulateDimension(0);

  itk::AccumulateImageFilter<TImage, TImage>::Pointer accumulateFilter2 =
    itk::AccumulateImageFilter<TImage, TImage>::New();
  accumulateFilter2->SetInput(accumulateFilter1->GetOutput());
  accumulateFilter2->SetAccumulateDimension(1);
  accumulateFilter2->Update();

  TImage::RegionType accumulatedRegion 
    = accumulateFilter2->GetOutput()->GetLargestPossibleRegion();
  itk::Size<3> accumulatedRegionSize = accumulatedRegion.GetSize();

  printf("Reduced dimensions: %d, %d, %d\n",
    accumulatedRegionSize[0], accumulatedRegionSize[1], accumulatedRegionSize[2]);

  // Compute number of pixels above the fiberness threshold value.
  int dims[3];
  this->GetDimensions(dims);

  TImage::PixelType pixelsPerSlice 
    = static_cast<TImage::PixelType>(dims[0]*dims[1]);
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