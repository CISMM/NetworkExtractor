#ifndef _IMAGE_PLANE_VISUALIZATION_PIPELINE_H_
#define _IMAGE_PLANE_VISUALIZATION_PIPELINE_H_

#include "VisualizationPipeline.h"

#include <vtkSmartPointer.h>

class vtkImageActor;
class vtkImageShiftScale;
class vtkRenderer;

class ImagePlaneVisualizationPipeline : public VisualizationPipeline {

public:
  ImagePlaneVisualizationPipeline();
  ~ImagePlaneVisualizationPipeline();

  virtual void SetInputConnection(vtkAlgorithmOutput* input);

  virtual void AddToRenderer(vtkRenderer* renderer);

  void SetVisible(bool visible);
  bool GetVisible();

  void SetSliceNumber(int sliceNumber);
  int GetSliceNumber();

  void AutoRescalingOn();
  void AutoRescalingOff();

protected:

  vtkSmartPointer<vtkImageShiftScale> m_ShiftScaler;

  vtkSmartPointer<vtkImageActor> m_ImageActor;

  bool m_AutoRescalingOn;

};

// _IMAGE_PLANE_VISUALIZATION_PIPELINE_H_
#endif
