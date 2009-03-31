#ifndef _DATA_MODEL_CXX_
#define _DATA_MODEL_CXX_

#include "DataModel.h"

#include <itkMetaDataObject.h>
#include <itkMultiThreader.h>


// Constants
template <class TImage>
const typename DataModel<TImage>::FilterType typename DataModel<TImage>::NO_FILTER_STRING = std::string("No filter");

template <class TImage>
const typename DataModel<TImage>::FilterType typename DataModel<TImage>::VESSELNESS_FILTER_STRING 
  = std::string("Vesselness");

template <class TImage>
const typename DataModel<TImage>::FilterType typename DataModel<TImage>::JUNCTIONNESS_FILTER_STRING 
  = std::string("Junctionness");


template <class TImage>
DataModel<TImage>
::DataModel() {
  this->fiberDiameter = 1.0f;
  this->imageData = NULL;

  this->minMaxFilter = MinMaxType::New();

  // Vesselness filters.
  this->hessianFilter = HessianFilterType::New();
  //this->hessianFilter->ReleaseDataFlagOn();

  this->eigenAnalysisFilter = HessianEigenAnalysisFilterType::New();
  //this->eigenAnalysisFilter->ReleaseDataFlagOn();
  this->eigenAnalysisFilter->SetInput(this->hessianFilter->GetOutput());
  
  this->vesselnessFilter = VesselnessFilterType::New();
  this->vesselnessFilter->SetInput(this->eigenAnalysisFilter->GetEigenValues());

  this->junctionnessFilter = JunctionnessFilterType::New();
  this->junctionnessFilter->SetEigenVectorInput(this->eigenAnalysisFilter->GetEigenVectors());
  this->junctionnessFilter->SetVesselnessInput(this->vesselnessFilter->GetOutput());
  
  this->itkToVtkFilter = new ITKImageToVTKImage<TImage>();
  this->progressCallback = NULL;

    // Hook up progress reporter for the filters.
  itk::MemberCommand<DataModel<TImage>>::Pointer hessianFilterProgressCommand 
    = itk::MemberCommand<DataModel<TImage>>::New();

  itk::MemberCommand<DataModel<TImage>>::Pointer eigenAnalysisFilterProgressCommand 
    = itk::MemberCommand<DataModel<TImage>>::New();

  itk::MemberCommand<DataModel<TImage>>::Pointer vesselnessFilterProgressCommand 
    = itk::MemberCommand<DataModel<TImage>>::New();

  itk::MemberCommand<DataModel<TImage>>::Pointer junctionnessFilterProgressCommand 
    = itk::MemberCommand<DataModel<TImage>>::New();

  // Set the callback function for each of the progress reporters.
  hessianFilterProgressCommand->SetCallbackFunction(this, &DataModel<TImage>::UpdateProgress);
  eigenAnalysisFilterProgressCommand->SetCallbackFunction(this, &DataModel<TImage>::UpdateProgress);
  vesselnessFilterProgressCommand->SetCallbackFunction(this, &DataModel<TImage>::UpdateProgress);
  junctionnessFilterProgressCommand->SetCallbackFunction(this, &DataModel<TImage>::UpdateProgress);

  // Attach a MetaDataObject with the name of the filter to each rilter.
  itk::EncapsulateMetaData<std::string >(hessianFilter->GetMetaDataDictionary(),
    "filterName", std::string("Hessian Filter"));
  itk::EncapsulateMetaData<std::string >(eigenAnalysisFilter->GetMetaDataDictionary(),
    "filterName", std::string("Eigen Analysis Filter"));
  itk::EncapsulateMetaData<std::string >(vesselnessFilter->GetMetaDataDictionary(),
    "filterName", std::string("Vesselness Filter"));
  itk::EncapsulateMetaData<std::string >(junctionnessFilter->GetMetaDataDictionary(),
    "filterName", std::string("Junctionness Filter"));

  this->hessianFilter->AddObserver(itk::ProgressEvent(), hessianFilterProgressCommand);
  this->eigenAnalysisFilter->AddObserver(itk::ProgressEvent(), eigenAnalysisFilterProgressCommand);
  this->vesselnessFilter->AddObserver(itk::ProgressEvent(), vesselnessFilterProgressCommand);
  this->junctionnessFilter->AddObserver(itk::ProgressEvent(), junctionnessFilterProgressCommand);

  itk::MultiThreader::SetGlobalMaximumNumberOfThreads(2);
}


template <class TImage>
DataModel<TImage>
::~DataModel() {
  delete this->itkToVtkFilter;
}


template <class TImage>
void
DataModel<TImage>
::LoadImageFile(std::string fileName) {
  FileReaderType::Pointer reader = FileReaderType::New();
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
  FileWriterType::Pointer writer = FileWriterType::New();
  if (filterName == NO_FILTER_STRING)
    writer->SetInput(this->imageData);
  else if (filterName == VESSELNESS_FILTER_STRING)
    writer->SetInput(this->vesselnessFilter->GetOutput());
  else if (filterName == JUNCTIONNESS_FILTER_STRING)
    writer->SetInput(this->junctionnessFilter->GetOutput());
  else
    return;

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

    // Set radius on junctionness filter. We'll set it to
    // twice the diameter of the fibers to make sure we
    // clear the fibers.
    this->junctionnessFilter->SetRadius(diameter);
  }
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
::SetImageData(typename TImage::Pointer image) {
  this->imageData = image;

  this->hessianFilter->SetInput(this->imageData);

  // Set display volume to image data by default.
  itkToVtkFilter->SetInput(this->imageData);
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
  return this->itkToVtkFilter->GetOutputPort();
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
    
    this->itkToVtkFilter->SetInput(this->imageData);
  }
}


template <class TImage>
void
DataModel<TImage>
::SetFilterToVesselness() {

  if (!this->imageData)
    return;

  this->vesselnessFilter->Update();
  this->minMaxFilter->SetImage(this->vesselnessFilter->GetOutput());
  this->minMaxFilter->Compute();
  this->itkToVtkFilter->SetInput(this->vesselnessFilter->GetOutput());
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
  this->itkToVtkFilter->SetInput(this->junctionnessFilter->GetOutput());
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