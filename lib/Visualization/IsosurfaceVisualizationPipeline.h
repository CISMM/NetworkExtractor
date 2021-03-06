#ifndef _ISOSURFACE_VISUALIZATION_PIPELINE_H_
#define _ISOSURFACE_VISUALIZATION_PIPELINE_H_

#include "VisualizationPipeline.h"

class vtkActor;
class vtkAlgorithmOutput;
class vtkContourFilter;
class vtkImageClip;
class vtkPolyDataMapper;
class vtkRenderer;

class IsosurfaceVisualizationPipeline : public VisualizationPipeline  {

public:
  IsosurfaceVisualizationPipeline();
  virtual ~IsosurfaceVisualizationPipeline();

  virtual void AddToRenderer(vtkRenderer* renderer);

  void SetIsoValue(double isoValue);
  double GetIsoValue();

  void SetColor(double r, double g, double b);

  void SetVisible(bool visible);
  bool GetVisible();

  void ClipDataOn();
  void ClipDataOff();
  void SetClipData(bool clip);
  bool GetClipData();

  void UpdateClipping();

  void SetZPlane(int zPlane);
  int GetZPlane();

  void SetDeltaZ(int deltaZ);
  int GetDeltaZ();

  vtkAlgorithmOutput* GetIsosurfaceOutputPort();

  void FastRenderingOn();
  void FastRenderingOff();
  bool GetFastRenderingOn();

protected:
  double m_IsoValue;

  int m_ZPlane;

  int m_DeltaZ;

  vtkImageClip* m_ImageClip;

  vtkContourFilter* m_IsoContourer;

  vtkPolyDataMapper* m_IsoMapper;
  
  vtkActor* m_IsoActor;

};

// _ISOSURFACE_VISUALIZATION_PIPELINE_H_
#endif

