#include "FibrinAnalysis.h"
#include "ConfigurationFileParser.h"
#include "Version.h"

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
FibrinAnalysis::FibrinAnalysis(QWidget* p) 
 : QMainWindow(p) {
  setupUi(this);

  m_FilterType = DataModel::NO_FILTER_STRING;

  // QT/VTK interact
  m_Renderer = vtkRenderer::New();
  this->qvtkWidget->GetRenderWindow()->AddRenderer(m_Renderer);

  // Instantiate data model.
  m_DataModel = new DataModel();
  m_DataModel->SetProgressCallback(ProgressCallback);

  // Instantiate visualization pipelines.
  m_Visualization = new Visualization();
  m_Visualization->SetDirectionArrowVisible(false);

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
  m_TableModel = new QStandardItemModel(8, 2, this);
  m_TableModel->setHeaderData(LEFT_COLUMN,  Qt::Horizontal, tr("Property"));
  m_TableModel->setHeaderData(RIGHT_COLUMN, Qt::Horizontal, tr("Value"));

  QStandardItem* labelItems[9];
  labelItems[0] = new QStandardItem(tr("Intensity minimum"));
  labelItems[1] = new QStandardItem(tr("Intensity maximum"));
  labelItems[2] = new QStandardItem(tr("X dimension (pixels)"));
  labelItems[3] = new QStandardItem(tr("Y dimension (pixels)"));
  labelItems[4] = new QStandardItem(tr("Z dimension (slices)"));
  labelItems[5] = new QStandardItem(tr("X pixel size (µm)"));
  labelItems[6] = new QStandardItem(tr("Y pixel size (µm)"));
  labelItems[7] = new QStandardItem(tr("Z slice spacing (µm)"));
  labelItems[8] = new QStandardItem(tr("Z squish factor"));

  for (unsigned int i = 0; i < sizeof(labelItems) / sizeof(QStandardItem*); i++) {
    labelItems[i]->setEditable(false);
    m_TableModel->setItem(i, LEFT_COLUMN, labelItems[i]);

    QStandardItem* item = new QStandardItem(tr(""));

    // Allow editing of voxel spacing.
    if (i < 5)
      item->setEditable(false);
    m_TableModel->setItem(i, RIGHT_COLUMN, item);
  }
  this->imageDataView->setModel(m_TableModel);

#if 0
  connect(m_TableModel, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), 
    this, SLOT(handle_tableModel_dataChanged(const QModelIndex&, const QModelIndex&)));
#endif

  QCoreApplication::setOrganizationName("CISMM");
  QCoreApplication::setOrganizationDomain("cismm.org");
  QCoreApplication::setApplicationName("Fibrin Analysis");

  // Restore GUI settings.
  this->readProgramSettings();

  // Set up error dialog box.
  m_ErrorDialog.setModal(true);
}


// Destructor
FibrinAnalysis::~FibrinAnalysis() {
  delete m_DataModel;
  delete m_Visualization;
  delete m_TableModel;
}


