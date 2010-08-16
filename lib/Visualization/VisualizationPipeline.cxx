#include "VisualizationPipeline.h"

#include <vtkActor.h>
#include <vtkAlgorithm.h>
#include <vtkPolyDataMapper.h>
#include <vtkOutlineFilter.h>
#include <vtkRenderer.h>


VisualizationPipeline
::VisualizationPipeline() {
  m_Input = NULL;
  m_InputAlgorithm = NULL;
}


VisualizationPipeline
::~VisualizationPipeline() {
}


void
VisualizationPipeline
::SetInputConnection(vtkAlgorithmOutput* input) {
  m_Input = input;

  if (m_InputAlgorithm) {
    m_InputAlgorithm->SetInputConnection(m_Input);
  
    // Need this here for visualizations to update properly.
    m_InputAlgorithm->Update();
  }
}


void
VisualizationPipeline
::SetInputAlgorithm(vtkAlgorithm* algorithm) {
  m_InputAlgorithm = algorithm;
}


vtkAlgorithmOutput*
VisualizationPipeline
::GetInputConnection() {
  return m_Input;
}
