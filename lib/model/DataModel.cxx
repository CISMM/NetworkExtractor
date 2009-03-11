#ifndef _DATA_MODEL_CXX_
#define _DATA_MODEL_CXX_

#include "DataModel.h"


template <class TImage>
DataModel<TImage>
::DataModel() {
  dirtyBit = true;
  imageData = NULL;

  minMaxFilter = MinMaxType::New();
  itkToVtkFilter = new ITKImageToVTKImage<TImage>();
}


template <class TImage>
DataModel<TImage>
::~DataModel() {
}


template <class TImage>
void
DataModel<TImage>
::SetDirty() {
  this->dirtyBit = true;
}


template <class TImage>
void
DataModel<TImage>
::SetClean() {
  this->dirtyBit = false;
}


template <class TImage>
bool
DataModel<TImage>
::IsDirty() {
  return this->dirtyBit;
}


template <class TImage>
void
DataModel<TImage>
::LoadImageFile(std::string fileName) {
  LoadVTKImageFile<TImage> *loader = new LoadVTKImageFile<TImage>();
  this->imageData = loader->LoadFile(fileName);
  this->SetImageData(this->imageData);
  delete loader;

  // Connect this image data to the various pipelines.
  this->minMaxFilter->SetImage(this->imageData);
  this->minMaxFilter->Compute();

  this->SetDirty();
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
::GetMinimumImageIntensity() {
  return minMaxFilter->GetMinimum();
}


template <class TImage>
double
DataModel<TImage>
::GetMaximumImageIntensity() {
  return minMaxFilter->GetMaximum();
}


template <class TImage>
void
DataModel<TImage>
::GetDimensions(int dimensions[3]) {
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
  itk::Vector<double> thisSpacing = this->GetImageData()->GetSpacing();
  spacing[0] = thisSpacing[0];
  spacing[1] = thisSpacing[1];
  spacing[2] = thisSpacing[2];
}


template <class TImage>
void
DataModel<TImage>
::SetThresholdMethodToIntensity() {
  this->thresholdMethod = INTENSITY_THRESHOLD;
  this->SetDirty();
}


template <class TImage>
void
DataModel<TImage>
::SetThresholdMethodToVesselness() {
  this->thresholdMethod = VESSELNESS_THRESHOLD;
  this->SetDirty();
}


#endif // _DATA_MODEL_CXX_