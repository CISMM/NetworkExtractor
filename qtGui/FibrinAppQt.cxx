#include "FibrinAppQT.h"

#include <qapplication.h>
#include <qfiledialog.h>

#include <vtkActor.h>
#include <vtkContourFilter.h>
#include <vtkImageImport.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

#include <itkMinimumMaximumImageCalculator.h>
#include <itkSize.h>

#include "LoadVTKImageFile.h"


// Constructor
FibrinAppQt::FibrinAppQt(QWidget* p) 
 : QMainWindow(p) {
  setupUi(this);

  // QT/VTK interact
  ren = vtkRenderer::New();
  qvtkWidget->GetRenderWindow()->AddRenderer(ren);

  // Hook up menus signals to slots
  connect(actionOpenImage, SIGNAL(triggered()), this, SLOT(fileOpenImage()));
  connect(actionOpenView, SIGNAL(triggered()), this, SLOT(fileOpenView()));
  connect(actionSaveView, SIGNAL(triggered()), this, SLOT(fileSaveView()));
  connect(actionExit, SIGNAL(triggered()), this, SLOT(fileExit()));
  connect(applyButton, SIGNAL(clicked()), this, SLOT(applyButtonHandler()));
  
};


// Action to be taken upon file open 
void FibrinAppQt::fileOpenImage() {

  // Locate file.
  QString fileName = QFileDialog::getOpenFileName(this, "Open Image Data", "", "VTK Images (*.vtk);;");

  // Now read the file
  if (fileName != "") {
    std::cout << "Loading file '" << fileName.toStdString() << "'" << std::endl;
    LoadVTKImageFile *loader = new LoadVTKImageFile();
    imageData = loader->LoadFile(fileName.toStdString());
    delete loader;
  }

  // Set status bar with info about the file.
  QString imageInfo("Loaded image '");
  imageInfo.append(fileName);
  imageInfo.append("'.");
  statusbar->showMessage(imageInfo);

  // Show image information in GUI.
  typedef itk::MinimumMaximumImageCalculator<UShort3DImageType> MinMaxType;
  MinMaxType::Pointer minMax = MinMaxType::New();
  minMax->SetImage(imageData);
  minMax->Compute();
  std::stringstream dataMin; dataMin << minMax->GetMinimum();
  dataRangeMin->setText(QString(dataMin.str().c_str()));
  std::stringstream dataMax; dataMax << minMax->GetMaximum();
  dataRangeMax->setText(QString(dataMax.str().c_str()));

  UShort3DImageType::RegionType region = imageData->GetLargestPossibleRegion();
  itk::Size<3> size = region.GetSize();

  std::stringstream xMin; xMin << size[0];
  xRangeMin->setText(QString("0"));
  xRangeMax->setText(QString(xMin.str().c_str()));

  std::stringstream yMin; yMin << size[1];
  yRangeMin->setText(QString("0"));
  yRangeMax->setText(QString(yMin.str().c_str()));

  std::stringstream zMin; zMin << size[2];
  zRangeMin->setText(QString("0"));
  zRangeMax->setText(QString(zMin.str().c_str()));

  itk::Vector<double> spacing = imageData->GetSpacing();
  std::stringstream xSpacing; xSpacing << spacing[0];
  xSpacingEdit->setText(QString(xSpacing.str().c_str()));

  std::stringstream ySpacing; ySpacing << spacing[1];
  ySpacingEdit->setText(QString(ySpacing.str().c_str()));

  std::stringstream zSpacing; zSpacing << spacing[2];
  zSpacingEdit->setText(QString(zSpacing.str().c_str()));

  // Move data from ITK to VTK and display an isosurface.
  exporter = UShort3DExporterType::New();
  exporter->SetInput(imageData);

  // Set isovalue to midpoint between data min and max
  double isoValue = 0.5*(minMax->GetMinimum() + minMax->GetMaximum());
  std::stringstream isoValueSS; isoValueSS << isoValue;
  isoValueEdit->setText(QString(isoValueSS.str().c_str()));

  // Geometry
  vtkImageImport* imageImporter = vtkImageImport::New();
  imageImporter->SetUpdateInformationCallback(exporter->GetUpdateInformationCallback());
  imageImporter->SetPipelineModifiedCallback(exporter->GetPipelineModifiedCallback());
  imageImporter->SetWholeExtentCallback(exporter->GetWholeExtentCallback());
  imageImporter->SetSpacingCallback(exporter->GetSpacingCallback());
  imageImporter->SetOriginCallback(exporter->GetOriginCallback());
  imageImporter->SetScalarTypeCallback(exporter->GetScalarTypeCallback());
  imageImporter->SetNumberOfComponentsCallback(exporter->GetNumberOfComponentsCallback());
  imageImporter->SetPropagateUpdateExtentCallback(exporter->GetPropagateUpdateExtentCallback());
  imageImporter->SetUpdateDataCallback(exporter->GetUpdateDataCallback());
  imageImporter->SetDataExtentCallback(exporter->GetDataExtentCallback());
  imageImporter->SetBufferPointerCallback(exporter->GetBufferPointerCallback());
  imageImporter->SetCallbackUserData(exporter->GetCallbackUserData());

  // Iso surfacer
  contourer = vtkContourFilter::New();
  contourer->SetInputConnection(imageImporter->GetOutputPort());
  contourer->SetNumberOfContours(1);
  contourer->SetValue(0, isoValue);

  // Mapper
  mapper = vtkPolyDataMapper::New();
  mapper->ScalarVisibilityOff();
  mapper->SetInputConnection(contourer->GetOutputPort());

  // Actor in scene
  actor = vtkActor::New();
  actor->SetMapper(mapper);

  // Add Actor to renderer
  ren->AddActor(actor);

  // Reset camera
  ren->ResetCamera();
}


void FibrinAppQt::fileOpenView() {

}


void FibrinAppQt::fileSaveView() {

}


void FibrinAppQt::fileExit() {
  qApp->exit();
}


void FibrinAppQt::applyButtonHandler() {
  // Read isovalue.
  double isoValue = isoValueEdit->text().toDouble();
  contourer->SetValue(0, isoValue);
  ren->Render();
}
