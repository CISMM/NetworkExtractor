#include "FibrinAppQt.h"

#include <qapplication.h>
#include <qfiledialog.h>
#include <qvariant.h>

#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkContourFilter.h>
#include <vtkImageImport.h>
#include <vtkPNGWriter.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataWriter.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkWindowToImageFilter.h>

#include <itkSize.h>

#include "IntensityThresholdThinningFilter.h"

// Constants
const std::string FibrinAppQt::NO_FILTER_STRING 
  = tr("No filter");
const std::string FibrinAppQt::VESSELNESS_FILTER_STRING 
  = tr("Vesselness");


void ProgressCallback(float progress) {
  QWidgetList list = QApplication::allWidgets();
  
  // I'm sure there is an easier way to do this, but this works for now.
  QProgressBar* progressBar = NULL;
  for (int i = 0; i < list.size(); i++) {
    QWidget* widget = list[i];
    if (widget->objectName().toStdString() == "progressBar") {
      progressBar = dynamic_cast<QProgressBar*>(widget);
    }
  }

  progressBar->setValue(static_cast<int>(100 * progress));
}


// Constructor
FibrinAppQt::FibrinAppQt(QWidget* p) 
 : QMainWindow(p) {
  setupUi(this);

  // QT/VTK interact
  this->ren = vtkRenderer::New();
  qvtkWidget->GetRenderWindow()->AddRenderer(ren);

  // Hook up menus signals to slots
  connect(actionOpenImage, SIGNAL(triggered()), this, SLOT(fileOpenImage()));
  connect(actionSaveFilteredImage, SIGNAL(triggered()), this, SLOT(fileSaveFilteredImage()));
  connect(actionSavePicture, SIGNAL(triggered()), this, SLOT(fileSavePicture()));
  connect(actionSaveRotationAnimation, SIGNAL(triggered()), this, SLOT(fileSaveRotationAnimation()));
  connect(actionSaveGeometry, SIGNAL(triggered()), this, SLOT(fileSaveGeometry()));
  connect(actionExit, SIGNAL(triggered()), this, SLOT(fileExit()));

  connect(actionResetView, SIGNAL(triggered()), this, SLOT(viewResetView()));
  connect(actionOpenView, SIGNAL(triggered()), this, SLOT(viewOpenView()));
  connect(actionSaveView, SIGNAL(triggered()), this, SLOT(viewSaveView()));

  connect(isoValueEdit, SIGNAL(textEdited(QString)), this, SLOT(isoValueEditHandler(QString)));
  connect(isoValueSlider, SIGNAL(sliderMoved(int)), this, SLOT(isoValueSliderHandler(int)));
  
  connect(showDataOutline, SIGNAL(toggled(bool)), this, SLOT(showDataOutlineHandler(bool)));

  connect(applyButton, SIGNAL(clicked()), this, SLOT(applyButtonHandler()));

  // Instantiate data model.
  this->dataModel = new DataModel<Float3DImageType>();
  this->dataModel->SetProgressCallback(ProgressCallback);

  // Instantiate visualization pipelines.
  this->visualization = new Visualization();

  this->imageFilterComboBox->addItem(QString(NO_FILTER_STRING.c_str()));
  this->imageFilterComboBox->addItem(QString(VESSELNESS_FILTER_STRING.c_str()));

  // Create and populate table model.
  this->tableModel = new QStandardItemModel(8, 2, this);
  this->tableModel->setHeaderData(0, Qt::Horizontal, tr("Property"));
  this->tableModel->setHeaderData(1, Qt::Horizontal, tr("Value"));

  this->tableModel->setItem(0, 0, new QStandardItem(tr("Data minimum")));
  this->tableModel->setItem(1, 0, new QStandardItem(tr("Data maximum")));
  this->tableModel->setItem(2, 0, new QStandardItem(tr("X dimension")));
  this->tableModel->setItem(3, 0, new QStandardItem(tr("Y dimension")));
  this->tableModel->setItem(4, 0, new QStandardItem(tr("Z dimension")));
  this->tableModel->setItem(5, 0, new QStandardItem(tr("X spacing")));
  this->tableModel->setItem(6, 0, new QStandardItem(tr("Y spacing")));
  this->tableModel->setItem(7, 0, new QStandardItem(tr("Z spacing")));
  for (int i = 0; i < 8; i++) {
    this->tableModel->setItem(i, 1, new QStandardItem(tr("")));
  }

  this->imageDataView->setModel(this->tableModel);
}


