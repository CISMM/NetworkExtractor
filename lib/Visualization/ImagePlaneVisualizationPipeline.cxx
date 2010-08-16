#include "ImagePlaneVisualizationPipeline.h"

#include <vtkAlgorithmOutput.h>
#include <vtkImageActor.h>
#include <vtkImageData.h>
#include <vtkImageShiftScale.h>
#include <vtkRenderer.h>


ImagePlaneVisualizationPipeline
::ImagePlaneVisualizationPipeline() {
  // Set up pipeline.
  m_ShiftScaler = vtkSmartPointer<vtkImageShiftScale>::New();
  m_ShiftScaler->SetOutputScalarTypeToUnsignedChar();
  m_ShiftScaler->ClampOverflowOn();

  // It is essential to set the input algorithm.
  SetInputAlgorithm(m_ShiftScaler);

  m_ImageActor = vtkSmartPointer<vtkImageActor>::New();
  m_ImageActor->InterpolateOn();
  m_ImageActor->SetInput(m_ShiftScaler->GetOutput());
}


ImagePlaneVisualizationPipeline
::~ImagePlaneVisualizationPipeline() {
}


void
ImagePlaneVisualizationPipeline
::SetInputConnection(vtkAlgorithmOutput* input) {
  m_Input = input;
  m_InputAlgorithm->SetInputConnection(input);

    // Update shift/scale filter automatically if auto-rescaling is on.
  if (m_AutoRescalingOn) {
    // Gotta be a better way to do this.
    input->GetProducer()->Update();
    vtkImageData* originalImage 
      = vtkImageData::SafeDownCast(input->GetProducer()->GetOutputDataObject(0));
    double* scalarRange = originalImage->GetScalarRange();
    m_ShiftScaler->SetShift(-scalarRange[0]);
    m_ShiftScaler->SetScale(255.0 / (scalarRange[1] - scalarRange[0]));
  } else {
    m_ShiftScaler->SetShift(0.0);
    m_ShiftScaler->SetScale(1.0);
  }
  m_ShiftScaler->UpdateWholeExtent();
}


void
ImagePlaneVisualizationPipeline
::AddToRenderer(vtkRenderer* renderer) {
  renderer->AddActor(m_ImageActor);
}


void
ImagePlaneVisualizationPipeline
::SetVisible(bool visible) {
  m_ImageActor->SetVisibility(visible ? 1 : 0);
}


bool
ImagePlaneVisualizationPipeline
::GetVisible() {
  return m_ImageActor->GetVisibility() == 1;
}


void
ImagePlaneVisualizationPipeline
::SetSliceNumber(int sliceNumber) {
  m_ShiftScaler->UpdateWholeExtent();
  int* extent = m_ImageActor->GetInput()->GetWholeExtent();
  m_ImageActor->SetDisplayExtent(extent[0], extent[1],
			       extent[2], extent[3],
			       sliceNumber, sliceNumber);
  m_ImageActor->Modified();
}


int
ImagePlaneVisualizationPipeline
::GetSliceNumber() {
  return m_ImageActor->GetZSlice();
}
