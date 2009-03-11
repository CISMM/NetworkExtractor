#ifndef _DATA_MODEL_H_
#define _DATA_MODEL_H_

#include <string>

#include <itkImage.h>
#include <itkMinimumMaximumImageCalculator.h>

class vtkAlgorithmOutput;

#include "LoadVTKImageFile.h"
#include "ITKImageToVTKImage.h"

// This is the data model for the Fibrin Analysis library.
template <class TImage>
class DataModel {

  typedef enum {
    INTENSITY_THRESHOLD,
    VESSELNESS_THRESHOLD
  } ThresholdMethod;

  typedef itk::MinimumMaximumImageCalculator<typename TImage> MinMaxType;

public:
  DataModel();
  virtual ~DataModel();

  void SetDirty();
  void SetClean();
  bool IsDirty();

  void LoadImageFile(std::string fileName);

  void SetImageData(typename TImage::Pointer image);
  typename TImage::Pointer GetImageData();

  vtkAlgorithmOutput* GetOutputPort();

  double GetMinimumImageIntensity();
  double GetMaximumImageIntensity();

  void GetDimensions(int dimensions[3]);

  void GetSpacing(double spacing[3]);

  void SetThresholdMethodToIntensity();
  void SetThresholdMethodToVesselness();

protected:
  bool dirtyBit;

  typename TImage::Pointer imageData;

  ThresholdMethod thresholdMethod;

  typename MinMaxType::Pointer minMaxFilter;

  ITKImageToVTKImage<TImage>* itkToVtkFilter;

};

#include "DataModel.cxx"

#endif // _DATA_MODEL_H_