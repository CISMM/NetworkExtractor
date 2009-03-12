#include "Visualization.h"

Visualization
::Visualization() {
  this->showOutline = true;

  this->outlineVisualization = new OutlineVisualizationPipeline();
  this->isoVisualization = new IsosurfaceVisualizationPipeline();
}


Visualization
::~Visualization() {
  delete this->outlineVisualization;
  delete this->isoVisualization;
}


void
Visualization
::SetInputConnection(vtkAlgorithmOutput* input) {
  this->isoVisualization->SetInputConnection(input);
  this->outlineVisualization->SetInputConnection(input);
}


void
Visualization
::AddToRenderer(vtkRenderer* renderer) {
  vtkAlgorithmOutput *visInput = NULL;
 
  // if (visualizationMode == ...)
  visInput = this->isoVisualization->GetInputConnection();
  this->isoVisualization->AddToRenderer(renderer);

  this->outlineVisualization->SetInputConnection(visInput);
  this->outlineVisualization->AddToRenderer(renderer);
}


void
Visualization
::SetShowOutline(bool show) {
  this->showOutline = show;
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
::SetIsoValue(double isoValue) {
  this->isoVisualization->SetIsoValue(isoValue);
}


double
Visualization
::GetIsoValue() {
  return this->isoVisualization->GetIsoValue();
}