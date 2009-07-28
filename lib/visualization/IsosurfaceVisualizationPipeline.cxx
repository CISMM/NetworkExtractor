#include "IsosurfaceVisualizationPipeline.h"

#include <vtkActor.h>
#include <vtkAlgorithmOutput.h>
#include <vtkContourFilter.h>
#include <vtkImageClip.h>
#include <vtkImageData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>


IsosurfaceVisualizationPipeline
::IsosurfaceVisualizationPipeline() {
  m_IsoValue = 0.0;
  m_ZPlane = 0;
  m_DeltaZ = 2;
  
  // Set up pipeline.
  m_ImageClip = vtkImageClip::New();
  m_ImageClip->ClipDataOff();

  m_IsoContourer = vtkContourFilter::New();
  m_IsoContourer->SetNumberOfContours(1);
  m_IsoContourer->SetValue(0, m_IsoValue);
  m_IsoContourer->SetInputConnection(m_ImageClip->GetOutputPort());

  m_IsoMapper = vtkPolyDataMapper::New();
  m_IsoMapper->ScalarVisibilityOff();
  m_IsoMapper->ImmediateModeRenderingOff();
  m_IsoMapper->SetInputConnection(m_IsoContourer->GetOutputPort()); //

  m_IsoActor = vtkActor::New();
  m_IsoActor->SetMapper(m_IsoMapper);

  // It is essential to set the inputAlgorithm
  SetInputAlgorithm(m_ImageClip);

}


IsosurfaceVisualizationPipeline
::~IsosurfaceVisualizationPipeline() {
}


void
IsosurfaceVisualizationPipeline
::AddToRenderer(vtkRenderer* renderer) {
  m_IsoContourer->Modified();
  renderer->AddActor(m_IsoActor);
}


void
IsosurfaceVisualizationPipeline
::SetColor(double r, double g, double b) {
  m_IsoActor->GetProperty()->SetColor(r, g, b);
}


void
IsosurfaceVisualizationPipeline
::SetVisible(bool visible) {
  m_IsoActor->SetVisibility(visible ? 1 : 0);
}


bool
IsosurfaceVisualizationPipeline
::GetVisible() {
  return m_IsoActor->GetVisibility() == 1;
}


void
IsosurfaceVisualizationPipeline
::ClipDataOn() {
  m_ImageClip->ClipDataOn();
}


void
IsosurfaceVisualizationPipeline
::ClipDataOff() {
  m_ImageClip->ClipDataOff();
}


void
IsosurfaceVisualizationPipeline
::SetClipData(bool clip) {
  m_ImageClip->SetClipData(clip ? 1 : 0);
}


bool
IsosurfaceVisualizationPipeline
::GetClipData() {
  return m_ImageClip->GetClipData() == 1;
}


void
IsosurfaceVisualizationPipeline
::SetZPlane(int zPlane) {
  m_ZPlane = zPlane;
  int dims[3];
  m_ImageClip->GetImageDataInput(0)->GetDimensions(dims);

  if (m_ImageClip->GetClipData()) {
    int minZ = zPlane - m_DeltaZ;
    if (minZ < 0) 
      minZ = 0;
    int maxZ = zPlane + m_DeltaZ;
    if (maxZ >= dims[2]) 
      maxZ = dims[2]-1;
    m_ImageClip->SetOutputWholeExtent(0, dims[0]-1, 0, dims[1]-1, minZ, maxZ);
  } else {
    m_ImageClip->SetOutputWholeExtent(0, dims[0]-1, 0, dims[1]-1, 0, dims[2]-1);
  }

}


int
IsosurfaceVisualizationPipeline
::GetZPlane() {
  return m_ZPlane;
}


void
IsosurfaceVisualizationPipeline
::SetDeltaZ(int deltaZ) {
  m_DeltaZ = deltaZ;
}


int
IsosurfaceVisualizationPipeline
::GetDeltaZ() {
  return m_DeltaZ;
}


void
IsosurfaceVisualizationPipeline
::SetIsoValue(double isoValue) {
  m_IsoValue = isoValue;
  m_IsoContourer->SetValue(0, m_IsoValue);
  m_IsoContourer->Modified();
}


double
IsosurfaceVisualizationPipeline
::GetIsoValue() {
  return m_IsoValue;
}


vtkAlgorithmOutput*
IsosurfaceVisualizationPipeline
::GetIsosurfaceOutputPort() {
  return m_IsoContourer->GetOutputPort();
}


void
IsosurfaceVisualizationPipeline
::FastRenderingOn() {
  m_IsoMapper->ImmediateModeRenderingOff();
}


void
IsosurfaceVisualizationPipeline
::FastRenderingOff() {
  m_IsoMapper->ImmediateModeRenderingOn();
}


bool
IsosurfaceVisualizationPipeline
::GetFastRenderingOn() {
  return m_IsoMapper->GetImmediateModeRendering() == 0;
}

