#include "FibrinAppQt.h"

#include <qapplication.h>
#include <qfiledialog.h>
#include <qvariant.h>

#include <vtkActor.h>
#include <vtkContourFilter.h>
#include <vtkImageImport.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

#include <itkSize.h>

#include "IntensityThresholdThinningFilter.h"


// Constructor
FibrinAppQt::FibrinAppQt(QWidget* p) 
 : QMainWindow(p) {
  setupUi(this);

  // QT/VTK interact
  this->ren = vtkRenderer::New();
  qvtkWidget->GetRenderWindow()->AddRenderer(ren);

  // Hook up menus signals to slots
  connect(actionOpenImage, SIGNAL(triggered()), this, SLOT(fileOpenImage()));
  connect(actionOpenView, SIGNAL(triggered()), this, SLOT(fileOpenView()));
  connect(actionSaveView, SIGNAL(triggered()), this, SLOT(fileSaveView()));
  connect(actionExit, SIGNAL(triggered()), this, SLOT(fileExit()));

  connect(applyButton, SIGNAL(clicked()), this, SLOT(applyButtonHandler()));

  connect(isoValueEdit, SIGNAL(textEdited(QString)), this, SLOT(isoValueEditHandler(QString)));
  connect(isoValueSlider, SIGNAL(sliderMoved(int)), this, SLOT(isoValueSliderHandler(int)));
  
  connect(showDataOutline, SIGNAL(toggled(bool)), this, SLOT(showDataOutlineHandler(bool)));

  // Instantiate data model.
  this->dataModel = new DataModel<UShort3DImageType>();

  // Instantiate visualization pipelines.
  this->visualization = new Visualization();

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
  QString fileName = QFileDialog::getOpenFileName(this, "Open Image Data", "", "VTK Images (*.vtk);;");

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
  double min = this->dataModel->GetMinimumImageIntensity();
  double max = this->dataModel->GetMaximumImageIntensity();
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


void FibrinAppQt::fileOpenView() {

}


void FibrinAppQt::fileSaveView() {

}


void FibrinAppQt::fileExit() {
  qApp->exit();
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
  // Read isovalue.
  double isoValue = isoValueEdit->text().toDouble();
  this->visualization->SetIsoValue(isoValue);
  refreshUI();
}


void FibrinAppQt::refreshUI() {
  ///////////////// Update GUI /////////////////
  const char *decimalFormat = "%.3f";

  QString dataMin = QString().sprintf(decimalFormat, this->dataModel->GetMinimumImageIntensity());
  this->tableModel->item(0, 1)->setText(dataMin);
  QString dataMax = QString().sprintf(decimalFormat, this->dataModel->GetMaximumImageIntensity());
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
    this->dataModel->GetMinimumImageIntensity()));
  this->isoValueSlider->setMaxValue(static_cast<int>(
    this->dataModel->GetMaximumImageIntensity()));


  ///////////////// Update visualization stuff /////////////////
  this->ren->RemoveAllViewProps();

  if (this->dataModel->GetImageData()) {
    this->visualization->AddToRenderer(this->ren);
  }

  this->qvtkWidget->GetRenderWindow()->Render();
}
