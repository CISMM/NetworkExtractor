#include "OutlineVisualizationPipeline.h"


OutlineVisualizationPipeline
::OutlineVisualizationPipeline() {
  m_OutlineFilter = vtkOutlineFilter::New();

  this->SetInputAlgorithm(m_OutlineFilter);

  m_OutlineMapper = vtkPolyDataMapper::New();
  m_OutlineMapper->SetInputConnection(m_OutlineFilter->GetOutputPort());
  m_OutlineActor = vtkActor::New();
  m_OutlineActor->SetMapper(m_OutlineMapper);
}


OutlineVisualizationPipeline
::~OutlineVisualizationPipeline() {
  m_OutlineFilter->Delete();
  m_OutlineMapper->Delete();
  m_OutlineActor->Delete();
}


void
OutlineVisualizationPipeline
::AddToRenderer(vtkRenderer* renderer) {
  renderer->AddActor(m_OutlineActor);
}


void
OutlineVisualizationPipeline
::SetVisible(bool visible) {
  m_OutlineActor->SetVisibility(static_cast<int>(visible));
}


bool
OutlineVisualizationPipeline
::GetVisible() {
  return m_OutlineActor->GetVisibility() == 1;
}

