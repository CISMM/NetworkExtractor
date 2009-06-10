#ifndef _VISUALIZATION_H_
#define _VISUALIZATION_H_

#include "OutlineVisualizationPipeline.h"
#include "ImagePlaneVisualizationPipeline.h"
#include "IsosurfaceVisualizationPipeline.h"

#include <vtkAlgorithmOutput.h>
#include <vtkRenderer.h>

class Visualization {

public:
  Visualization();
  virtual ~Visualization();

  void SetImageInputConnection(vtkAlgorithmOutput* input);
  void SetFilteredImageInputConnection(vtkAlgorithmOutput* input);
  void AddToRenderer(vtkRenderer* renderer);

  void SetShowOutline(bool show);
  void ShowOutlineOn();
  void ShowOutlineOff();

  void SetIsosurfaceVisible(bool show);
  void SetIsoValue(double isoValue);
  double GetIsoValue();

  vtkAlgorithmOutput* GetIsosurfaceOutputPort();

  void SetFastIsosurfaceRendering(bool toggle);
  void FastIsosurfaceRenderingOn();
  void FastIsosurfaceRenderingOff();

  void SetImagePlaneVisible(bool show);
  void SetZSlice(int slice);



protected:

  OutlineVisualizationPipeline* outlineVisualization;

  IsosurfaceVisualizationPipeline* isoVisualization;

  ImagePlaneVisualizationPipeline* imagePlaneVisualization;
};

#endif // _VISUALIZATION_H_