// Destructor
FibrinAppQt::~FibrinAppQt() {
  delete this->dataModel;
  delete this->visualization;
  delete this->tableModel;
}


// Action to be taken upon file open 
void FibrinAppQt::fileOpenImage() {

  // Locate file.
  QString fileName = QFileDialog::getOpenFileName(this, "Open Image Data", "", "VTK Images (*.vtk);;LSM Images (*.lsm);;");

  // Now read the file
  if (fileName == "") {
    return;

  }
  std::cout << "Loading file '" << fileName.toStdString() << "'" << std::endl;
  this->dataModel->LoadImageFile(fileName.toStdString());
  
  // Set status bar with info about the file.
  QString imageInfo("Loaded image '"); imageInfo.append(fileName); imageInfo.append("'.");
  this->statusbar->showMessage(imageInfo);

  // Set isovalue to midpoint between data min and max
  double min = this->dataModel->GetFilteredDataMinimum();
  double max = this->dataModel->GetFilteredDataMaximum();
  double isoValue = 0.5*(min + max);
  QString isoValueString = QString().sprintf(".4f", isoValue);
  this->isoValueEdit->setText(isoValueString);
  this->isoValueSlider->setValue(static_cast<int>(isoValue));

  // Set up visualization pipeline.
  this->visualization->SetInputConnection(this->dataModel->GetOutputPort());
  this->visualization->SetIsoValue(isoValue);

  // Refresh the UI
  this->refreshUI();

  // Reset camera
  this->ren->ResetCamera();
}


void FibrinAppQt::fileSaveFilteredImage() {
  QString fileName = QFileDialog::getSaveFileName(this, "Save Filtered Image", "", "VTK (*.vtk);;");
  if (fileName == "")
    return;

  this->dataModel->SaveImageFile(fileName.toStdString());

}


void FibrinAppQt::fileSavePicture() {
  QString fileName = QFileDialog::getSaveFileName(this, "Save Picture", "", "PNG (*.png);;");
  if (fileName == "")
    return;

  vtkWindowToImageFilter* capturer = vtkWindowToImageFilter::New();
  capturer->SetInput(this->qvtkWidget->GetRenderWindow());

  vtkPNGWriter* writer = vtkPNGWriter::New();
  writer->SetInputConnection(capturer->GetOutputPort());
  writer->SetFileName(fileName.toStdString().c_str());
  writer->Write();

  capturer->Delete();
  writer->Delete();
}


void FibrinAppQt::fileSaveRotationAnimation() {
  QString fileName = QFileDialog::getSaveFileName(this, "Save Rotation Animation", "", "PNG (*.png);;");
  if (fileName == "")
    return;

  // Chop off extension and save it for later.
  QString fileRoot = fileName.left(fileName.size()-4);
  QString extension = fileName.right(4);
  std::cout << extension.toStdString() << std::endl;
  
  vtkWindowToImageFilter* capturer = vtkWindowToImageFilter::New();
  capturer->SetInput(this->qvtkWidget->GetRenderWindow());

  vtkPNGWriter* writer = vtkPNGWriter::New();
  writer->SetInputConnection(capturer->GetOutputPort());

  vtkCamera* camera = this->ren->GetActiveCamera();

  int frames = 120;
  double angleIncrement = 360 / frames;
  for (int i = 0; i < frames; i++) {
    capturer->Modified();

    QString thisFileName = QString().sprintf("%s%04d%s",
      fileRoot.toStdString().c_str(), i, extension.toStdString().c_str());
    writer->SetFileName(thisFileName.toStdString().c_str());
    writer->Write();

    camera->Azimuth(angleIncrement);
    ren->ResetCameraClippingRange();
    qvtkWidget->GetRenderWindow()->Render();

    float progress = static_cast<float>(i) / static_cast<float>(frames);
    this->UpdateProgress(progress);
  }
  this->UpdateProgress(1.0);

  capturer->Delete();
  writer->Delete();
}


void FibrinAppQt::fileSaveGeometry() {
  QString fileName = QFileDialog::getSaveFileName(this, "Save Geometry", "", "VTK (*.vtk);;");
  if (fileName == "")
    return;

  vtkPolyDataWriter* writer = vtkPolyDataWriter::New();
  writer->SetInputConnection(this->visualization->GetIsosurfaceOutputPort());
  writer->SetFileName(fileName.toStdString().c_str());
  writer->Write();

  writer->Delete();
}


void FibrinAppQt::fileExit() {
  qApp->exit();
}


