#ifndef _DATA_MODEL_CXX_
#define _DATA_MODEL_CXX_

#include "DataModel.h"


template <class TImage>
DataModel<TImage>
::DataModel() {
  this->imageData = NULL;

  this->intensityThinningFilter = new IntensityThinningFilterType();
  this->minMaxFilter = MinMaxType::New();
  this->itkToVtkFilter = new ITKImageToVTKImage<TImage>();
}


template <class TImage>
DataModel<TImage>
::~DataModel() {
  delete this->intensityThinningFilter;
  delete this->itkToVtkFilter;
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