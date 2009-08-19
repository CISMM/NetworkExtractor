#ifndef _DVR_ISOSURFACE_VISUALIZATION_PIPELINE_H_
#define _DVR_ISOSURFACE_VISUALIZATION_PIPELINE_H_

#include "VisualizationPipeline.h"

#include <vtkSmartPointer.h>

class vtkActor;
class vtkAlgorithmOutput;
class vtkColorTransferFunction;
class vtkImageClip;
class vtkImageShiftScale;
class vtkPiecewiseFunction;
class vtkRenderer;
class vtkVolume;
class vtkVolumeProperty;
class vtkVolumeTextureMapper2D;
class vtkVolumeRayCastMapper;
class vtkVolumeRayCastIsosurfaceFunction;


class DVRIsosurfaceVisualizationPipeline : public VisualizationPipeline  {

public:
  DVRIsosurfaceVisualizationPipeline();
  virtual ~DVRIsosurfaceVisualizationPipeline();

  //virtual void SetInputConnection(vtkAlgorithmOutput* input);

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

protected:
  double m_IsoValue;
  int    m_ZPlane;
  int    m_DeltaZ;

  vtkSmartPointer<vtkImageShiftScale>       m_ShiftScaler;
  vtkSmartPointer<vtkImageClip>             m_ImageClip;
  vtkSmartPointer<vtkPiecewiseFunction>     m_OpacityTransferFunction;
  vtkSmartPointer<vtkColorTransferFunction> m_ColorTransferFunction;
  vtkSmartPointer<vtkVolume>                m_Volume;
  vtkSmartPointer<vtkVolumeProperty>        m_VolumeProperty;

  vtkSmartPointer<vtkVolumeRayCastIsosurfaceFunction> m_RayCastFunction;
  vtkSmartPointer<vtkVolumeRayCastMapper>   m_VolumeMapper;
};

// _DVR_ISOSURFACE_VISUALIZATION_PIPELINE_H_
#endif
