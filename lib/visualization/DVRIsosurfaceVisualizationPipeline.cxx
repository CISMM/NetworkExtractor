#include "DVRIsosurfaceVisualizationPipeline.h"

#include "vtkActor.h"
#include "vtkAlgorithmOutput.h"
#include "vtkColorTransferFunction.h"
#include "vtkImageClip.h"
#include "vtkImageData.h"
#include "vtkImageShiftScale.h"
#include "vtkPiecewiseFunction.h"
#include "vtkRenderer.h"
#include "vtkVolume.h"
#include "vtkVolumeProperty.h"


#include "vtkVolumeRayCastMapper.h"
#include "vtkVolumeRayCastIsosurfaceFunction.h"


DVRIsosurfaceVisualizationPipeline
::DVRIsosurfaceVisualizationPipeline() {
  m_IsoValue = 0.0;
  m_ZPlane = 0;
  m_DeltaZ = 2;

  // Set up pipeline.
  m_ShiftScaler = vtkSmartPointer<vtkImageShiftScale>::New();
  m_ShiftScaler->SetOutputScalarTypeToUnsignedShort();
  m_ShiftScaler->ClampOverflowOn();

  // It is essential to set the inputAlgorithm
  SetInputAlgorithm(m_ShiftScaler);

  m_ImageClip = vtkSmartPointer<vtkImageClip>::New();
  m_ImageClip->ClipDataOff();
  m_ImageClip->SetInputConnection(m_ShiftScaler->GetOutputPort());

  m_OpacityTransferFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
  m_OpacityTransferFunction->AddPoint(0, 1.0);
  m_OpacityTransferFunction->AddPoint(1.0e9, 1.0);  

  m_ColorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
  m_ColorTransferFunction->AddRGBPoint(0.0, 0.0, 1.0, 0.0);
  m_ColorTransferFunction->AddRGBPoint(1.0e9, 0.0, 1.0, 0.0);
  
  m_VolumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
  m_VolumeProperty->SetColor(m_ColorTransferFunction);
  m_VolumeProperty->SetScalarOpacity(m_OpacityTransferFunction);
  m_VolumeProperty->SetInterpolationTypeToLinear();
  m_VolumeProperty->ShadeOn();

  m_RayCastFunction = vtkSmartPointer<vtkVolumeRayCastIsosurfaceFunction>::New();
  m_RayCastFunction->SetIsoValue(200.0);

  m_VolumeMapper = vtkSmartPointer<vtkVolumeRayCastMapper>::New();
  m_VolumeMapper->AutoAdjustSampleDistancesOn();
  m_VolumeMapper->IntermixIntersectingGeometryOn();
  m_VolumeMapper->SetVolumeRayCastFunction(m_RayCastFunction);
  m_VolumeMapper->SetInputConnection(m_ImageClip->GetOutputPort());

  m_Volume = vtkSmartPointer<vtkVolume>::New();
  m_Volume->SetMapper(m_VolumeMapper);
  m_Volume->SetProperty(m_VolumeProperty);
}


DVRIsosurfaceVisualizationPipeline
::~DVRIsosurfaceVisualizationPipeline() {

}


#if 0
void
DVRIsosurfaceVisualizationPipeline
::SetInputConnection(vtkAlgorithmOutput* input) {
  m_Input = input;
  m_InputAlgorithm->SetInputConnection(input);

  // Gotta be a better way to do this.
  input->GetProducer()->Update();
  vtkImageData* originalImage 
    = vtkImageData::SafeDownCast(input->GetProducer()->GetOutputDataObject(0));
  double* scalarRange = originalImage->GetScalarRange();
  //m_ShiftScaler->SetShift(-scalarRange[0]);
  //m_ShiftScaler->SetScale(255.0 / (scalarRange[1] - scalarRange[0]));
  m_ShiftScaler->UpdateWholeExtent();
}
#endif


void
DVRIsosurfaceVisualizationPipeline
::AddToRenderer(vtkRenderer* renderer) {
  renderer->AddVolume(m_Volume);
}


void
DVRIsosurfaceVisualizationPipeline
::SetColor(double r, double g, double b) {
  double nodeValue[6];
  m_ColorTransferFunction->GetNodeValue(0, nodeValue);
  nodeValue[1] = r;
  nodeValue[2] = g;
  nodeValue[3] = b;
  m_ColorTransferFunction->SetNodeValue(0, nodeValue);

  m_ColorTransferFunction->GetNodeValue(1, nodeValue);
  nodeValue[1] = r;
  nodeValue[2] = g;
  nodeValue[3] = b;
  m_ColorTransferFunction->SetNodeValue(1, nodeValue);
}


void
DVRIsosurfaceVisualizationPipeline
::SetVisible(bool visible) {
  m_Volume->SetVisibility(visible ? 1 : 0);
}


bool
DVRIsosurfaceVisualizationPipeline
::GetVisible() {
  return m_Volume->GetVisibility() == 1;
}


void
DVRIsosurfaceVisualizationPipeline
::ClipDataOn() {
  m_ImageClip->ClipDataOn();
}


void
DVRIsosurfaceVisualizationPipeline
::ClipDataOff() {
  m_ImageClip->ClipDataOff();
}


void
DVRIsosurfaceVisualizationPipeline
::SetClipData(bool clip) {
  m_ImageClip->SetClipData(clip ? 1: 0);
}


bool
DVRIsosurfaceVisualizationPipeline
::GetClipData() {
  return m_ImageClip->GetClipData() == 1;
}


void
DVRIsosurfaceVisualizationPipeline
::UpdateClipping() {
  m_InputAlgorithm->GetInputDataObject(0, 0)->Update();
  int *extent = m_InputAlgorithm->GetInputDataObject(0, 0)->GetWholeExtent();
  int dims[3];
  dims[0] = extent[1] - extent[0] + 1;
  dims[1] = extent[3] - extent[2] + 1;
  dims[2] = extent[5] - extent[4] + 1;

  if (m_ImageClip->GetClipData()) {
    int minZ = m_ZPlane - m_DeltaZ;
    if (minZ < 0) 
      minZ = 0;
    int maxZ = m_ZPlane + m_DeltaZ;
    if (maxZ >= dims[2]) 
      maxZ = dims[2]-1;
    m_ImageClip->SetOutputWholeExtent(0, dims[0]-1, 0, dims[1]-1, minZ, maxZ);
  } else {
    m_ImageClip->SetOutputWholeExtent(0, dims[0]-1, 0, dims[1]-1, 0, dims[2]-1);
  }
}


void
DVRIsosurfaceVisualizationPipeline
::SetZPlane(int zPlane) {
  m_ZPlane = zPlane;

  UpdateClipping();
}


int
DVRIsosurfaceVisualizationPipeline
::GetZPlane() {
  return m_ZPlane;
}


void
DVRIsosurfaceVisualizationPipeline
::SetDeltaZ(int deltaZ) {
  m_DeltaZ = deltaZ;
}


int
DVRIsosurfaceVisualizationPipeline
::GetDeltaZ() {
  return m_DeltaZ;
}


void
DVRIsosurfaceVisualizationPipeline
::SetIsoValue(double isoValue) {
  m_IsoValue = isoValue;
  double nodeValue[4];

  m_RayCastFunction->SetIsoValue(isoValue);
}


double
DVRIsosurfaceVisualizationPipeline
::GetIsoValue() {
  return m_IsoValue;
}
