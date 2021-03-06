#ifndef _VISUALIZATION_H_
#define _VISUALIZATION_H_

#include "ArrowVisualizationPipeline.h"
#include "OutlineVisualizationPipeline.h"
#include "ImagePlaneVisualizationPipeline.h"
#include "DVRIsosurfaceVisualizationPipeline.h"

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
  bool GetShowOutline();
  void ShowOutlineOn();
  void ShowOutlineOff();

  void SetIsosurfaceVisible(bool show);
  bool GetIsosurfaceVisible();

  void SetIsoValue(double isoValue);
  double GetIsoValue();

  void SetImagePlaneVisible(bool show);
  bool GetImagePlaneVisible();

  void SetZSlice(int slice);
  int  GetZSlice();

  void SetCropIsosurface(bool crop);
  bool GetCropIsosurface();

  void SetKeepPlanesAboveBelowImagePlane(int keep);
  int  GetKeepPlanesAboveBelowImagePlane();

  void   SetDirectionArrowAzimuth(double azimuth);
  double GetDirectionArrowAzimuth();

  void   SetDirectionArrowInclination(double inclination);
  double GetDirectionArrowInclination();

  void   SetDirectionArrowCenter(double center[3]);
  void   SetDirectionArrowScale(double scale);

  void   SetDirectionArrowVisible(bool show);
  bool   GetDirectionArrowVisible();

protected:
  ArrowVisualizationPipeline*         m_ArrowVisualization;
  OutlineVisualizationPipeline*       m_OutlineVisualization;
  DVRIsosurfaceVisualizationPipeline* m_IsoVisualization;
  ImagePlaneVisualizationPipeline*    m_ImagePlaneVisualization;
};

#endif // _VISUALIZATION_H_
