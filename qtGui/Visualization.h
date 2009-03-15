#ifndef _VISUALIZATION_H_
#define _VISUALIZATION_H_

#include "OutlineVisualizationPipeline.h"
#include "IsosurfaceVisualizationPipeline.h"

#include <vtkAlgorithmOutput.h>
#include <vtkRenderer.h>

class Visualization {

public:
  Visualization();
  virtual ~Visualization();

  void SetInputConnection(vtkAlgorithmOutput* input);
  void AddToRenderer(vtkRenderer* renderer);

  void SetShowOutline(bool show);
  void ShowOutlineOn();
  void ShowOutlineOff();

  void SetIsoValue(double isoValue);
  double GetIsoValue();

  vtkAlgorithmOutput* GetIsosurfaceOutputPort();

  void SetFastIsosurfaceRendering(bool toggle);
  void FastIsosurfaceRenderingOn();
  void FastIsosurfaceRenderingOff();

protected:
  bool showOutline;

  OutlineVisualizationPipeline* outlineVisualization;

  IsosurfaceVisualizationPipeline* isoVisualization;
};

#endif // _VISUALIZATION_H_