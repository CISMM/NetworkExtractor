#include "IsosurfaceVisualizationPipeline.h"

#include <vtkActor.h>
#include <vtkAlgorithmOutput.h>
#include <vtkContourFilter.h>
#include <vtkPolyDataMapper.h>
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
  this->inputAlgorithm = isoContourer;

  this->isoMapper = vtkPolyDataMapper::New();
  this->isoMapper->SetInputConnection(this->inputAlgorithm->GetOutputPort());
  this->isoMapper->ScalarVisibilityOff();

  this->isoActor = vtkActor::New();
  this->isoActor->SetMapper(this->isoMapper);
}


IsosurfaceVisualizationPipeline
::~IsosurfaceVisualizationPipeline() {
}


void
IsosurfaceVisualizationPipeline
::AddToRenderer(vtkRenderer* renderer) {
  renderer->AddActor(this->isoActor);
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
