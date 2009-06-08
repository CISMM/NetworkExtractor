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
::SetInputConnection(vtkAlgorithmOutput* input) {
  if (input != this->outlineVisualization->GetInputConnection())
    this->outlineVisualization->SetInputConnection(input);

  if (input == this->isoVisualization->GetInputConnection())
    this->isoVisualization->SetInputConnection(input);

  if (input == this->imagePlaneVisualization->GetInputConnection())
    this->imagePlaneVisualization->SetInputConnection(input);

}


void
Visualization
::AddToRenderer(vtkRenderer* renderer) {
  vtkAlgorithmOutput *visInput = NULL; 
  visInput = this->outlineVisualization->GetInputConnection();
  this->outlineVisualization->AddToRenderer(renderer);

  this->isoVisualization->SetInputConnection(visInput);
  this->isoVisualization->AddToRenderer(renderer);

  this->imagePlaneVisualization->SetInputConnection(visInput);
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
}


void
Visualization
::SetImagePlaneVisible(bool show) {
  this->imagePlaneVisualization->SetVisible(show);
}
