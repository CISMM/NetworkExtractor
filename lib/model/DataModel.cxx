#ifndef _DATA_MODEL_CXX_
#define _DATA_MODEL_CXX_

#include "DataModel.h"


template <class TImage>
DataModel<TImage>
::DataModel() {
  this->fiberDiameter = 1.0f;
  this->imageData = NULL;

  this->minMaxFilter = MinMaxType::New();
  
  // Vesselness filters.
  this->hessianFilter = HessianFilterType::New();
  this->vesselnessFilter = VesselnessFilterType::New();
  
  this->itkToVtkFilter = new ITKImageToVTKImage<TImage>();

  this->progressCallback = NULL;
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
::SaveImageFile(std::string fileName) {
  FileWriterType::Pointer writer = FileWriterType::New();
  writer->SetInput(this->vesselnessFilter->GetOutput());
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
  itkToVtkFilter->SetInput(image);
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

  if (this->imageData) {
    itk::MemberCommand<DataModel<TImage>>::Pointer progressCommand 
      = itk::MemberCommand<DataModel<TImage>>::New();
    progressCommand->SetCallbackFunction(this, &DataModel<TImage>::UpdateProgress);
    this->hessianFilter->AddObserver(itk::ProgressEvent(), progressCommand);
    this->hessianFilter->SetInput(this->imageData);

    this->vesselnessFilter->AddObserver(itk::ProgressEvent(), progressCommand);
    this->vesselnessFilter->SetInput(hessianFilter->GetOutput());
    this->vesselnessFilter->Update();
    this->minMaxFilter->SetImage(this->vesselnessFilter->GetOutput());
    this->minMaxFilter->Compute();

    this->itkToVtkFilter->SetInput(this->vesselnessFilter->GetOutput());
  }
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
    this->progressCallback(processObject->GetProgress());
    std::cout << processObject->GetNameOfClass() << std::endl;
  }
}


#endif // _DATA_MODEL_CXX_