void FibrinAnalysis::on_actionOpenSession_triggered() {
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
  stringValue = parser.GetValue("Image", "fileName");
  printf("Opening %s\n", stringValue.c_str());
  this->OpenFile(stringValue);

  // Load image settings
  double doubleValue3[3];
  parser.GetValueAsDouble3("Image", "voxelSpacing", doubleValue3);
  m_DataModel->SetVoxelSpacing(doubleValue3);

  // Set up visualization pipeline.
  m_Visualization->SetImageInputConnection(m_DataModel->GetImageOutputPort());
  m_Visualization->SetFilteredImageInputConnection(m_DataModel->GetFilteredImageOutputPort());

  // Filter settings
  doubleValue = parser.GetValueAsDouble("MultiscaleFibernessFilter", "alpha");
  m_DataModel->SetMultiscaleFibernessAlphaCoefficient(doubleValue);
  doubleValue = parser.GetValueAsDouble("MultiscaleFibernessFilter", "beta");
  m_DataModel->SetMultiscaleFibernessBetaCoefficient(doubleValue);
  doubleValue = parser.GetValueAsDouble("MultiscaleFibernessFilter", "gamma");
  m_DataModel->SetMultiscaleFibernessGammaCoefficient(doubleValue);
  doubleValue = parser.GetValueAsDouble("MultiscaleFibernessFilter", "minimumScale");
  m_DataModel->SetMultiscaleFibernessMinimumScale(doubleValue);
  doubleValue = parser.GetValueAsDouble("MultiscaleFibernessFilter", "maximumScale");
  m_DataModel->SetMultiscaleFibernessMaximumScale(doubleValue);
  intValue = parser.GetValueAsInt("MultiscaleFibernessFilter", "numberOfScales");
  m_DataModel->SetMultiscaleFibernessNumberOfScales(intValue);
  doubleValue = parser.GetValueAsDouble("MultiscaleFibernessFilterThreshold", "threshold");
  m_DataModel->SetMultiscaleFibernessThreshold(doubleValue);
  doubleValue = parser.GetValueAsDouble("FilteredImageOutput", "intensityScaleFactor");
  m_DataModel->SetFilteredImageScaleFactor(doubleValue);

  // Display settings
  boolValue = parser.GetValueAsBool("Display", "isosurfaceVisible");
  m_Visualization->SetIsosurfaceVisible(boolValue);
  doubleValue = parser.GetValueAsDouble("Display", "isovalue");
  m_Visualization->SetIsoValue(doubleValue);
  boolValue = parser.GetValueAsBool("Display", "imagePlaneVisible");
  m_Visualization->SetImagePlaneVisible(boolValue);
  intValue = parser.GetValueAsInt("Display", "zPlane");
  m_Visualization->SetZSlice(intValue);
  boolValue = parser.GetValueAsBool("Display", "cropIsosurface");
  m_Visualization->SetCropIsosurface(boolValue);
  intValue = parser.GetValueAsInt("Display", "keepPlanesAboveBelow");
  m_Visualization->SetKeepPlanesAboveBelowImagePlane(intValue);
  boolValue = parser.GetValueAsBool("Display", "fastIsosurfaceRendering");
  m_Visualization->SetFastIsosurfaceRendering(boolValue);
  boolValue = parser.GetValueAsBool("Display", "showDataOutline");
  m_Visualization->SetShowOutline(boolValue);

  this->RefreshUI();

  // Reset camera
  m_Renderer->ResetCamera();
  
  // Render
  this->qvtkWidget->GetRenderWindow()->Render();
}


void FibrinAnalysis::on_actionSaveSession_triggered() {
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

  fprintf(fp, "[Image]\n");
  fprintf(fp, "fileName=%s\n\n", m_DataModel->GetImageFileName().c_str());
  double spacing[3];
  m_DataModel->GetVoxelSpacing(spacing);
  fprintf(fp, "voxelSpacing=%f %f %f\n", spacing[0], spacing[1], spacing[2]);
  
  fprintf(fp, "[MultiscaleFibernessFilter]\n");
  fprintf(fp, "alpha=%.3f\n", m_DataModel->GetMultiscaleFibernessAlphaCoefficient());
  fprintf(fp, "beta=%.3f\n",  m_DataModel->GetMultiscaleFibernessBetaCoefficient());
  fprintf(fp, "gamma=%.3f\n", m_DataModel->GetMultiscaleFibernessGammaCoefficient());
  fprintf(fp, "minimumScale=%.3f\n", m_DataModel->GetMultiscaleFibernessMinimumScale());
  fprintf(fp, "maximumScale=%.3f\n", m_DataModel->GetMultiscaleFibernessMaximumScale());
  fprintf(fp, "numberOfScales=%d\n\n", m_DataModel->GetMultiscaleFibernessNumberOfScales());
  fprintf(fp, "[MultiscaleFibernessFilterThreshold]\n");
  fprintf(fp, "threshold=%.3f\n\n", m_DataModel->GetMultiscaleFibernessThreshold());

  fprintf(fp, "[FilteredImageOutput]\n");
  fprintf(fp, "intensityScaleFactor=%.3f\n\n", m_DataModel->GetFilteredImageScaleFactor());

  fprintf(fp, "[Display]\n");
  fprintf(fp, "isosurfaceVisible=%s\n", m_Visualization->GetIsosurfaceVisible() ? "true" : "false");
  fprintf(fp, "isovalue=%.3f\n", m_Visualization->GetIsoValue());
  fprintf(fp, "imagePlaneVisible=%s\n", m_Visualization->GetImagePlaneVisible() ? "true" : "false");
  fprintf(fp, "zPlane=%d\n", m_Visualization->GetZSlice());
  fprintf(fp, "cropIsosurface=%s\n", m_Visualization->GetCropIsosurface() ? "true" : "false");
  fprintf(fp, "keepPlanesAboveBelow=%d\n", m_Visualization->GetKeepPlanesAboveBelowImagePlane());
  fprintf(fp, "fastIsosurfaceRendering=%s\n", m_Visualization->GetFastIsosurfaceRendering() ? "true" : "false");
  fprintf(fp, "showDataOutline=%s\n", m_Visualization->GetShowOutline() ? "true" : "false");

  fclose(fp);

}


