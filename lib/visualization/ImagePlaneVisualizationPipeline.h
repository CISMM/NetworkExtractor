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

  void SetInputConnection(vtkAlgorithmOutput* input);

  void AddToRenderer(vtkRenderer* renderer);

  void SetVisible(bool visible);

  void SetSliceNumber(int sliceNumber);
  int GetSliceNumber();

protected:

  vtkSmartPointer<vtkImageShiftScale> typeCaster;

  vtkSmartPointer<vtkImageActor> imageActor;

};

#endif // _IMAGE_PLANE_VISUALIZATION_PIPELINE_H_
