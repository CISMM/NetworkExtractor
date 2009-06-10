#include "IsosurfaceVisualizationPipeline.h"

#include <vtkActor.h>
#include <vtkAlgorithmOutput.h>
#include <vtkContourFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>


IsosurfaceVisualizationPipeline
::IsosurfaceVisualizationPipeline() {
  this->isoValue = 0.0;
  
  // Set up pipeline.
  this->isoContourer = vtkContourFilter::New();
  this->isoContourer->SetNumberOfContours(1);
  this->isoContourer->SetValue(0, this->isoValue);

  // It is essential to set the inputAlgorithm
  this->SetInputAlgorithm(isoContourer);

  this->isoMapper = vtkPolyDataMapper::New();
  this->isoMapper->SetInputConnection(this->inputAlgorithm->GetOutputPort());
  this->isoMapper->ScalarVisibilityOff();
  this->isoMapper->ImmediateModeRenderingOn();

  this->isoActor = vtkActor::New();
  this->isoActor->SetMapper(this->isoMapper);
}


IsosurfaceVisualizationPipeline
::~IsosurfaceVisualizationPipeline() {
}


void
IsosurfaceVisualizationPipeline
::AddToRenderer(vtkRenderer* renderer) {
  this->isoContourer->Modified();
  renderer->AddActor(this->isoActor);
}


void
IsosurfaceVisualizationPipeline
::SetColor(double r, double g, double b) {
  this->isoActor->GetProperty()->SetColor(r, g, b);
}


void
IsosurfaceVisualizationPipeline
::SetVisible(bool visible) {
  this->isoActor->SetVisibility(visible ? 1 : 0);
}


void
IsosurfaceVisualizationPipeline
::SetIsoValue(double isoValue) {
  this->isoValue = isoValue;
  this->isoContourer->SetValue(0, this->isoValue);
  this->isoContourer->Modified();
}


double
IsosurfaceVisualizationPipeline
::GetIsoValue() {
  return this->isoValue;
}


vtkAlgorithmOutput*
IsosurfaceVisualizationPipeline
::GetIsosurfaceOutputPort() {
  return this->isoContourer->GetOutputPort();
}


void
IsosurfaceVisualizationPipeline
::FastRenderingOn() {
  this->isoMapper->ImmediateModeRenderingOff();
}


void
IsosurfaceVisualizationPipeline
::FastRenderingOff() {
  this->isoMapper->ImmediateModeRenderingOn();
}
