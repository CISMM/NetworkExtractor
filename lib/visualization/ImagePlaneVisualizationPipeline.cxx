#include "ImagePlaneVisualizationPipeline.h"

#include <vtkImageActor.h>
#include <vtkImageData.h>
#include <vtkImageShiftScale.h>
#include <vtkRenderer.h>


ImagePlaneVisualizationPipeline
::ImagePlaneVisualizationPipeline() {

  // Set up pipeline.
  this->typeCaster = vtkSmartPointer<vtkImageShiftScale>::New();
  this->typeCaster->SetOutputScalarTypeToUnsignedChar();

  // It is essential to set the input algorithm.
  this->SetInputAlgorithm(this->typeCaster);

  this->imageActor = vtkSmartPointer<vtkImageActor>::New();
  this->imageActor->InterpolateOn();
  this->imageActor->SetInput(this->typeCaster->GetOutput());
}


ImagePlaneVisualizationPipeline
::~ImagePlaneVisualizationPipeline() {
}


void
ImagePlaneVisualizationPipeline
::SetInputConnection(vtkAlgorithmOutput* input) {
  this->input = input;
  this->inputAlgorithm->SetInputConnection(input);
}


void
ImagePlaneVisualizationPipeline
::AddToRenderer(vtkRenderer* renderer) {
  renderer->AddActor(this->imageActor);
}


void
ImagePlaneVisualizationPipeline
::SetVisible(bool visible) {
  this->imageActor->SetVisibility(visible ? 1 : 0);
}


void
ImagePlaneVisualizationPipeline
::SetSliceNumber(int sliceNumber) {
  int* extent = this->imageActor->GetInput()->GetWholeExtent();
  this->imageActor->SetDisplayExtent(extent[0], extent[1],
				     extent[2], extent[3],
				     sliceNumber, sliceNumber);
  this->imageActor->Modified();
}


int
ImagePlaneVisualizationPipeline
::GetSliceNumber() {
  return this->imageActor->GetZSlice();
}