void FibrinAppQt::viewResetView() {
  vtkCamera* camera = this->ren->GetActiveCamera();
  camera->SetFocalPoint(0, 0, 0);
  camera->SetPosition(0, 0, 1);
  camera->SetViewUp(0, 1, 0);
  this->ren->ResetCamera();
  this->qvtkWidget->GetRenderWindow()->Render();
}


void FibrinAppQt::viewOpenView() {
  this->qvtkWidget->GetRenderWindow()->Render();

}


void FibrinAppQt::viewSaveView() {

}


void FibrinAppQt::isoValueEditHandler(QString text) {
  int value = static_cast<int>(text.toDouble());
  this->isoValueSlider->setValue(value);
}


void FibrinAppQt::isoValueSliderHandler(int value) {
  QString text = QString().sprintf("%d", value);
  this->isoValueEdit->setText(text);
}


void FibrinAppQt::showDataOutlineHandler(bool show) {
  this->visualization->SetShowOutline(show);
  this->qvtkWidget->GetRenderWindow()->Render();
}


void FibrinAppQt::applyButtonHandler() {
  // Read fiber diameter.
  double fiberDiameter = fiberDiameterEdit->text().toDouble();
  this->dataModel->SetFiberDiameter(fiberDiameter);

  // Read isovalue.
  double isoValue = isoValueEdit->text().toDouble();
  this->visualization->SetIsoValue(isoValue);
  refreshUI();
}


void FibrinAppQt::refreshUI() {
  ///////////////// Update image filters ////////////////
  QString filterText = this->imageFilterComboBox->currentText();
  if (filterText.toStdString() == NO_FILTER_STRING) {
    this->dataModel->SetFilterToNone();
  } else if (filterText.toStdString() == VESSELNESS_FILTER_STRING) {
    this->dataModel->SetFilterToVesselness();
  }

  ///////////////// Update GUI /////////////////
  const char *decimalFormat = "%.3f";

  QString fiberDiameter = QString().sprintf(decimalFormat, this->dataModel->GetFiberDiameter());
  this->fiberDiameterEdit->setText(fiberDiameter);

  QString dataMin = QString().sprintf(decimalFormat, this->dataModel->GetFilteredDataMinimum());
  this->tableModel->item(0, 1)->setText(dataMin);
  QString dataMax = QString().sprintf(decimalFormat, this->dataModel->GetFilteredDataMaximum());
  this->tableModel->item(1, 1)->setText(dataMax);

  int dims[3];
  this->dataModel->GetDimensions(dims);
  QString xDim = QString().sprintf("%d", dims[0]);
  this->tableModel->item(2, 1)->setText(xDim);
  QString yDim = QString().sprintf("%d", dims[1]);
  this->tableModel->item(3, 1)->setText(yDim);
  QString zDim = QString().sprintf("%d", dims[2]);
  this->tableModel->item(4, 1)->setText(zDim);

  double spacing[3];
  this->dataModel->GetSpacing(spacing);
  QString xSpacing = QString().sprintf(decimalFormat, spacing[0]);
  this->tableModel->item(5, 1)->setText(xSpacing);

  QString ySpacing = QString().sprintf(decimalFormat, spacing[1]);
  this->tableModel->item(6, 1)->setText(ySpacing);

  QString zSpacing = QString().sprintf(decimalFormat, spacing[2]);
  this->tableModel->item(7, 1)->setText(zSpacing);

  double isoValue = this->visualization->GetIsoValue();
  QString isoValueString = QString().sprintf(decimalFormat, isoValue);
  this->isoValueEdit->setText(isoValueString);
  this->isoValueSlider->setValue(static_cast<int>(isoValue));
  this->isoValueSlider->setMinValue(static_cast<int>(
    this->dataModel->GetFilteredDataMinimum()));
  this->isoValueSlider->setMaxValue(static_cast<int>(
    this->dataModel->GetFilteredDataMaximum()));

  ///////////////// Update visualization stuff /////////////////
  this->ren->RemoveAllViewProps();

  if (this->dataModel->GetImageData()) {
    this->visualization->SetInputConnection(this->dataModel->GetOutputPort());
    this->visualization->AddToRenderer(this->ren);
    this->visualization->SetFastIsosurfaceRendering(this->fastRenderingCheckBox->isChecked());
  }

  this->qvtkWidget->GetRenderWindow()->Render();
}


void FibrinAppQt::UpdateProgress(float progress) const {
  int progressValue = static_cast<int>(progress*100.0);
  this->progressBar->setValue(progressValue);
}