// Action to be taken upon file open 
void FibrinAnalysis::on_actionOpenImage_triggered() {

  // Locate file.
  QString fileName = QFileDialog::getOpenFileName(this, "Open Image Data", "", "TIF Images (*.tif);;VTK Images (*.vtk);;LSM Images (*.lsm);;");

  // Now read the file
  if (fileName == "") {
    return;
  }

  this->OpenFile(fileName.toStdString());

  // Set isovalue to midpoint between data min and max
  double min = m_DataModel->GetFilteredDataMinimum();
  double max = m_DataModel->GetFilteredDataMaximum();
  double isoValue = 0.5*(min + max);
  QString isoValueString = QString().sprintf("%.4f", isoValue);
  this->isoValueEdit->setText(isoValueString);
  this->isoValueSlider->setValue(this->isoValueSliderPosition(isoValue));

  // Set up visualization pipeline.
  m_Visualization->SetImageInputConnection(m_DataModel->GetImageOutputPort());
  m_Visualization->SetFilteredImageInputConnection(m_DataModel->GetFilteredImageOutputPort());
  m_Visualization->SetIsoValue(isoValue);

  // Refresh the UI
  this->RefreshUI();

  // Reset camera
  m_Renderer->ResetCamera();
  
  // Render
  this->qvtkWidget->GetRenderWindow()->Render();
}


void FibrinAnalysis::OpenFile(std::string fileName) {
  std::cout << "Loading file '" << fileName << "'" << std::endl;
  m_DataModel->LoadImageFile(fileName);
  
  // Set status bar with info about the file.
  QString imageInfo("Loaded image '"); imageInfo.append(fileName.c_str()); imageInfo.append("'.");
  this->statusbar->showMessage(imageInfo);
}


void FibrinAnalysis::on_actionSaveFilteredImage_triggered() {
  QString fileName = QFileDialog::getSaveFileName(this, "Save Filtered Image", "", "TIF Images (*.tif);;VTK (*.vtk);;");
  if (fileName == "")
    return;

  float scaleFactor = this->filteredImageScaleEdit->text().toDouble();
  m_DataModel->SaveFilteredImageFile(fileName.toStdString(), m_FilterType, scaleFactor);

}


void FibrinAnalysis::on_actionSaveFiberOrientationImage_triggered() {
  QString fileName = QFileDialog::getSaveFileName(this, "Save Fiber Orientation Image", "", "VTK (*.vtk);;");
  if (fileName == "")
    return;

  m_DataModel->SaveFiberOrientationImageFile(fileName.toStdString());
}


void FibrinAnalysis::on_actionSaveFiberOrientationData_triggered() {

  // Allow saving only when skeletonization filter is selected.
  if (m_FilterType == DataModel::MULTISCALE_SKELETONIZATION_FILTER_STRING) {
    QString fileName = QFileDialog::getSaveFileName(this, "Save Fiber Orientation Data", "", "CSV (*.csv);;");
    if (fileName == "")
      return;
    m_DataModel->SaveFiberOrientationDataFile(fileName.toStdString());

  } else {

    // Notify the user that skeletonization filter must be selected.
    QMessageBox messageBox;
    messageBox.setText("Error");
    messageBox.setInformativeText("You must have the Skeletonization filter selected in the Image Analysis dock to save the fiber orientation data.");
    messageBox.setStandardButtons(QMessageBox::Ok);
    messageBox.exec();
  }
}


