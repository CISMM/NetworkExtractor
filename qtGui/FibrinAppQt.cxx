#include "FibrinAppQt.h"
#include "ConfigurationFileParser.h"

#if defined(_WIN32) // Turn off deprecation warnings in Visual Studio
#pragma warning( disable : 4996 )
#endif

#include <qapplication.h>
#include <qerrormessage.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qsettings.h>
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


void ProgressCallback(float progress, const char* processName) {
  QWidgetList list = QApplication::allWidgets();
  
  // I'm sure there is an easier way to do this, but this works for now.
  for (int i = 0; i < list.size(); i++) {
    QWidget* widget = list[i];
    if (widget->objectName().toStdString() == "progressBar") {
      QProgressBar* progressBar = dynamic_cast<QProgressBar*>(widget);
      progressBar->setValue(static_cast<int>(100 * progress));
    } else if (widget->objectName().toStdString() == "statusbar") {
      QStatusBar* statusbar = dynamic_cast<QStatusBar*>(widget);
      statusbar->showMessage(QString(processName));
    }
  }
}


// Constructor
FibrinAppQt::FibrinAppQt(QWidget* p) 
 : QMainWindow(p) {
  setupUi(this);

  this->filterType = DataModel::NO_FILTER_STRING;

  // QT/VTK interact
  this->ren = vtkRenderer::New();
  this->qvtkWidget->GetRenderWindow()->AddRenderer(ren);

  // Instantiate data model.
  this->dataModel = new DataModel();
  this->dataModel->SetProgressCallback(ProgressCallback);

  // Instantiate visualization pipelines.
  this->visualization = new Visualization();

  this->imageFilterComboBox->addItem(QString(DataModel::NO_FILTER_STRING.c_str()));
  //this->imageFilterComboBox->addItem(QString(DataModel::FRANGI_FIBERNESS_FILTER_STRING.c_str()));
  this->imageFilterComboBox->addItem(QString(DataModel::MULTISCALE_FIBERNESS_FILTER_STRING.c_str()));
  this->imageFilterComboBox->addItem(QString(DataModel::MULTISCALE_FIBERNESS_THRESHOLD_FILTER_STRING.c_str()));
  this->imageFilterComboBox->addItem(QString(DataModel::MULTISCALE_SKELETONIZATION_FILTER_STRING.c_str()));
  //this->imageFilterComboBox->addItem(QString(DataModel::JUNCTIONNESS_FILTER_STRING.c_str()));
  //this->imageFilterComboBox->addItem(QString(DataModel::JUNCTIONNESS_LOCAL_MAX_FILTER_STRING.c_str()));

  // Create and populate table model.
  int LEFT_COLUMN = 0;
  int RIGHT_COLUMN = 1;
  this->tableModel = new QStandardItemModel(8, 2, this);
  this->tableModel->setHeaderData(LEFT_COLUMN,  Qt::Horizontal, tr("Property"));
  this->tableModel->setHeaderData(RIGHT_COLUMN, Qt::Horizontal, tr("Value"));

  QStandardItem* labelItems[9];
  labelItems[0] = new QStandardItem(tr("Intensity minimum"));
  labelItems[1] = new QStandardItem(tr("Intensity maximum"));
  labelItems[2] = new QStandardItem(tr("X dimension (pixels)"));
  labelItems[3] = new QStandardItem(tr("Y dimension (pixels)"));
  labelItems[4] = new QStandardItem(tr("Z dimension (slices)"));
  labelItems[5] = new QStandardItem(tr("X pixel size (µm)"));
  labelItems[6] = new QStandardItem(tr("Y pixel size (µm)"));
  labelItems[7] = new QStandardItem(tr("Z slice spacing (µm)"));
  labelItems[8] = new QStandardItem(tr("File name"));

  for (int i = 0; i < 9; i++) {
    labelItems[i]->setEditable(false);
    this->tableModel->setItem(i, LEFT_COLUMN, labelItems[i]);

    QStandardItem* item = new QStandardItem(tr(""));

    // Allow editing of voxel spacing.
    if (i < 5)
      item->setEditable(false);
    this->tableModel->setItem(i, RIGHT_COLUMN, item);
  }
  this->imageDataView->setModel(this->tableModel);

  connect(tableModel, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), 
    this, SLOT(handle_tableModel_dataChanged(const QModelIndex&, const QModelIndex&)));

  QCoreApplication::setOrganizationName("CISMM");
  QCoreApplication::setOrganizationDomain("cismm.org");
  QCoreApplication::setApplicationName("Fibrin Analysis");

  // Restore GUI settings.
  this->readProgramSettings();

  // Set up error dialog box.
  this->errorDialog.setModal(true);
}


