#include "Visualization.h"

Visualization
::Visualization() {
  m_ArrowVisualization   = new ArrowVisualizationPipeline();
  m_OutlineVisualization = new OutlineVisualizationPipeline();
  m_IsoVisualization     = new IsosurfaceVisualizationPipeline();
  m_IsoVisualization->SetColor(0.0, 1.0, 0.0);
  m_ImagePlaneVisualization = new ImagePlaneVisualizationPipeline();
}


Visualization
::~Visualization() {
  delete m_ArrowVisualization;
  delete m_OutlineVisualization;
  delete m_IsoVisualization;
  delete m_ImagePlaneVisualization;
}


void
Visualization
::SetImageInputConnection(vtkAlgorithmOutput* input) {
  input->GetProducer()->Modified();
  input->GetProducer()->Update();

  m_ArrowVisualization->SetInputConnection(input);
  m_OutlineVisualization->SetInputConnection(input);
  m_ImagePlaneVisualization->SetInputConnection(input);  
}


void
Visualization
::SetFilteredImageInputConnection(vtkAlgorithmOutput* input) {
  input->GetProducer()->Modified();
  input->GetProducer()->Update();
  m_IsoVisualization->SetInputConnection(input);
}


void
Visualization
::AddToRenderer(vtkRenderer* renderer) {
  m_ArrowVisualization->AddToRenderer(renderer);
  m_OutlineVisualization->AddToRenderer(renderer);
  m_IsoVisualization->AddToRenderer(renderer);
  m_ImagePlaneVisualization->AddToRenderer(renderer);
}


void
Visualization
::SetShowOutline(bool show) {
  m_OutlineVisualization->SetVisible(show);
}


bool
Visualization
::GetShowOutline() {
  return m_OutlineVisualization->GetVisible();
}


void
Visualization
::ShowOutlineOn() {
  SetShowOutline(true);
}


void
Visualization
::ShowOutlineOff() {
  SetShowOutline(false);
}


void
Visualization
::SetIsosurfaceVisible(bool show) {
  m_IsoVisualization->SetVisible(show);
}


bool
Visualization
::GetIsosurfaceVisible() {
  return m_IsoVisualization->GetVisible();
}


void
Visualization
::SetIsoValue(double isoValue) {
  m_IsoVisualization->SetIsoValue(isoValue);
}


double
Visualization
::GetIsoValue() {
  return m_IsoVisualization->GetIsoValue();
}


vtkAlgorithmOutput*
Visualization
::GetIsosurfaceOutputPort() {
  return m_IsoVisualization->GetIsosurfaceOutputPort();
}


void
Visualization
::SetFastIsosurfaceRendering(bool toggle) {
  if (toggle)
    FastIsosurfaceRenderingOn();
  else
    FastIsosurfaceRenderingOff();
}


bool
Visualization
::GetFastIsosurfaceRendering() {
  return m_IsoVisualization->GetFastRenderingOn();
}


void
Visualization
::FastIsosurfaceRenderingOn() {
  m_IsoVisualization->FastRenderingOn();
}


void
Visualization
::FastIsosurfaceRenderingOff() {
  m_IsoVisualization->FastRenderingOff();
}


void
Visualization
::SetZSlice(int slice) {
  m_ImagePlaneVisualization->SetSliceNumber(slice);
  m_IsoVisualization->SetZPlane(slice);
}


int
Visualization
::GetZSlice() {
  return m_IsoVisualization->GetZPlane();
}


void
Visualization
::SetCropIsosurface(bool crop) {
  m_IsoVisualization->SetClipData(crop);
}


bool
Visualization
::GetCropIsosurface() {
  return m_IsoVisualization->GetClipData();
}


void
Visualization
::SetKeepPlanesAboveBelowImagePlane(int keep) {
  m_IsoVisualization->SetDeltaZ(keep);
}


int
Visualization
::GetKeepPlanesAboveBelowImagePlane() {
  return m_IsoVisualization->GetDeltaZ();
}


void
Visualization
::SetDirectionArrowAzimuth(double azimuth) {
  m_ArrowVisualization->SetAzimuth(azimuth);
}


double
Visualization
::GetDirectionArrowAzimuth() {
  return m_ArrowVisualization->GetAzimuth();
}


void
Visualization
::SetDirectionArrowInclination(double inclination) {
  m_ArrowVisualization->SetInclination(inclination);
}


double
Visualization
::GetDirectionArrowInclination() {
  return m_ArrowVisualization->GetInclination();
}


void
Visualization
::SetDirectionArrowCenter(double center[3]) {
  m_ArrowVisualization->SetCenter(center);
}


void
Visualization
::SetDirectionArrowScale(double scale) {
  m_ArrowVisualization->SetScale(scale);
}


void
Visualization
::SetDirectionArrowVisible(bool show) {
  m_ArrowVisualization->SetVisible(show);
}


bool
Visualization
::GetDirectionArrowVisible() {
  return m_ArrowVisualization->GetVisible();
}


void
Visualization
::SetImagePlaneVisible(bool show) {
  m_ImagePlaneVisualization->SetVisible(show);
}


bool
Visualization
::GetImagePlaneVisible() {
  return m_ImagePlaneVisualization->GetVisible();
}
