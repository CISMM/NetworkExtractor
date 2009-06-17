#include "Visualization.h"

Visualization
::Visualization() {
  this->outlineVisualization = new OutlineVisualizationPipeline();
  this->isoVisualization = new IsosurfaceVisualizationPipeline();
  this->isoVisualization->SetColor(0.0, 1.0, 0.0);
  this->imagePlaneVisualization = new ImagePlaneVisualizationPipeline();
}


Visualization
::~Visualization() {
  delete this->outlineVisualization;
  delete this->isoVisualization;
  delete this->imagePlaneVisualization;
}


void
Visualization
::SetImageInputConnection(vtkAlgorithmOutput* input) {
  this->outlineVisualization->SetInputConnection(input);
  this->imagePlaneVisualization->SetInputConnection(input);  
}


void
Visualization
::SetFilteredImageInputConnection(vtkAlgorithmOutput* input) {
  this->isoVisualization->SetInputConnection(input);
}


void
Visualization
::AddToRenderer(vtkRenderer* renderer) {
  this->outlineVisualization->AddToRenderer(renderer);
  this->isoVisualization->AddToRenderer(renderer);
  this->imagePlaneVisualization->AddToRenderer(renderer);
}


void
Visualization
::SetShowOutline(bool show) {
  this->outlineVisualization->SetVisible(show);
}


void
Visualization
::ShowOutlineOn() {
  this->SetShowOutline(true);
}


void
Visualization
::ShowOutlineOff() {
  this->SetShowOutline(false);
}


void
Visualization
::SetIsosurfaceVisible(bool show) {
  this->isoVisualization->SetVisible(show);
}


void
Visualization
::SetIsoValue(double isoValue) {
  this->isoVisualization->SetIsoValue(isoValue);
}


double
Visualization
::GetIsoValue() {
  return this->isoVisualization->GetIsoValue();
}


vtkAlgorithmOutput*
Visualization
::GetIsosurfaceOutputPort() {
  return this->isoVisualization->GetIsosurfaceOutputPort();
}


void
Visualization
::SetFastIsosurfaceRendering(bool toggle) {
  if (toggle)
    FastIsosurfaceRenderingOn();
  else
    FastIsosurfaceRenderingOff();
}


void
Visualization
::FastIsosurfaceRenderingOn() {
  this->isoVisualization->FastRenderingOn();
}


void
Visualization
::FastIsosurfaceRenderingOff() {
  this->isoVisualization->FastRenderingOff();
}


void
Visualization
::SetZSlice(int slice) {
  this->imagePlaneVisualization->SetSliceNumber(slice);
  this->isoVisualization->SetZPlane(slice);
}


int
Visualization
::GetZSlice() {
  return this->isoVisualization->GetZPlane();
}


void
Visualization
::SetCropIsosurface(bool crop) {
  this->isoVisualization->SetClipData(crop);
}


bool
Visualization
::GetCropIsosurface() {
  return this->isoVisualization->GetClipData();
}


void
Visualization
::SetKeepPlanesAboveBelowImagePlane(int keep) {
  this->isoVisualization->SetDeltaZ(keep);
}


int
Visualization
::GetKeepPlanesAboveBelowImagePlane() {
  return this->isoVisualization->GetDeltaZ();
}


void
Visualization
::SetImagePlaneVisible(bool show) {
  this->imagePlaneVisualization->SetVisible(show);
}


bool
Visualization
::GetImagePlaneVisible() {
  return this->imagePlaneVisualization->GetVisible();
}