// Destructor
FibrinAppQt::~FibrinAppQt() {
  delete this->dataModel;
  delete this->visualization;
  delete this->tableModel;
}


void FibrinAppQt::on_actionOpenSession_triggered() {
  QString fileName =
    QFileDialog::getOpenFileName(this, tr("Open Session File"), tr(""),
				 tr("Session File (*.fas);;"));
  if (fileName == "") {
    return;
  }

  ConfigurationFileParser parser;
  parser.Parse(fileName.toStdString());

  std::string stringValue;
  bool boolValue = false;
  int intValue = 0;
  double doubleValue = 0.0;

  // Load image
  stringValue = parser.GetValue("ImageFile", "fileName");
  printf("Opening %s\n", stringValue.c_str());
  this->OpenFile(stringValue);

  // Set up visualization pipeline.
  this->visualization->SetImageInputConnection(this->dataModel->GetImageOutputPort());
  this->visualization->SetFilteredImageInputConnection(this->dataModel->GetFilteredImageOutputPort());

  // Filter settings
  doubleValue = parser.GetValueAsDouble("MultiscaleFibernessFilter", "alpha");
  this->dataModel->SetMultiscaleFibernessAlphaCoefficient(doubleValue);
  doubleValue = parser.GetValueAsDouble("MultiscaleFibernessFilter", "beta");
  this->dataModel->SetMultiscaleFibernessBetaCoefficient(doubleValue);
  doubleValue = parser.GetValueAsDouble("MultiscaleFibernessFilter", "gamma");
  this->dataModel->SetMultiscaleFibernessGammaCoefficient(doubleValue);
  doubleValue = parser.GetValueAsDouble("MultiscaleFibernessFilter", "minimumScale");
  this->dataModel->SetMultiscaleFibernessMinimumScale(doubleValue);
  doubleValue = parser.GetValueAsDouble("MultiscaleFibernessFilter", "maximumScale");
  this->dataModel->SetMultiscaleFibernessMaximumScale(doubleValue);
  intValue = parser.GetValueAsInt("MultiscaleFibernessFilter", "numberOfScales");
  this->dataModel->SetMultiscaleFibernessNumberOfScales(intValue);
  doubleValue = parser.GetValueAsDouble("MultiscaleFibernessFilterThreshold", "threshold");
  this->dataModel->SetMultiscaleFibernessThreshold(doubleValue);
  doubleValue = parser.GetValueAsDouble("FilteredImageOutput", "intensityScaleFactor");
  this->dataModel->SetFilteredImageScaleFactor(doubleValue);

  // Display settings
  boolValue = parser.GetValueAsBool("Display", "isosurfaceVisible");
  this->visualization->SetIsosurfaceVisible(boolValue);
  doubleValue = parser.GetValueAsDouble("Display", "isovalue");
  this->visualization->SetIsoValue(doubleValue);
  boolValue = parser.GetValueAsBool("Display", "imagePlaneVisible");
  this->visualization->SetImagePlaneVisible(boolValue);
  intValue = parser.GetValueAsInt("Display", "zPlane");
  this->visualization->SetZSlice(intValue);
  boolValue = parser.GetValueAsBool("Display", "cropIsosurface");
  this->visualization->SetCropIsosurface(boolValue);
  intValue = parser.GetValueAsInt("Display", "keepPlanesAboveBelow");
  this->visualization->SetKeepPlanesAboveBelowImagePlane(intValue);
  boolValue = parser.GetValueAsBool("Display", "fastIsosurfaceRendering");
  this->visualization->SetFastIsosurfaceRendering(boolValue);
  boolValue = parser.GetValueAsBool("Display", "showDataOutline");
  this->visualization->SetShowOutline(boolValue);

  this->RefreshUI();

  // Reset camera
  this->ren->ResetCamera();
  
  // Render
  this->qvtkWidget->GetRenderWindow()->Render();
}


