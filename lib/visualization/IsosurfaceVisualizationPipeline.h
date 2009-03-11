#ifndef _ISOSURFACE_VISUALIZATION_PIPELINE_H_
#define _ISOSURFACE_VISUALIZATION_PIPELINE_H_

#include "VisualizationPipeline.h"

class vtkActor;
class vtkAlgorithmOutput;
class vtkContourFilter;
class vtkPolyDataMapper;
class vtkRenderer;

class IsosurfaceVisualizationPipeline : public VisualizationPipeline  {

public:
  IsosurfaceVisualizationPipeline();
  ~IsosurfaceVisualizationPipeline();

  void AddToRenderer(vtkRenderer* renderer);

  void SetIsoValue(double isoValue);
  double GetIsoValue();

protected:
  double isoValue;

  vtkContourFilter* isoContourer;

  vtkPolyDataMapper* isoMapper;
  
  vtkActor* isoActor;

};


#endif // _ISOSURFACE_VISUALIZATION_PIPELINE_H_