#ifndef _ARROW_VISUALIZATION_PIPELINE_H_
#define _ARROW_VISUALIZATION_PIPELINE_H_

#include "VisualizationPipeline.h"

#include <vtkSmartPointer.h>

class vtkActor;
class vtkAlgorithmOutput;
class vtkArrowSource;
class vtkPolyDataMapper;
class vtkRenderer;

class ArrowVisualizationPipeline : public VisualizationPipeline {

public:
  ArrowVisualizationPipeline();
  virtual ~ArrowVisualizationPipeline();

  virtual void AddToRenderer(vtkRenderer* renderer);

  void   SetAzimuth(double azimuth);
  double GetAzimuth();

  void   SetInclination(double inclination);
  double GetInclination();

  void   SetCenter(double center[3]);
  void   SetScale(double scale);

  void SetVisible(bool visible);
  bool GetVisible();

protected:
  double m_Azimuth;
  double m_Inclination;

  vtkSmartPointer<vtkArrowSource>    m_ArrowSource;
  vtkSmartPointer<vtkPolyDataMapper> m_Mapper;
  vtkSmartPointer<vtkActor>          m_Actor;

};

#endif // _ARROW_VISUALIZATION_PIPELINE_H_
