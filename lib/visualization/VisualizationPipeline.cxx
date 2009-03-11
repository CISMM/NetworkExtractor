#include "VisualizationPipeline.h"

#include <vtkAlgorithm.h>


VisualizationPipeline
::VisualizationPipeline() {
}


VisualizationPipeline
::~VisualizationPipeline() {
	
}


void
VisualizationPipeline
::SetInputConnection(vtkAlgorithmOutput* input) {
  this->inputAlgorithm->SetInputConnection(input);
}


void
VisualizationPipeline
::SetInputAlgorithm(vtkAlgorithm* algorithm) {
  this->inputAlgorithm = algorithm;
}