void FibrinAppQt::on_actionSaveSession_triggered() {
  QString fileName = 
    QFileDialog::getSaveFileName(this, tr("Save Session File"), tr(""), 
				 tr("Session File (*.fas);;"));

  if (fileName == "") {
    return;
  }

  FILE* fp = fopen(fileName.toStdString().c_str(), "w");
  if (fp == NULL) {
    return;
  }

  fprintf(fp, "; Written by Fibrin Analysis\n\n");

  fprintf(fp, "[ImageFile]\n");
  fprintf(fp, "fileName=%s\n\n", this->dataModel->GetImageFileName().c_str());
  
  fprintf(fp, "[MultiscaleFibernessFilter]\n");
  fprintf(fp, "alpha=%.3f\n", this->dataModel->GetMultiscaleFibernessAlphaCoefficient());
  fprintf(fp, "beta=%.3f\n",  this->dataModel->GetMultiscaleFibernessBetaCoefficient());
  fprintf(fp, "gamma=%.3f\n", this->dataModel->GetMultiscaleFibernessGammaCoefficient());
  fprintf(fp, "minimumScale=%.3f\n", this->dataModel->GetMultiscaleFibernessMinimumScale());
  fprintf(fp, "maximumScale=%.3f\n", this->dataModel->GetMultiscaleFibernessMaximumScale());
  fprintf(fp, "numberOfScales=%d\n\n", this->dataModel->GetMultiscaleFibernessNumberOfScales());
  fprintf(fp, "[MultiscaleFibernessFilterThreshold]\n");
  fprintf(fp, "threshold=%.3f\n\n", this->dataModel->GetMultiscaleFibernessThreshold());

  fprintf(fp, "[FilteredImageOutput]\n");
  fprintf(fp, "intensityScaleFactor=%.3f\n\n", this->dataModel->GetFilteredImageScaleFactor());

  fprintf(fp, "[Display]\n");
  fprintf(fp, "isosurfaceVisible=%s\n", this->visualization->GetIsosurfaceVisible() ? "true" : "false");
  fprintf(fp, "isovalue=%.3f\n", this->visualization->GetIsoValue());
  fprintf(fp, "imagePlaneVisible=%s\n", this->visualization->GetImagePlaneVisible() ? "true" : "false");
  fprintf(fp, "zPlane=%d\n", this->visualization->GetZSlice());
  fprintf(fp, "cropIsosurface=%s\n", this->visualization->GetCropIsosurface() ? "true" : "false");
  fprintf(fp, "keepPlanesAboveBelow=%d\n", this->visualization->GetKeepPlanesAboveBelowImagePlane());
  fprintf(fp, "fastIsosurfaceRendering=%s\n", this->visualization->GetFastIsosurfaceRendering() ? "true" : "false");
  fprintf(fp, "showDataOutline=%s\n", this->visualization->GetShowOutline() ? "true" : "false");

  fclose(fp);

}


// Action to be taken upon file open 
void FibrinAppQt::on_actionOpenImage_triggered() {

  // Locate file.
  QString fileName = QFileDialog::getOpenFileName(this, "Open Image Data", "", "TIF Images (*.tif);;VTK Images (*.vtk);;LSM Images (*.lsm);;");

  // Now read the file
  if (fileName == "") {
    return;
  }

  this->OpenFile(fileName.toStdString());

  // Set isovalue to midpoint between data min and max
  double min = this->dataModel->GetFilteredDataMinimum();
  double max = this->dataModel->GetFilteredDataMaximum();
  double isoValue = 0.5*(min + max);
  QString isoValueString = QString().sprintf("%.4f", isoValue);
  this->isoValueEdit->setText(isoValueString);
  this->isoValueSlider->setValue(this->isoValueSliderPosition(isoValue));

  // Set up visualization pipeline.
  this->visualization->SetImageInputConnection(this->dataModel->GetImageOutputPort());
  this->visualization->SetFilteredImageInputConnection(this->dataModel->GetFilteredImageOutputPort());
  this->visualization->SetIsoValue(isoValue);

  // Refresh the UI
  this->RefreshUI();

  // Reset camera
  this->ren->ResetCamera();
  
  // Render
  this->qvtkWidget->GetRenderWindow()->Render();
}


void FibrinAppQt::OpenFile(std::string fileName) {
  std::cout << "Loading file '" << fileName << "'" << std::endl;
  this->dataModel->LoadImageFile(fileName);
  
  // Set status bar with info about the file.
  QString imageInfo("Loaded image '"); imageInfo.append(fileName.c_str()); imageInfo.append("'.");
  this->statusbar->showMessage(imageInfo);
}


void FibrinAppQt::on_actionSaveFilteredImage_triggered() {
  QString fileName = QFileDialog::getSaveFileName(this, "Save Filtered Image", "", "TIF Images (*.tif);;VTK (*.vtk);;");
  if (fileName == "")
    return;

  float scaleFactor = this->filteredImageScaleEdit->text().toDouble();
  this->dataModel->SaveFilteredImageFile(fileName.toStdString(), this->filterType, scaleFactor);

}


void FibrinAppQt::on_actionSaveFiberOrientationImage_triggered() {
  QString fileName = QFileDialog::getSaveFileName(this, "Save Fiber Orientation Image", "", "VTK (*.vtk);;");
  if (fileName == "")
    return;

  this->dataModel->SaveFiberOrientationImageFile(fileName.toStdString());
}


