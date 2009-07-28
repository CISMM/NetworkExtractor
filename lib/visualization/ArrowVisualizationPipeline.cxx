#include "ArrowVisualizationPipeline.h"

#include <vtkActor.h>
#include <vtkAlgorithmOutput.h>
#include <vtkArrowSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>


ArrowVisualizationPipeline
::ArrowVisualizationPipeline() {
  m_Azimuth = 0.0;
  m_Inclination = 0.0;

  m_ArrowSource = vtkSmartPointer<vtkArrowSource>::New();

  m_Mapper      = vtkSmartPointer<vtkPolyDataMapper>::New();
  m_Mapper->SetInputConnection(m_ArrowSource->GetOutputPort());

  m_Actor       = vtkSmartPointer<vtkActor>::New();
  m_Actor->SetMapper(m_Mapper);
}


ArrowVisualizationPipeline
::~ArrowVisualizationPipeline() {
}


void
ArrowVisualizationPipeline
::AddToRenderer(vtkRenderer* renderer) {
  renderer->AddActor(m_Actor);
}


void
ArrowVisualizationPipeline
::SetAzimuth(double azimuth) {
  m_Azimuth = azimuth;
  m_Actor->SetOrientation(0.0, -m_Inclination, m_Azimuth);
}


double
ArrowVisualizationPipeline
::GetAzimuth() {
  return m_Azimuth;
}


void
ArrowVisualizationPipeline
::SetInclination(double inclination) {
  m_Inclination = inclination;
  m_Actor->SetOrientation(0.0, -m_Inclination, m_Azimuth);
}


double
ArrowVisualizationPipeline
::GetInclination() {
  return m_Inclination;
}


void
ArrowVisualizationPipeline
::SetCenter(double center[3]) {
  m_Actor->SetPosition(center);
}


void
ArrowVisualizationPipeline
::SetScale(double scale) {
  m_Actor->SetScale(scale, scale, scale);
}


void
ArrowVisualizationPipeline
::SetVisible(bool visible) {
  m_Actor->SetVisibility(visible ? 1 : 0);
}


bool
ArrowVisualizationPipeline
::GetVisible() {
  return m_Actor->GetVisibility() == 1;
}