void FibrinAnalysis::on_actionSavePicture_triggered() {
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


void FibrinAnalysis::on_actionSaveRotationAnimation_triggered() {
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

  vtkCamera* camera = m_Renderer->GetActiveCamera();

  int frames = 120;
  double angleIncrement = 360 / frames;
  for (int i = 0; i < frames; i++) {
    capturer->Modified();

    QString thisFileName = QString().sprintf("%s%04d%s",
      fileRoot.toStdString().c_str(), i, extension.toStdString().c_str());
    writer->SetFileName(thisFileName.toStdString().c_str());
    writer->Write();

    camera->Azimuth(angleIncrement);
    m_Renderer->ResetCameraClippingRange();
    qvtkWidget->GetRenderWindow()->Render();

    float progress = static_cast<float>(i) / static_cast<float>(frames);
    this->UpdateProgress(progress);
  }
  this->UpdateProgress(1.0);

  capturer->Delete();
  writer->Delete();
}


void FibrinAnalysis::on_actionSaveGeometry_triggered() {
  QString fileName = QFileDialog::getSaveFileName(this, "Save Geometry", "", "VTK (*.vtk);;");
  if (fileName == "")
    return;

  vtkPolyDataWriter* writer = vtkPolyDataWriter::New();
  writer->SetInputConnection(m_Visualization->GetIsosurfaceOutputPort());
  writer->SetFileName(fileName.toStdString().c_str());
  writer->Write();

  writer->Delete();
}


void FibrinAnalysis::on_actionExit_triggered() {
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


void FibrinAnalysis::on_actionResetView_triggered() {
  vtkCamera* camera = m_Renderer->GetActiveCamera();
  camera->SetFocalPoint(0, 0, 0);
  camera->SetPosition(0, 0, 1);
  camera->SetViewUp(0, 1, 0);
  m_Renderer->ResetCamera();
  this->qvtkWidget->GetRenderWindow()->Render();
}


void FibrinAnalysis::on_actionOpenView_triggered() {
  QString fileName = 
    QFileDialog::getOpenFileName(this, tr("Open Camera View File"), tr(""), 
				 tr("View Files (*.view);;"));
  if (fileName == "")
    return;

  vtkCamera* camera = m_Renderer->GetActiveCamera();

  ConfigurationFileParser parser;
  parser.Parse(fileName.toStdString());

  double values[3];
  parser.GetValueAsDouble3("Camera", "Position", values);
  camera->SetPosition(values);

  parser.GetValueAsDouble3("Camera", "FocalPoint", values);
  camera->SetFocalPoint(values);

  parser.GetValueAsDouble3("Camera", "UpVector", values);
  camera->SetViewUp(values);

  m_Renderer->ResetCameraClippingRange();
  this->qvtkWidget->GetRenderWindow()->Render();
}


void FibrinAnalysis::on_actionSaveView_triggered() {

  QString fileName = QFileDialog::getSaveFileName(this, 
    "Save Camera View File", "", "View files (*.view);;");
  if (fileName == "")
    return;

  FILE* fp = fopen(fileName.toStdString().c_str(), "w");
  if (fp == NULL) {
    m_ErrorDialog.showMessage(tr("Could not write view file ").append(fileName));
  }

  fprintf(fp, "; Written by Fibrin Analysis\n\n");
  fprintf(fp, "[Camera]\n");

  double* position = m_Renderer->GetActiveCamera()->GetPosition();
  fprintf(fp, "Position=%.4f %.4f %.4f\n", position[0], position[1], position[2]);

  double* focalPoint = m_Renderer->GetActiveCamera()->GetFocalPoint();
  fprintf(fp, "FocalPoint=%.4f %.4f %.4f\n", focalPoint[0], focalPoint[1], focalPoint[2]);

  double* upVector = m_Renderer->GetActiveCamera()->GetViewUp();
  fprintf(fp, "UpVector=%.4f %.4f %.4f\n", upVector[0], upVector[1], upVector[2]);

  fclose(fp);
}


void FibrinAnalysis::on_actionAboutFibrinAnalysis_triggered() {
  QString version = QString().sprintf("%d.%d.%d", 
				      FIBRIN_ANALYSIS_MAJOR_NUMBER,
				      FIBRIN_ANALYSIS_MINOR_NUMBER,
				      FIBRIN_ANALYSIS_REVISION_NUMBER);
  QChar copyright(169);
  QString title = QString("About Fibrin Analysis ").append(version);
  QString text  = QString("Fibrin Analysis ").append(version).append("\n");
  text.append(copyright).append(" 2009, UNC CISMM\n\n");
  text.append("Developed by Cory Quammen");
  QMessageBox::about(this, title, text);
}


void FibrinAnalysis::on_imageFilterComboBox_currentIndexChanged(QString filterText) {
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


void FibrinAnalysis::on_showIsosurfaceCheckBox_toggled(bool show) {
  m_Visualization->SetIsosurfaceVisible(show);
  this->qvtkWidget->GetRenderWindow()->Render();
}


void FibrinAnalysis::on_isoValueEdit_textEdited(QString text) {
  int value = static_cast<int>(text.toDouble());
  this->isoValueSlider->setValue(this->isoValueSliderPosition(value));
}


void FibrinAnalysis::on_isoValueSlider_sliderMoved(int value) {
  QString text = QString().sprintf("%.3f", this->isoValueSliderValue(value));
  this->isoValueEdit->setText(text);
}


void FibrinAnalysis::on_showZPlaneCheckbox_toggled(bool show) {
  m_Visualization->SetImagePlaneVisible(show);
  this->qvtkWidget->GetRenderWindow()->Render();
}


void FibrinAnalysis::on_zPlaneEdit_textEdited(QString text) {
  int value = static_cast<int>(text.toDouble()) - 1;
  this->zPlaneSlider->setValue(value);

  // Read z-slice.
  int dims[3];
  m_DataModel->GetDimensions(dims);
  int slice = zPlaneEdit->text().toInt()-1;
  if (slice >= 0 && slice < dims[2]) {
    m_Visualization->SetZSlice(slice);
    qvtkWidget->GetRenderWindow()->Render();
  }
}


void FibrinAnalysis::on_zPlaneSlider_sliderMoved(int value) {
  QString text = QString().sprintf("%d", value);
  this->zPlaneEdit->setText(text);

  // Read z-slice.
  int dims[3];
  m_DataModel->GetDimensions(dims);
  int slice = zPlaneEdit->text().toInt()-1;
  if (slice >= 0 && slice < dims[2]) {
    m_Visualization->SetZSlice(slice);
    qvtkWidget->GetRenderWindow()->Render();
  }
}


#if 0
void FibrinAnalysis::on_saveConnectedComponentsData_clicked() {
  QString fileName = QFileDialog::getSaveFileName(this, 
    "Save Connected Component vs. Threshold Data", "", "TXT (*.txt);;");
  if (fileName == "")
    return;
  
  double minThreshold = minThresholdEdit->text().toDouble();
  double maxThreshold = maxThresholdEdit->text().toDouble();
  double thresholdIncrement = 0.5;
  m_DataModel->ComputeConnectedComponentsVsThresholdData(minThreshold,
    maxThreshold, thresholdIncrement, fileName.toStdString());
}


void FibrinAnalysis::on_saveVolumeFractionEstimateData_clicked() {
  QString fileName = QFileDialog::getSaveFileName(this, 
    "Save Volume Fraction Estimate vs. Z Data", "", "TXT (*.txt);;");
  if (fileName == "")
    return;

  double threshold = fibernessThresholdEdit->text().toDouble();
  m_DataModel->ComputeVolumeFractionEstimateVsZData(threshold, fileName.toStdString());
}
#endif


void FibrinAnalysis::on_azimuthEdit_textEdited(QString text) {
  double azimuth = text.toDouble();
  m_DataModel->SetReferenceDirectionAzimuth(azimuth);
  m_Visualization->SetDirectionArrowAzimuth(azimuth);
  RefreshVisualization();
}


void FibrinAnalysis::on_inclinationEdit_textEdited(QString text) {
  double inclination = text.toDouble();
  m_DataModel->SetReferenceDirectionInclination(inclination);
  m_Visualization->SetDirectionArrowInclination(inclination);
  RefreshVisualization();
}


void FibrinAnalysis::on_showDirectionArrowCheckBox_toggled(bool state) {
  m_Visualization->SetDirectionArrowVisible(state);
  RefreshVisualization();
}


void FibrinAnalysis::on_saveAngleHistogram_clicked() {
  // Allow saving only when skeletonization filter is selected.
  //if (m_FilterType == DataModel::MULTISCALE_SKELETONIZATION_FILTER_STRING) {
  if (1) {
    QString fileName = QFileDialog::getSaveFileName(this, "Save Fiber Angle Histogram", "", "CSV (*.csv);;");
    if (fileName == "")
      return;

    double azimuth = azimuthEdit->text().toDouble();
    double inclination = inclinationEdit->text().toDouble();
    unsigned int bins = histogramBinsEdit->text().toUInt();
    m_DataModel->SaveFiberAngleHistogram
      (fileName.toStdString(), azimuth, inclination, bins);

  } else {

    // Notify the user that skeletonization filter must be selected.
    QMessageBox messageBox;
    messageBox.setText("Error");
    messageBox.setInformativeText("You must have the Skeletonization filter selected in the Image Analysis dock to save the fiber orientation data.");
    messageBox.setStandardButtons(QMessageBox::Ok);
    messageBox.exec();
  }
}


void FibrinAnalysis::on_showDataOutlineCheckBox_toggled(bool show) {
  m_Visualization->SetShowOutline(show);
  qvtkWidget->GetRenderWindow()->Render();
}


void FibrinAnalysis::on_cropIsosurfaceCheckBox_toggled(bool crop) {
  m_Visualization->SetCropIsosurface(crop);
  
  // Read z-slice.
  int dims[3];
  m_DataModel->GetDimensions(dims);
  int slice = zPlaneEdit->text().toInt()-1;
  if (slice >= 0 && slice < dims[2]) {
    m_Visualization->SetZSlice(slice);
    this->qvtkWidget->GetRenderWindow()->Render();
  }
}


void FibrinAnalysis::on_applyButton_clicked() {

  // Read fiber diameter.
  double fiberDiameter = this->fiberDiameterEdit->text().toDouble();
  m_DataModel->SetFiberDiameter(fiberDiameter);

  double fibernessAlphaCoef = this->fibernessAlphaCoefficientEdit->text().toDouble();
  m_DataModel->SetMultiscaleFibernessAlphaCoefficient(fibernessAlphaCoef);

  double fibernessBetaCoef = this->fibernessBetaCoefficientEdit->text().toDouble();
  m_DataModel->SetMultiscaleFibernessBetaCoefficient(fibernessBetaCoef);

  double fibernessGammaCoef = this->fibernessGammaCoefficientEdit->text().toDouble();
  m_DataModel->SetMultiscaleFibernessGammaCoefficient(fibernessGammaCoef);

  double fibernessScaleMinimum = this->fibernessScaleMinimumEdit->text().toDouble();
  m_DataModel->SetMultiscaleFibernessMinimumScale(fibernessScaleMinimum);

  double fibernessScaleMaximum = this->fibernessScaleMaximumEdit->text().toDouble();
  m_DataModel->SetMultiscaleFibernessMaximumScale(fibernessScaleMaximum);

  double fibernessNumberOfScales = this->fibernessNumberOfScalesEdit->text().toInt();
  m_DataModel->SetMultiscaleFibernessNumberOfScales(fibernessNumberOfScales);

  double fibernessThreshold = this->fibernessThresholdEdit->text().toDouble();
  m_DataModel->SetMultiscaleFibernessThreshold(fibernessThreshold);

  double junctionProbeDiameter = this->junctionProbeDiameterEdit->text().toDouble();
  m_DataModel->SetJunctionProbeDiameter(junctionProbeDiameter);

  double junctionFibernessThreshold = this->junctionFibernessThresholdEdit->text().toDouble();
  m_DataModel->SetJunctionFibernessThreshold(junctionFibernessThreshold);

  double junctionnessLocalMaxHeight = this->junctionnessLocalMaxHeightEdit->text().toDouble();
  m_DataModel->SetJunctionnessLocalMaxHeight(junctionnessLocalMaxHeight);

  double filteredImageScaleFactor = this->filteredImageScaleEdit->text().toDouble();
  m_DataModel->SetFilteredImageScaleFactor(filteredImageScaleFactor);

  ///////////////// Update image filters ////////////////
  QString text = this->imageFilterComboBox->currentText();
  if (text.toStdString() != m_FilterType) {
    if (text.toStdString() == DataModel::NO_FILTER_STRING) {
      m_DataModel->SetFilterToNone();
    } else if (text.toStdString() == DataModel::FRANGI_FIBERNESS_FILTER_STRING) {
      m_DataModel->SetFilterToFrangiFiberness();
    } else if (text.toStdString() == DataModel::MULTISCALE_FIBERNESS_FILTER_STRING) {
      m_DataModel->SetFilterToMultiscaleFiberness();
    } else if (text.toStdString() == DataModel::MULTISCALE_FIBERNESS_THRESHOLD_FILTER_STRING) {
      m_DataModel->SetFilterToMultiscaleFibernessThreshold();
    } else if (text.toStdString() == DataModel::MULTISCALE_SKELETONIZATION_FILTER_STRING) {
      m_DataModel->SetFilterToMultiscaleSkeletonization();
    } else if (text.toStdString() == DataModel::JUNCTIONNESS_FILTER_STRING) {
      m_DataModel->SetFilterToJunctionness();
    } else if (text.toStdString() == DataModel::JUNCTIONNESS_LOCAL_MAX_FILTER_STRING) {
      m_DataModel->SetFilterToJunctionnessLocalMax();
    }
    m_FilterType = text.toStdString();
  }

  ///////////////// Update image spacing settings ////////////////
  int index = 5;
  int column = 1;
  QStandardItem *item = NULL;
  
  // Get image spacing
  double spacing[3];

  item = m_TableModel->item(index++, column);
  spacing[0] = item->text().toDouble();

  item = m_TableModel->item(index++, column);
  spacing[1] = item->text().toDouble();

  item = m_TableModel->item(index++, column);
  spacing[2] = item->text().toDouble();

  m_DataModel->SetVoxelSpacing(spacing);

  double zSquish = 1.0;
  item = m_TableModel->item(index++, column);
  zSquish = item->text().toDouble();
  m_DataModel->SetZSquishFactor(zSquish);

  ///////////////// Update visualization settings /////////////////
  double isoValue = isoValueEdit->text().toDouble();
  m_Visualization->SetIsoValue(isoValue);

  int resampledDimensions[3];
  m_DataModel->GetResampledDimensions(resampledDimensions);
  int slice = zPlaneEdit->text().toInt()-1;
  if (slice >= 0 && slice < resampledDimensions[2]) {
    m_Visualization->SetZSlice(slice);
  }

  RefreshUI();
}


void FibrinAnalysis::handle_tableModel_dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight) {
  if (topLeft != bottomRight) {
    return;
  }
  
  QStandardItem* item = m_TableModel->item(topLeft.row(), topLeft.column());
  double value = item->text().toDouble();

  int itemIndex = topLeft.row();
  if (itemIndex == 5) {
    m_DataModel->SetVoxelXSpacing(value);
  } else if (itemIndex == 6) {
    m_DataModel->SetVoxelYSpacing(value);
  } else if (itemIndex == 7) {
    m_DataModel->SetVoxelZSpacing(value);
  } else if (itemIndex == 8) {
    // Set the z squish factor
    m_DataModel->SetZSquishFactor(value);
  }

  m_DataModel->MarkPipelineAsModified();

  RefreshUI();
}


void FibrinAnalysis::RefreshUI() {

  ///////////////// Update GUI /////////////////
  
  // Update window title
  QFileInfo fileInfo(m_DataModel->GetImageFileName().c_str());
  QString windowTitle("Fibrin Analysis");
  if (fileInfo.fileName() != "")
    windowTitle.append(tr(" - '").append(fileInfo.fileName()).append("'"));
  this->setWindowTitle(windowTitle);

  const char *decimalFormat = "%.3f";
  const char *intFormat = "%d";

  QString fiberDiameter = QString().sprintf(decimalFormat, m_DataModel->GetFiberDiameter());
  this->fiberDiameterEdit->setText(fiberDiameter);

  QString fibernessAlphaCoef = QString().sprintf(decimalFormat, m_DataModel->GetMultiscaleFibernessAlphaCoefficient());
  this->fibernessAlphaCoefficientEdit->setText(fibernessAlphaCoef);

  QString fibernessBetaCoef = QString().sprintf(decimalFormat, m_DataModel->GetMultiscaleFibernessBetaCoefficient());
  this->fibernessBetaCoefficientEdit->setText(fibernessBetaCoef);

  QString fibernessGammaCoef = QString().sprintf(decimalFormat, m_DataModel->GetMultiscaleFibernessGammaCoefficient());
  this->fibernessGammaCoefficientEdit->setText(fibernessGammaCoef);

  QString fibernessScaleMinimum = QString().sprintf(decimalFormat, m_DataModel->GetMultiscaleFibernessMinimumScale());
  this->fibernessScaleMinimumEdit->setText(fibernessScaleMinimum);

  QString fibernessScaleMaximum = QString().sprintf(decimalFormat, m_DataModel->GetMultiscaleFibernessMaximumScale());
  this->fibernessScaleMaximumEdit->setText(fibernessScaleMaximum);

  QString fibernessScales = QString().sprintf(intFormat, m_DataModel->GetMultiscaleFibernessNumberOfScales());
  this->fibernessNumberOfScalesEdit->setText(fibernessScales);

  QString fibernessThreshold = QString().sprintf(decimalFormat, m_DataModel->GetMultiscaleFibernessThreshold());
  this->fibernessThresholdEdit->setText(fibernessThreshold);
  
  QString junctionProbeFilterDiameter = QString().sprintf(decimalFormat, m_DataModel->GetJunctionProbeDiameter());
  this->junctionProbeDiameterEdit->setText(junctionProbeFilterDiameter);

  QString junctionFibernessThreshold = QString().sprintf(decimalFormat, m_DataModel->GetJunctionFibernessThreshold());
  this->junctionFibernessThresholdEdit->setText(junctionFibernessThreshold);

  QString junctionnessLocalMaxHeight = QString().sprintf(decimalFormat, m_DataModel->GetJunctionnessLocalMaxHeight());
  this->junctionnessLocalMaxHeightEdit->setText(junctionnessLocalMaxHeight);

  QString filteredImageScaleFactor = QString().sprintf(decimalFormat, m_DataModel->GetFilteredImageScaleFactor());
  this->filteredImageScaleEdit->setText(filteredImageScaleFactor);

  azimuthEdit->setText(QString().sprintf(decimalFormat, m_DataModel->GetReferenceDirectionAzimuth()));
  inclinationEdit->setText(QString().sprintf(decimalFormat, m_DataModel->GetReferenceDirectionInclination()));

  QString dataMin = QString().sprintf(decimalFormat, m_DataModel->GetFilteredDataMinimum());
  m_TableModel->item(0, 1)->setText(dataMin);
  QString dataMax = QString().sprintf(decimalFormat, m_DataModel->GetFilteredDataMaximum());
  m_TableModel->item(1, 1)->setText(dataMax);

  int dims[3];
  m_DataModel->GetDimensions(dims);
  QString xDim = QString().sprintf("%d", dims[0]);
  m_TableModel->item(2, 1)->setText(xDim);
  QString yDim = QString().sprintf("%d", dims[1]);
  m_TableModel->item(3, 1)->setText(yDim);
  QString zDim = QString().sprintf("%d", dims[2]);
  m_TableModel->item(4, 1)->setText(zDim);

  double spacing[3];
  m_DataModel->GetVoxelSpacing(spacing);
  QString xSpacing = QString().sprintf(decimalFormat, spacing[0]);
  m_TableModel->item(5, 1)->setText(xSpacing);

  QString ySpacing = QString().sprintf(decimalFormat, spacing[1]);
  m_TableModel->item(6, 1)->setText(ySpacing);

  QString zSpacing = QString().sprintf(decimalFormat, spacing[2]);
  m_TableModel->item(7, 1)->setText(zSpacing);

  QString zSquishFactor = 
    QString().sprintf(decimalFormat, m_DataModel->GetZSquishFactor());    
  m_TableModel->item(8, 1)->setText(zSquishFactor);

  showIsosurfaceCheckBox->setChecked(m_Visualization->GetIsosurfaceVisible());

  double isoValue = m_Visualization->GetIsoValue();
  QString isoValueString = QString().sprintf(decimalFormat, isoValue);
  isoValueEdit->setText(isoValueString);
  isoValueSlider->setValue(this->isoValueSliderPosition(isoValue));

  showZPlaneCheckbox->setChecked(m_Visualization->GetImagePlaneVisible());

  // Update slice slider
  zPlaneSlider->setMinValue(1);
  zPlaneSlider->setMaxValue(dims[2]);
  int zSlice = m_Visualization->GetZSlice()+1;
  zPlaneSlider->setValue(zSlice);
  QString zPlaneString = QString().sprintf(intFormat, zSlice);
  zPlaneEdit->setText(zPlaneString);

  // Update cropping widgets
  cropIsosurfaceCheckBox->setChecked(m_Visualization->GetCropIsosurface());
  QString keepPlanesString = QString().sprintf(intFormat, m_Visualization->GetKeepPlanesAboveBelowImagePlane());
  keepPlanesEdit->setText(keepPlanesString);

  fastRenderingCheckBox->setChecked(m_Visualization->GetFastIsosurfaceRendering());
  showDataOutlineCheckBox->setChecked(m_Visualization->GetShowOutline());

  RefreshVisualization();
}


void FibrinAnalysis::RefreshVisualization() {

  ///////////////// Update visualization stuff /////////////////
  m_Renderer->RemoveAllViewProps();

  if (m_DataModel->GetImageData()) {
    double arrowCenter[3];
    m_DataModel->GetImageCenter(arrowCenter);
    m_Visualization->SetDirectionArrowCenter(arrowCenter);

    double arrowScale = 0.333 * m_DataModel->GetMaxImageSize();
    m_Visualization->SetDirectionArrowScale(arrowScale);

    m_Visualization->SetImageInputConnection(m_DataModel->GetImageOutputPort());
    m_Visualization->SetFilteredImageInputConnection(m_DataModel->GetFilteredImageOutputPort());
    m_Visualization->AddToRenderer(m_Renderer);
    m_Visualization->SetFastIsosurfaceRendering(this->fastRenderingCheckBox->isChecked());
  }

  this->qvtkWidget->GetRenderWindow()->Render();
}


void FibrinAnalysis::UpdateProgress(float progress) const {
  int progressValue = static_cast<int>(progress*100.0);
  this->progressBar->setValue(progressValue);
}


void FibrinAnalysis::closeEvent(QCloseEvent* event) {
  this->writeProgramSettings();
}


void FibrinAnalysis::writeProgramSettings() {
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


void FibrinAnalysis::readProgramSettings() {
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


int FibrinAnalysis::isoValueSliderPosition(double value) {
  double dataMin = m_DataModel->GetFilteredDataMinimum();
  double dataMax = m_DataModel->GetFilteredDataMaximum();
  double sliderMax = static_cast<double>(this->isoValueSlider->maximum());
  return static_cast<int>(sliderMax*((value-dataMin)/(dataMax-dataMin)));
}


double FibrinAnalysis::isoValueSliderValue(int position) {
  double dataMin = m_DataModel->GetFilteredDataMinimum();
  double dataMax = m_DataModel->GetFilteredDataMaximum();
  double sliderMax = static_cast<double>(this->isoValueSlider->maximum());
  return (static_cast<double>(position) / sliderMax) 
    * (dataMax - dataMin) + dataMin;
}