void FibrinAppQt::on_actionSaveFiberOrientationData_triggered() {

  // Allow saving only when skeletonization filter is selected.
  if (this->filterType == DataModel::MULTISCALE_SKELETONIZATION_FILTER_STRING) {
    QString fileName = QFileDialog::getSaveFileName(this, "Save Fiber Orientation Data", "", "CSV (*.csv);;");
    if (fileName == "")
      return;
    this->dataModel->SaveFiberOrientationDataFile(fileName.toStdString());

  } else {

    // Notify the user that skeletonization filter must be selected.
    QMessageBox messageBox;
    messageBox.setText("Error");
    messageBox.setInformativeText("You must have the Skeletonization filter selected in the Image Analysis dock to save the fiber orientation data.");
    messageBox.setStandardButtons(QMessageBox::Ok);
    messageBox.exec();
  }
}


void FibrinAppQt::on_actionSavePicture_triggered() {
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


void FibrinAppQt::on_actionSaveRotationAnimation_triggered() {
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


void FibrinAppQt::on_actionSaveGeometry_triggered() {
  QString fileName = QFileDialog::getSaveFileName(this, "Save Geometry", "", "VTK (*.vtk);;");
  if (fileName == "")
    return;

  vtkPolyDataWriter* writer = vtkPolyDataWriter::New();
  writer->SetInputConnection(this->visualization->GetIsosurfaceOutputPort());
  writer->SetFileName(fileName.toStdString().c_str());
  writer->Write();

  writer->Delete();
}


void FibrinAppQt::on_actionExit_triggered() {
  // Ask if user really wants to quit.
  QMessageBox messageBox;
  messageBox.setText("Do you really want to exit?");
  messageBox.setInformativeText("If you exit now, all settings will be lost.");
  messageBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
  messageBox.setDefaultButton(QMessageBox::Cancel);
  int selected = messageBox.exec();

  if (selected == QMessageBox::Ok) {
    this->writeProgramSettings();
    qApp->exit();
  }

}


void FibrinAppQt::on_actionResetView_triggered() {
  vtkCamera* camera = this->ren->GetActiveCamera();
  camera->SetFocalPoint(0, 0, 0);
  camera->SetPosition(0, 0, 1);
  camera->SetViewUp(0, 1, 0);
  this->ren->ResetCamera();
  this->qvtkWidget->GetRenderWindow()->Render();
}


void FibrinAppQt::on_actionOpenView_triggered() {
  QString fileName = 
    QFileDialog::getOpenFileName(this, tr("Open Camera View File"), tr(""), 
				 tr("View Files (*.view);;"));
  if (fileName == "")
    return;

  vtkCamera* camera = this->ren->GetActiveCamera();

  ConfigurationFileParser parser;
  parser.Parse(fileName.toStdString());

  double values[3];
  parser.GetValueAsDouble3("Camera", "Position", values);
  camera->SetPosition(values);

  parser.GetValueAsDouble3("Camera", "FocalPoint", values);
  camera->SetFocalPoint(values);

  parser.GetValueAsDouble3("Camera", "UpVector", values);
  camera->SetViewUp(values);

  this->ren->ResetCameraClippingRange();
  this->qvtkWidget->GetRenderWindow()->Render();
}


void FibrinAppQt::on_actionSaveView_triggered() {

  QString fileName = QFileDialog::getSaveFileName(this, 
    "Save Camera View File", "", "View files (*.view);;");
  if (fileName == "")
    return;

  FILE* fp = fopen(fileName.toStdString().c_str(), "w");
  if (fp == NULL) {
    errorDialog.showMessage(tr("Could not write view file ").append(fileName));
  }

  fprintf(fp, "; Written by Fibrin Analysis\n\n");
  fprintf(fp, "[Camera]\n");

  double* position = this->ren->GetActiveCamera()->GetPosition();
  fprintf(fp, "Position=%.4f %.4f %.4f\n", position[0], position[1], position[2]);

  double* focalPoint = this->ren->GetActiveCamera()->GetFocalPoint();
  fprintf(fp, "FocalPoint=%.4f %.4f %.4f\n", focalPoint[0], focalPoint[1], focalPoint[2]);

  double* upVector = this->ren->GetActiveCamera()->GetViewUp();
  fprintf(fp, "UpVector=%.4f %.4f %.4f\n", upVector[0], upVector[1], upVector[2]);

  fclose(fp);
}


void FibrinAppQt::on_imageFilterComboBox_currentIndexChanged(QString filterText) {
  this->fibernessSettingsWidget->setVisible(false);
  this->multiscaleFibernessSettingsWidget->setVisible(false);
  this->multiscaleFibernessThresholdSettingsWidget->setVisible(false);
  this->junctionnessSettingsWidget->setVisible(false);
  this->junctionnessLocalMaxSettingsWidget->setVisible(false);

  if (filterText.toStdString() == DataModel::NO_FILTER_STRING) {

  } else if (filterText.toStdString() == DataModel::FRANGI_FIBERNESS_FILTER_STRING) {
    this->fibernessSettingsWidget->setVisible(true);
  } else if (filterText.toStdString() == DataModel::MULTISCALE_FIBERNESS_FILTER_STRING) {
    this->multiscaleFibernessSettingsWidget->setVisible(true);
  } else if (filterText.toStdString() == DataModel::MULTISCALE_FIBERNESS_THRESHOLD_FILTER_STRING) {
    this->multiscaleFibernessSettingsWidget->setVisible(true);
    this->multiscaleFibernessThresholdSettingsWidget->setVisible(true);
  } else if (filterText.toStdString() == DataModel::MULTISCALE_SKELETONIZATION_FILTER_STRING) {
    this->multiscaleFibernessSettingsWidget->setVisible(true);
    this->multiscaleFibernessThresholdSettingsWidget->setVisible(true);
  } else if (filterText.toStdString() == DataModel::JUNCTIONNESS_FILTER_STRING) {
    this->fibernessSettingsWidget->setVisible(true);
    this->junctionnessSettingsWidget->setVisible(true);
  } else if (filterText.toStdString() == DataModel::JUNCTIONNESS_LOCAL_MAX_FILTER_STRING) {
    this->fibernessSettingsWidget->setVisible(true);
    this->junctionnessSettingsWidget->setVisible(true);
    this->junctionnessLocalMaxSettingsWidget->setVisible(true);
  }
}


void FibrinAppQt::on_showIsosurfaceCheckBox_toggled(bool show) {
  this->visualization->SetIsosurfaceVisible(show);
  this->qvtkWidget->GetRenderWindow()->Render();
}


void FibrinAppQt::on_isoValueEdit_textEdited(QString text) {
  int value = static_cast<int>(text.toDouble());
  this->isoValueSlider->setValue(this->isoValueSliderPosition(value));
}


void FibrinAppQt::on_isoValueSlider_sliderMoved(int value) {
  QString text = QString().sprintf("%.3f", this->isoValueSliderValue(value));
  this->isoValueEdit->setText(text);
}


void FibrinAppQt::on_showZPlaneCheckbox_toggled(bool show) {
  this->visualization->SetImagePlaneVisible(show);
  this->qvtkWidget->GetRenderWindow()->Render();
}


void FibrinAppQt::on_zPlaneEdit_textEdited(QString text) {
  int value = static_cast<int>(text.toDouble()) - 1;
  this->zPlaneSlider->setValue(value);

  // Read z-slice.
  int dims[3];
  this->dataModel->GetDimensions(dims);
  int slice = zPlaneEdit->text().toInt()-1;
  if (slice >= 0 && slice < dims[2]) {
    this->visualization->SetZSlice(slice);
    this->qvtkWidget->GetRenderWindow()->Render();
  }
}


void FibrinAppQt::on_zPlaneSlider_sliderMoved(int value) {
  QString text = QString().sprintf("%d", value);
  this->zPlaneEdit->setText(text);

  // Read z-slice.
  int dims[3];
  this->dataModel->GetDimensions(dims);
  int slice = zPlaneEdit->text().toInt()-1;
  if (slice >= 0 && slice < dims[2]) {
    this->visualization->SetZSlice(slice);
    this->qvtkWidget->GetRenderWindow()->Render();
  }
}


void FibrinAppQt::on_saveConnectedComponentsData_clicked() {
  QString fileName = QFileDialog::getSaveFileName(this, 
    "Save Connected Component vs. Threshold Data", "", "TXT (*.txt);;");
  if (fileName == "")
    return;
  
  double minThreshold = minThresholdEdit->text().toDouble();
  double maxThreshold = maxThresholdEdit->text().toDouble();
  double thresholdIncrement = 0.5;
  this->dataModel->ComputeConnectedComponentsVsThresholdData(minThreshold,
    maxThreshold, thresholdIncrement, fileName.toStdString());
}


void FibrinAppQt::on_saveVolumeFractionEstimateData_clicked() {
  QString fileName = QFileDialog::getSaveFileName(this, 
    "Save Volume Fraction Estimate vs. Z Data", "", "TXT (*.txt);;");
  if (fileName == "")
    return;

  double threshold = fibernessThresholdEdit->text().toDouble();
  this->dataModel->ComputeVolumeFractionEstimateVsZData(threshold, fileName.toStdString());
}


void FibrinAppQt::on_showDataOutlineCheckBox_toggled(bool show) {
  this->visualization->SetShowOutline(show);
  this->qvtkWidget->GetRenderWindow()->Render();
}


void FibrinAppQt::on_cropIsosurfaceCheckBox_toggled(bool crop) {
  this->visualization->SetCropIsosurface(crop);
  
  // Read z-slice.
  int dims[3];
  this->dataModel->GetDimensions(dims);
  int slice = zPlaneEdit->text().toInt()-1;
  if (slice >= 0 && slice < dims[2]) {
    this->visualization->SetZSlice(slice);
    this->qvtkWidget->GetRenderWindow()->Render();
  }
}


void FibrinAppQt::on_applyButton_clicked() {

  // Read fiber diameter.
  double fiberDiameter = this->fiberDiameterEdit->text().toDouble();
  this->dataModel->SetFiberDiameter(fiberDiameter);

  double fibernessAlphaCoef = this->fibernessAlphaCoefficientEdit->text().toDouble();
  this->dataModel->SetMultiscaleFibernessAlphaCoefficient(fibernessAlphaCoef);

  double fibernessBetaCoef = this->fibernessBetaCoefficientEdit->text().toDouble();
  this->dataModel->SetMultiscaleFibernessBetaCoefficient(fibernessBetaCoef);

  double fibernessGammaCoef = this->fibernessGammaCoefficientEdit->text().toDouble();
  this->dataModel->SetMultiscaleFibernessGammaCoefficient(fibernessGammaCoef);

  double fibernessScaleMinimum = this->fibernessScaleMinimumEdit->text().toDouble();
  this->dataModel->SetMultiscaleFibernessMinimumScale(fibernessScaleMinimum);

  double fibernessScaleMaximum = this->fibernessScaleMaximumEdit->text().toDouble();
  this->dataModel->SetMultiscaleFibernessMaximumScale(fibernessScaleMaximum);

  double fibernessNumberOfScales = this->fibernessNumberOfScalesEdit->text().toInt();
  this->dataModel->SetMultiscaleFibernessNumberOfScales(fibernessNumberOfScales);

  double fibernessThreshold = this->fibernessThresholdEdit->text().toDouble();
  this->dataModel->SetMultiscaleFibernessThreshold(fibernessThreshold);

  double junctionProbeDiameter = this->junctionProbeDiameterEdit->text().toDouble();
  this->dataModel->SetJunctionProbeDiameter(junctionProbeDiameter);

  double junctionFibernessThreshold = this->junctionFibernessThresholdEdit->text().toDouble();
  this->dataModel->SetJunctionFibernessThreshold(junctionFibernessThreshold);

  double junctionnessLocalMaxHeight = this->junctionnessLocalMaxHeightEdit->text().toDouble();
  this->dataModel->SetJunctionnessLocalMaxHeight(junctionnessLocalMaxHeight);

  double filteredImageScaleFactor = this->filteredImageScaleEdit->text().toDouble();
  this->dataModel->SetFilteredImageScaleFactor(filteredImageScaleFactor);

  ///////////////// Update image filters ////////////////
  QString filterText = this->imageFilterComboBox->currentText();
  if (filterText.toStdString() != this->filterType) {
    if (filterText.toStdString() == DataModel::NO_FILTER_STRING) {
      this->dataModel->SetFilterToNone();
    } else if (filterText.toStdString() == DataModel::FRANGI_FIBERNESS_FILTER_STRING) {
      this->dataModel->SetFilterToFrangiFiberness();
    } else if (filterText.toStdString() == DataModel::MULTISCALE_FIBERNESS_FILTER_STRING) {
      this->dataModel->SetFilterToMultiscaleFiberness();
    } else if (filterText.toStdString() == DataModel::MULTISCALE_FIBERNESS_THRESHOLD_FILTER_STRING) {
      this->dataModel->SetFilterToMultiscaleFibernessThreshold();
    } else if (filterText.toStdString() == DataModel::MULTISCALE_SKELETONIZATION_FILTER_STRING) {
      this->dataModel->SetFilterToMultiscaleSkeletonization();
    } else if (filterText.toStdString() == DataModel::JUNCTIONNESS_FILTER_STRING) {
      this->dataModel->SetFilterToJunctionness();
    } else if (filterText.toStdString() == DataModel::JUNCTIONNESS_LOCAL_MAX_FILTER_STRING) {
      this->dataModel->SetFilterToJunctionnessLocalMax();
    }
    this->filterType = filterText.toStdString();
  }

  // Read isovalue.
  double isoValue = this->isoValueEdit->text().toDouble();
  this->visualization->SetIsoValue(isoValue);

  // Read delta z.
  int keepPlanes = this->keepPlanesEdit->text().toInt();
  this->visualization->SetKeepPlanesAboveBelowImagePlane(keepPlanes);

  // Read z-slice.
  int slice = this->zPlaneEdit->text().toInt()-1;
  this->visualization->SetZSlice(slice);

  RefreshUI();
}


void FibrinAppQt::handle_tableModel_dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight) {
  if (topLeft != bottomRight) {
    return;
  }
  
  QStandardItem* item = this->tableModel->item(topLeft.row(), topLeft.column());
  double value = item->text().toDouble();

  int itemIndex = topLeft.row();
  if (itemIndex == 5) {
    this->dataModel->SetVoxelXSpacing(value);
  } else if (itemIndex == 6) {
    this->dataModel->SetVoxelYSpacing(value);
  } else if (itemIndex == 7) {
    this->dataModel->SetVoxelZSpacing(value);
  } else if (itemIndex == 8) {
    QFileInfo fileInfo(this->dataModel->GetImageFileName().c_str());
    this->tableModel->item(8, 1)->setText(fileInfo.fileName());
  }

  this->qvtkWidget->GetRenderWindow()->Render();
}


void FibrinAppQt::RefreshUI() {

  ///////////////// Update GUI /////////////////
  
  // Update window title
  QFileInfo fileInfo(this->dataModel->GetImageFileName().c_str());
  QString windowTitle("Fibrin Analysis");
  if (fileInfo.fileName() != "")
    windowTitle.append(tr(" - '").append(fileInfo.fileName()).append("'"));
  this->setWindowTitle(windowTitle);

  const char *decimalFormat = "%.3f";
  const char *intFormat = "%d";

  QString fiberDiameter = QString().sprintf(decimalFormat, this->dataModel->GetFiberDiameter());
  this->fiberDiameterEdit->setText(fiberDiameter);

  QString fibernessAlphaCoef = QString().sprintf(decimalFormat, this->dataModel->GetMultiscaleFibernessAlphaCoefficient());
  this->fibernessAlphaCoefficientEdit->setText(fibernessAlphaCoef);

  QString fibernessBetaCoef = QString().sprintf(decimalFormat, this->dataModel->GetMultiscaleFibernessBetaCoefficient());
  this->fibernessBetaCoefficientEdit->setText(fibernessBetaCoef);

  QString fibernessGammaCoef = QString().sprintf(decimalFormat, this->dataModel->GetMultiscaleFibernessGammaCoefficient());
  this->fibernessGammaCoefficientEdit->setText(fibernessGammaCoef);

  QString fibernessScaleMinimum = QString().sprintf(decimalFormat, this->dataModel->GetMultiscaleFibernessMinimumScale());
  this->fibernessScaleMinimumEdit->setText(fibernessScaleMinimum);

  QString fibernessScaleMaximum = QString().sprintf(decimalFormat, this->dataModel->GetMultiscaleFibernessMaximumScale());
  this->fibernessScaleMaximumEdit->setText(fibernessScaleMaximum);

  QString fibernessScales = QString().sprintf(intFormat, this->dataModel->GetMultiscaleFibernessNumberOfScales());
  this->fibernessNumberOfScalesEdit->setText(fibernessScales);

  QString fibernessThreshold = QString().sprintf(decimalFormat, this->dataModel->GetMultiscaleFibernessThreshold());
  this->fibernessThresholdEdit->setText(fibernessThreshold);
  
  QString junctionProbeFilterDiameter = QString().sprintf(decimalFormat, this->dataModel->GetJunctionProbeDiameter());
  this->junctionProbeDiameterEdit->setText(junctionProbeFilterDiameter);

  QString junctionFibernessThreshold = QString().sprintf(decimalFormat, this->dataModel->GetJunctionFibernessThreshold());
  this->junctionFibernessThresholdEdit->setText(junctionFibernessThreshold);

  QString junctionnessLocalMaxHeight = QString().sprintf(decimalFormat, this->dataModel->GetJunctionnessLocalMaxHeight());
  this->junctionnessLocalMaxHeightEdit->setText(junctionnessLocalMaxHeight);

  QString filteredImageScaleFactor = QString().sprintf(decimalFormat, this->dataModel->GetFilteredImageScaleFactor());
  this->filteredImageScaleEdit->setText(filteredImageScaleFactor);

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

  QString fileName = QString(this->dataModel->GetImageFileName().c_str());
  this->tableModel->item(8, 1)->setText(fileName);

  this->showIsosurfaceCheckBox->setChecked(this->visualization->GetIsosurfaceVisible());

  double isoValue = this->visualization->GetIsoValue();
  QString isoValueString = QString().sprintf(decimalFormat, isoValue);
  this->isoValueEdit->setText(isoValueString);
  this->isoValueSlider->setValue(this->isoValueSliderPosition(isoValue));

  this->showZPlaneCheckbox->setChecked(this->visualization->GetImagePlaneVisible());

  // Update slice slider
  this->zPlaneSlider->setMinValue(1);
  this->zPlaneSlider->setMaxValue(dims[2]);
  int zSlice = this->visualization->GetZSlice()+1;
  this->zPlaneSlider->setValue(zSlice);
  QString zPlaneString = QString().sprintf(intFormat, zSlice);
  this->zPlaneEdit->setText(zPlaneString);

  // Update cropping widgets
  this->cropIsosurfaceCheckBox->setChecked(this->visualization->GetCropIsosurface());
  QString keepPlanesString = QString().sprintf(intFormat, this->visualization->GetKeepPlanesAboveBelowImagePlane());
  this->keepPlanesEdit->setText(keepPlanesString);

  this->fastRenderingCheckBox->setChecked(this->visualization->GetFastIsosurfaceRendering());
  this->showDataOutlineCheckBox->setChecked(this->visualization->GetShowOutline());


  ///////////////// Update visualization stuff /////////////////
  this->ren->RemoveAllViewProps();

  if (this->dataModel->GetImageData()) {
    this->visualization->SetImageInputConnection(this->dataModel->GetImageOutputPort());
    this->visualization->SetFilteredImageInputConnection(this->dataModel->GetFilteredImageOutputPort());
    this->visualization->AddToRenderer(this->ren);
    this->visualization->SetFastIsosurfaceRendering(this->fastRenderingCheckBox->isChecked());
  }

  this->qvtkWidget->GetRenderWindow()->Render();
}


void FibrinAppQt::UpdateProgress(float progress) const {
  int progressValue = static_cast<int>(progress*100.0);
  this->progressBar->setValue(progressValue);
}


void FibrinAppQt::closeEvent(QCloseEvent* event) {
  this->writeProgramSettings();
}


void FibrinAppQt::writeProgramSettings() {
  QSettings settings;

  settings.beginGroup("MainWindow");
  settings.setValue("size", this->size());
  settings.setValue("pos", this->pos());
  settings.endGroup();

  QList<QDockWidget*> widgets = this->findChildren<QDockWidget*>();
  QListIterator<QDockWidget*> iterator(widgets);
  while (iterator.hasNext()) {
    QDockWidget* dockWidget = iterator.next();
    settings.beginGroup(dockWidget->objectName());
    settings.setValue("size", dockWidget->size());
    settings.setValue("pos", dockWidget->pos());
    settings.setValue("visible", dockWidget->isVisible());
    settings.setValue("floating", dockWidget->isFloating());
    settings.setValue("dockArea", this->dockWidgetArea(dockWidget));
    settings.endGroup();
  }

}


void FibrinAppQt::readProgramSettings() {
  QSettings settings;

  settings.beginGroup("MainWindow");
  this->resize(settings.value("size", QSize(1000, 743)).toSize());
  this->move(settings.value("pos", QPoint(0, 0)).toPoint());
  settings.endGroup();

  QList<QDockWidget*> widgets = this->findChildren<QDockWidget*>();
  QListIterator<QDockWidget*> iterator(widgets);
  while (iterator.hasNext()) {
    QDockWidget* dockWidget = iterator.next();
    settings.beginGroup(dockWidget->objectName());
    dockWidget->resize(settings.value("size", QSize(340, 200)).toSize());
    dockWidget->move(settings.value("pos", QPoint(0, 0)).toPoint());
    dockWidget->setVisible(settings.value("visible", true).toBool());
    dockWidget->setFloating(settings.value("floating", false).toBool());
    this->addDockWidget(static_cast<Qt::DockWidgetArea>(settings.value("dockArea", Qt::LeftDockWidgetArea).toUInt()), dockWidget);
    settings.endGroup();
  }

}


int FibrinAppQt::isoValueSliderPosition(double value) {
  double dataMin = this->dataModel->GetFilteredDataMinimum();
  double dataMax = this->dataModel->GetFilteredDataMaximum();
  double sliderMax = static_cast<double>(this->isoValueSlider->maximum());
  return static_cast<int>(sliderMax*((value-dataMin)/(dataMax-dataMin)));
}


double FibrinAppQt::isoValueSliderValue(int position) {
  double dataMin = this->dataModel->GetFilteredDataMinimum();
  double dataMax = this->dataModel->GetFilteredDataMaximum();
  double sliderMax = static_cast<double>(this->isoValueSlider->maximum());
  return (static_cast<double>(position) / sliderMax) 
    * (dataMax - dataMin) + dataMin;
}
