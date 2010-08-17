#include "NetworkExtractor.h"
#include "ConfigurationFileParser.h"
#include "Version.h"

#if defined(_WIN32) // Turn off deprecation warnings in Visual Studio
#pragma warning( disable : 4996 )
#endif

#include <QApplication>
#include <QCloseEvent>
#include <QErrorMessage>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QVariant>

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
NetworkExtractor::NetworkExtractor(QWidget* p) 
 : QMainWindow(p) {

  gui = new Ui_MainWindow();
  gui->setupUi(this);

  m_FilterType = DataModel::NO_FILTER_STRING;

  // QT/VTK interact
  m_Renderer = vtkRenderer::New();
  gui->qvtkWidget->GetRenderWindow()->AddRenderer(m_Renderer);

  // Instantiate data model.
  m_DataModel = new DataModel();
  m_DataModel->SetProgressCallback(ProgressCallback);

  // Instantiate visualization pipelines.
  m_Visualization = new Visualization();
  m_Visualization->SetDirectionArrowVisible(false);

  gui->imageFilterComboBox->addItem(QString(DataModel::NO_FILTER_STRING.c_str()));
  //gui->imageFilterComboBox->addItem(QString(DataModel::FRANGI_FIBERNESS_FILTER_STRING.c_str()));
  gui->imageFilterComboBox->addItem(QString(DataModel::MULTISCALE_FIBERNESS_FILTER_STRING.c_str()));
  gui->imageFilterComboBox->addItem(QString(DataModel::MULTISCALE_FIBERNESS_THRESHOLD_FILTER_STRING.c_str()));
  gui->imageFilterComboBox->addItem(QString(DataModel::MULTISCALE_SKELETONIZATION_FILTER_STRING.c_str()));
  //gui->imageFilterComboBox->addItem(QString(DataModel::JUNCTIONNESS_FILTER_STRING.c_str()));
  //gui->imageFilterComboBox->addItem(QString(DataModel::JUNCTIONNESS_LOCAL_MAX_FILTER_STRING.c_str()));

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
  gui->imageDataView->setModel(m_TableModel);

  QCoreApplication::setOrganizationName("CISMM");
  QCoreApplication::setOrganizationDomain("cismm.org");
  QCoreApplication::setApplicationName("Network Extractor");

  QString windowTitle = 
    QString().sprintf("Network Extractor %d.%d.%d", NETWORK_EXTRACTOR_MAJOR_NUMBER,
		    NETWORK_EXTRACTOR_MINOR_NUMBER,
		    NETWORK_EXTRACTOR_REVISION_NUMBER);
  setWindowTitle(windowTitle);

  // Restore GUI settings.
  ReadProgramSettings();

  // Set up error dialog box.
  m_ErrorDialog.setModal(true);
}


// Destructor
NetworkExtractor::~NetworkExtractor() {
  delete m_DataModel;
  delete m_Visualization;
  delete m_TableModel;
}


void NetworkExtractor::on_actionOpenSession_triggered() {
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
  OpenFile(stringValue);

  // Load image settings
  double doubleValue3[3];
  parser.GetValueAsDouble3("Image", "voxelSpacing", doubleValue3);
  m_DataModel->SetVoxelSpacing(doubleValue3);
  double zSquishFactor = parser.GetValueAsDouble("Image", "zSquishFactor");
  m_DataModel->SetZSquishFactor(zSquishFactor);

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
  boolValue = parser.GetValueAsBool("Display", "showDataOutline");
  m_Visualization->SetShowOutline(boolValue);

  RefreshUI();

  // Reset camera
  m_Renderer->ResetCamera();
  
  // Render
  gui->qvtkWidget->GetRenderWindow()->Render();
}


void NetworkExtractor::on_actionSaveSession_triggered() {
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

  fprintf(fp, "; Written by Network Extractor\n\n");

  fprintf(fp, "[Image]\n");
  fprintf(fp, "fileName=%s\n", m_DataModel->GetImageFileName().c_str());
  double spacing[3];
  m_DataModel->GetVoxelSpacing(spacing);
  fprintf(fp, "voxelSpacing=%f %f %f\n", spacing[0], spacing[1], spacing[2]);
  fprintf(fp, "zSquishFactor=%f\n\n", m_DataModel->GetZSquishFactor());
  
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
  fprintf(fp, "showDataOutline=%s\n", m_Visualization->GetShowOutline() ? "true" : "false");

  fclose(fp);

}


// Action to be taken upon file open 
void NetworkExtractor::on_actionOpenImage_triggered() {

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
  gui->isoValueEdit->setText(isoValueString);
  gui->isoValueSlider->setValue(this->IsoValueSliderPosition(isoValue));

  // Set up visualization pipeline.
  m_Visualization->SetImageInputConnection(m_DataModel->GetImageOutputPort());
  m_Visualization->SetFilteredImageInputConnection(m_DataModel->GetFilteredImageOutputPort());
  m_Visualization->SetIsoValue(isoValue);

  // Refresh the UI
  RefreshUI();

  // Reset camera
  m_Renderer->ResetCamera();
  
  // Render
  gui->qvtkWidget->GetRenderWindow()->Render();
}


void NetworkExtractor::OpenFile(std::string fileName) {
  std::cout << "Loading file '" << fileName << "'" << std::endl;
  m_DataModel->LoadImageFile(fileName);
  
  // Set status bar with info about the file.
  QString imageInfo("Loaded image '"); imageInfo.append(fileName.c_str()); imageInfo.append("'.");
  gui->statusbar->showMessage(imageInfo);
}


void NetworkExtractor::on_actionSaveFilteredImage_triggered() {
  QString fileName = QFileDialog::getSaveFileName(this, "Save Filtered Image", "", "TIF Images (*.tif);;VTK (*.vtk);;");
  if (fileName == "")
    return;

  float scaleFactor = gui->filteredImageScaleEdit->text().toDouble();
  m_DataModel->SaveFilteredImageFile(fileName.toStdString(), m_FilterType, scaleFactor);

}


void NetworkExtractor::on_actionSaveFiberOrientationImage_triggered() {

  if (m_FilterType == DataModel::MULTISCALE_FIBERNESS_FILTER_STRING) {
    QString fileName = QFileDialog::getSaveFileName(this, "Save Fiber Orientation Image", "", "VTK (*.vtk);;");
    if (fileName == "")
      return;

    m_DataModel->SaveFiberOrientationImageFile(fileName.toStdString());

  } else {

    // Notify the user that skeletonization filter must be selected.
    QMessageBox messageBox;
    messageBox.setText("Error");
    messageBox.setInformativeText("You must have the Multiscale Fiberness filter selected in the Image Analysis dock to save the fiber orientation image.");
    messageBox.setStandardButtons(QMessageBox::Ok);
    messageBox.exec();
  }
}


void NetworkExtractor::on_actionSaveFiberOrientationData_triggered() {

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


void NetworkExtractor::on_actionSavePicture_triggered() {
  QString fileName = QFileDialog::getSaveFileName(this, "Save Picture", "", "PNG (*.png);;");
  if (fileName == "")
    return;

  vtkWindowToImageFilter* capturer = vtkWindowToImageFilter::New();
  capturer->SetInput(gui->qvtkWidget->GetRenderWindow());

  vtkPNGWriter* writer = vtkPNGWriter::New();
  writer->SetInputConnection(capturer->GetOutputPort());
  writer->SetFileName(fileName.toStdString().c_str());
  writer->Write();

  capturer->Delete();
  writer->Delete();
}


void NetworkExtractor::on_actionSaveRotationAnimation_triggered() {
  QString fileName = QFileDialog::getSaveFileName(this, "Save Rotation Animation", "", "PNG (*.png);;");
  if (fileName == "")
    return;

  // Chop off extension and save it for later.
  QString fileRoot = fileName.left(fileName.size()-4);
  QString extension = fileName.right(4);
  std::cout << extension.toStdString() << std::endl;
  
  vtkWindowToImageFilter* capturer = vtkWindowToImageFilter::New();
  capturer->SetInput(gui->qvtkWidget->GetRenderWindow());

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
    gui->qvtkWidget->GetRenderWindow()->Render();

    float progress = static_cast<float>(i) / static_cast<float>(frames);
    this->UpdateProgress(progress);
  }
  UpdateProgress(1.0);

  capturer->Delete();
  writer->Delete();
}


void NetworkExtractor::on_actionSaveGeometry_triggered() {
  QString fileName = QFileDialog::getSaveFileName(this, "Save Geometry", "", "VTK (*.vtk);;");
  if (fileName == "")
    return;

  vtkSmartPointer<vtkContourFilter> isosurfaceFilter = 
    vtkSmartPointer<vtkContourFilter>::New();
  isosurfaceFilter->SetInputConnection(m_DataModel->GetImageOutputPort());
  isosurfaceFilter->SetNumberOfContours(1);
  isosurfaceFilter->SetValue(0, m_Visualization->GetIsoValue());
  isosurfaceFilter->Update();

  vtkPolyDataWriter* writer = vtkPolyDataWriter::New();
  writer->SetInputConnection(isosurfaceFilter->GetOutputPort());
  writer->SetFileName(fileName.toStdString().c_str());
  writer->Write();
  writer->Delete();
}


void NetworkExtractor::on_actionExit_triggered() {
  Exit();
}


void NetworkExtractor::on_actionResetView_triggered() {
  vtkCamera* camera = m_Renderer->GetActiveCamera();
  camera->SetFocalPoint(0, 0, 0);
  camera->SetPosition(0, 0, 1);
  camera->SetViewUp(0, 1, 0);
  m_Renderer->ResetCamera();
  gui->qvtkWidget->GetRenderWindow()->Render();
}


void NetworkExtractor::on_actionOpenView_triggered() {
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
  gui->qvtkWidget->GetRenderWindow()->Render();
}


void NetworkExtractor::on_actionSaveView_triggered() {

  QString fileName = QFileDialog::getSaveFileName(this, 
    "Save Camera View File", "", "View files (*.view);;");
  if (fileName == "")
    return;

  FILE* fp = fopen(fileName.toStdString().c_str(), "w");
  if (fp == NULL) {
    m_ErrorDialog.showMessage(tr("Could not write view file ").append(fileName));
  }

  fprintf(fp, "; Written by Network Extractor\n\n");
  fprintf(fp, "[Camera]\n");

  double* position = m_Renderer->GetActiveCamera()->GetPosition();
  fprintf(fp, "Position=%.4f %.4f %.4f\n", position[0], position[1], position[2]);

  double* focalPoint = m_Renderer->GetActiveCamera()->GetFocalPoint();
  fprintf(fp, "FocalPoint=%.4f %.4f %.4f\n", focalPoint[0], focalPoint[1], focalPoint[2]);

  double* upVector = m_Renderer->GetActiveCamera()->GetViewUp();
  fprintf(fp, "UpVector=%.4f %.4f %.4f\n", upVector[0], upVector[1], upVector[2]);

  fclose(fp);
}


void NetworkExtractor::on_actionAboutNetworkExtractor_triggered() {
  QString version = QString().sprintf("%d.%d.%d", 
				      NETWORK_EXTRACTOR_MAJOR_NUMBER,
				      NETWORK_EXTRACTOR_MINOR_NUMBER,
				      NETWORK_EXTRACTOR_REVISION_NUMBER);
  QChar copyright(169);
  QString title = QString("About Network Extractor ").append(version);
  QString text  = QString("Network Extractor ").append(version).append("\n");
  text.append(copyright).append(" 2009-10, UNC CISMM\n\n");
  text.append("Developed by Cory Quammen");
  QMessageBox::about(this, title, text);
}


void NetworkExtractor::on_imageFilterComboBox_currentIndexChanged(QString filterText) {
  gui->fibernessSettingsWidget->setVisible(false);
  gui->multiscaleFibernessSettingsWidget->setVisible(false);
  gui->multiscaleFibernessThresholdSettingsWidget->setVisible(false);
  gui->junctionnessSettingsWidget->setVisible(false);
  gui->junctionnessLocalMaxSettingsWidget->setVisible(false);

  if (filterText.toStdString() == DataModel::NO_FILTER_STRING) {

  } else if (filterText.toStdString() == DataModel::FRANGI_FIBERNESS_FILTER_STRING) {
    gui->fibernessSettingsWidget->setVisible(true);
  } else if (filterText.toStdString() == DataModel::MULTISCALE_FIBERNESS_FILTER_STRING) {
    gui->multiscaleFibernessSettingsWidget->setVisible(true);
  } else if (filterText.toStdString() == DataModel::MULTISCALE_FIBERNESS_THRESHOLD_FILTER_STRING) {
    gui->multiscaleFibernessSettingsWidget->setVisible(true);
    gui->multiscaleFibernessThresholdSettingsWidget->setVisible(true);
  } else if (filterText.toStdString() == DataModel::MULTISCALE_SKELETONIZATION_FILTER_STRING) {
    gui->multiscaleFibernessSettingsWidget->setVisible(true);
    gui->multiscaleFibernessThresholdSettingsWidget->setVisible(true);
  } else if (filterText.toStdString() == DataModel::JUNCTIONNESS_FILTER_STRING) {
    gui->fibernessSettingsWidget->setVisible(true);
    gui->junctionnessSettingsWidget->setVisible(true);
  } else if (filterText.toStdString() == DataModel::JUNCTIONNESS_LOCAL_MAX_FILTER_STRING) {
    gui->fibernessSettingsWidget->setVisible(true);
    gui->junctionnessSettingsWidget->setVisible(true);
    gui->junctionnessLocalMaxSettingsWidget->setVisible(true);
  }
}


void NetworkExtractor::on_showIsosurfaceCheckBox_toggled(bool show) {
  m_Visualization->SetIsosurfaceVisible(show);
  gui->qvtkWidget->GetRenderWindow()->Render();
}


void NetworkExtractor::on_isoValueEdit_textEdited(QString text) {
  int value = static_cast<int>(text.toDouble());
  gui->isoValueSlider->setValue(this->IsoValueSliderPosition(value));
}


void NetworkExtractor::on_isoValueSlider_sliderMoved(int value) {
  QString text = QString().sprintf("%.3f", this->IsoValueSliderValue(value));
  gui->isoValueEdit->setText(text);
}


void NetworkExtractor::on_showZPlaneCheckbox_toggled(bool show) {
  m_Visualization->SetImagePlaneVisible(show);
  gui->qvtkWidget->GetRenderWindow()->Render();
}


void NetworkExtractor::on_zPlaneEdit_textEdited(QString text) {
  int value = static_cast<int>(text.toDouble());
  gui->zPlaneSlider->setValue(value);

  SetZPlane(value);
}


void NetworkExtractor::on_zPlaneSlider_sliderMoved(int value) {
  QString text = QString().sprintf("%d", value);
  gui->zPlaneEdit->setText(text);

  SetZPlane(value);
}


void NetworkExtractor::SetZPlane(int plane) {
  int dims[3];
  m_DataModel->GetResampledDimensions(dims);
  plane--;
  if (plane >= 0 && plane < dims[2]) {
    m_Visualization->SetZSlice(plane);
  } else if (plane < 0) {
    m_Visualization->SetZSlice(0);
  } else {
    m_Visualization->SetZSlice(dims[2]-1);
  }

  gui->qvtkWidget->GetRenderWindow()->Render();
}


#if 0
void NetworkExtractor::on_saveConnectedComponentsData_clicked() {
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


void NetworkExtractor::on_saveVolumeFractionEstimateData_clicked() {
  QString fileName = QFileDialog::getSaveFileName(this, 
    "Save Volume Fraction Estimate vs. Z Data", "", "TXT (*.txt);;");
  if (fileName == "")
    return;

  double threshold = fibernessThresholdEdit->text().toDouble();
  m_DataModel->ComputeVolumeFractionEstimateVsZData(threshold, fileName.toStdString());
}
#endif


void NetworkExtractor::on_azimuthEdit_textEdited(QString text) {
  double azimuth = text.toDouble();
  m_DataModel->SetReferenceDirectionAzimuth(azimuth);
  m_Visualization->SetDirectionArrowAzimuth(azimuth);
  gui->qvtkWidget->GetRenderWindow()->Render();
}


void NetworkExtractor::on_inclinationEdit_textEdited(QString text) {
  double inclination = text.toDouble();
  m_DataModel->SetReferenceDirectionInclination(inclination);
  m_Visualization->SetDirectionArrowInclination(inclination);
  gui->qvtkWidget->GetRenderWindow()->Render();
}


void NetworkExtractor::on_showDirectionArrowCheckBox_toggled(bool state) {
  m_Visualization->SetDirectionArrowVisible(state);
  RefreshVisualization();
}


void NetworkExtractor::on_saveAngleHistogram_clicked() {
  // Allow saving only when skeletonization filter is selected.
  if (m_FilterType == DataModel::MULTISCALE_SKELETONIZATION_FILTER_STRING) {
    QString fileName = QFileDialog::getSaveFileName(this, "Save Fiber Angle Histogram", "", "CSV (*.csv);;");
    if (fileName == "")
      return;

    double azimuth = gui->azimuthEdit->text().toDouble();
    double inclination = gui->inclinationEdit->text().toDouble();
    unsigned int bins = gui->histogramBinsEdit->text().toUInt();
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


void NetworkExtractor::on_showDataOutlineCheckBox_toggled(bool show) {
  m_Visualization->SetShowOutline(show);
  gui->qvtkWidget->GetRenderWindow()->Render();
}


void NetworkExtractor::on_cropIsosurfaceCheckBox_toggled(bool crop) {
  m_Visualization->SetCropIsosurface(crop);
  
  // Read z-plane.
  int plane = gui->zPlaneEdit->text().toInt();
  SetZPlane(plane);
}


void NetworkExtractor::on_applyButton_clicked() {

  // Read fiber diameter.
  double fiberDiameter = gui->fiberDiameterEdit->text().toDouble();
  m_DataModel->SetFiberDiameter(fiberDiameter);

  double fibernessAlphaCoef = gui->fibernessAlphaCoefficientEdit->text().toDouble();
  m_DataModel->SetMultiscaleFibernessAlphaCoefficient(fibernessAlphaCoef);

  double fibernessBetaCoef = gui->fibernessBetaCoefficientEdit->text().toDouble();
  m_DataModel->SetMultiscaleFibernessBetaCoefficient(fibernessBetaCoef);

  double fibernessGammaCoef = gui->fibernessGammaCoefficientEdit->text().toDouble();
  m_DataModel->SetMultiscaleFibernessGammaCoefficient(fibernessGammaCoef);

  double fibernessScaleMinimum = gui->fibernessScaleMinimumEdit->text().toDouble();
  m_DataModel->SetMultiscaleFibernessMinimumScale(fibernessScaleMinimum);

  double fibernessScaleMaximum = gui->fibernessScaleMaximumEdit->text().toDouble();
  m_DataModel->SetMultiscaleFibernessMaximumScale(fibernessScaleMaximum);

  double fibernessNumberOfScales = gui->fibernessNumberOfScalesEdit->text().toInt();
  m_DataModel->SetMultiscaleFibernessNumberOfScales(fibernessNumberOfScales);

  double fibernessThreshold = gui->fibernessThresholdEdit->text().toDouble();
  m_DataModel->SetMultiscaleFibernessThreshold(fibernessThreshold);

  double junctionProbeDiameter = gui->junctionProbeDiameterEdit->text().toDouble();
  m_DataModel->SetJunctionProbeDiameter(junctionProbeDiameter);

  double junctionFibernessThreshold = gui->junctionFibernessThresholdEdit->text().toDouble();
  m_DataModel->SetJunctionFibernessThreshold(junctionFibernessThreshold);

  double junctionnessLocalMaxHeight = gui->junctionnessLocalMaxHeightEdit->text().toDouble();
  m_DataModel->SetJunctionnessLocalMaxHeight(junctionnessLocalMaxHeight);

  double filteredImageScaleFactor = gui->filteredImageScaleEdit->text().toDouble();
  m_DataModel->SetFilteredImageScaleFactor(filteredImageScaleFactor);

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

  ///////////////// Update image filters ////////////////
  QString text = gui->imageFilterComboBox->currentText();
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

  ///////////////// Update visualization settings /////////////////
  double isoValue = gui->isoValueEdit->text().toDouble();
  m_Visualization->SetIsoValue(isoValue);

  int plane = gui->zPlaneEdit->text().toInt();
  SetZPlane(plane);

  int keepPlanes = gui->keepPlanesEdit->text().toInt();
  m_Visualization->SetKeepPlanesAboveBelowImagePlane(keepPlanes);

  RefreshUI();
}


void NetworkExtractor::handle_tableModel_dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight) {
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

  RefreshUI();
}


void NetworkExtractor::RefreshUI() {

  ///////////////// Update GUI /////////////////
  
  // Update window title
  QFileInfo fileInfo(m_DataModel->GetImageFileName().c_str());
  QString windowTitle = 
    QString().sprintf("Network Extractor %d.%d.%d", NETWORK_EXTRACTOR_MAJOR_NUMBER,
		    NETWORK_EXTRACTOR_MINOR_NUMBER,
		    NETWORK_EXTRACTOR_REVISION_NUMBER);
					
  if (fileInfo.fileName() != "")
    windowTitle.append(tr(" - '").append(fileInfo.fileName()).append("'"));
  setWindowTitle(windowTitle);

  const char *decimalFormat = "%.3f";
  const char *intFormat = "%d";

  QString fiberDiameter = QString().sprintf(decimalFormat, m_DataModel->GetFiberDiameter());
  gui->fiberDiameterEdit->setText(fiberDiameter);

  QString fibernessAlphaCoef = QString().sprintf(decimalFormat, m_DataModel->GetMultiscaleFibernessAlphaCoefficient());
  gui->fibernessAlphaCoefficientEdit->setText(fibernessAlphaCoef);

  QString fibernessBetaCoef = QString().sprintf(decimalFormat, m_DataModel->GetMultiscaleFibernessBetaCoefficient());
  gui->fibernessBetaCoefficientEdit->setText(fibernessBetaCoef);

  QString fibernessGammaCoef = QString().sprintf(decimalFormat, m_DataModel->GetMultiscaleFibernessGammaCoefficient());
  gui->fibernessGammaCoefficientEdit->setText(fibernessGammaCoef);

  QString fibernessScaleMinimum = QString().sprintf(decimalFormat, m_DataModel->GetMultiscaleFibernessMinimumScale());
  gui->fibernessScaleMinimumEdit->setText(fibernessScaleMinimum);

  QString fibernessScaleMaximum = QString().sprintf(decimalFormat, m_DataModel->GetMultiscaleFibernessMaximumScale());
  gui->fibernessScaleMaximumEdit->setText(fibernessScaleMaximum);

  QString fibernessScales = QString().sprintf(intFormat, m_DataModel->GetMultiscaleFibernessNumberOfScales());
  gui->fibernessNumberOfScalesEdit->setText(fibernessScales);

  QString fibernessThreshold = QString().sprintf(decimalFormat, m_DataModel->GetMultiscaleFibernessThreshold());
  gui->fibernessThresholdEdit->setText(fibernessThreshold);
  
  QString junctionProbeFilterDiameter = QString().sprintf(decimalFormat, m_DataModel->GetJunctionProbeDiameter());
  gui->junctionProbeDiameterEdit->setText(junctionProbeFilterDiameter);

  QString junctionFibernessThreshold = QString().sprintf(decimalFormat, m_DataModel->GetJunctionFibernessThreshold());
  gui->junctionFibernessThresholdEdit->setText(junctionFibernessThreshold);

  QString junctionnessLocalMaxHeight = QString().sprintf(decimalFormat, m_DataModel->GetJunctionnessLocalMaxHeight());
  gui->junctionnessLocalMaxHeightEdit->setText(junctionnessLocalMaxHeight);

  QString filteredImageScaleFactor = QString().sprintf(decimalFormat, m_DataModel->GetFilteredImageScaleFactor());
  gui->filteredImageScaleEdit->setText(filteredImageScaleFactor);

  gui->azimuthEdit->setText(QString().sprintf(decimalFormat, m_DataModel->GetReferenceDirectionAzimuth()));
  gui->inclinationEdit->setText(QString().sprintf(decimalFormat, m_DataModel->GetReferenceDirectionInclination()));

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

  gui->showIsosurfaceCheckBox->setChecked(m_Visualization->GetIsosurfaceVisible());

  double isoValue = m_Visualization->GetIsoValue();
  QString isoValueString = QString().sprintf(decimalFormat, isoValue);
  gui->isoValueEdit->setText(isoValueString);
  gui->isoValueSlider->setValue(this->IsoValueSliderPosition(isoValue));

  gui->showZPlaneCheckbox->setChecked(m_Visualization->GetImagePlaneVisible());

  // Update slice slider
  int resampledDims[3];
  m_DataModel->GetResampledDimensions(resampledDims);
  gui->zPlaneSlider->setMinValue(1);
  gui->zPlaneSlider->setMaxValue(resampledDims[2]);
  int zSlice = m_Visualization->GetZSlice()+1;
  gui->zPlaneSlider->setValue(zSlice);
  QString zPlaneString = QString().sprintf(intFormat, zSlice);
  gui->zPlaneEdit->setText(zPlaneString);

  // Update cropping widgets
  gui->cropIsosurfaceCheckBox->setChecked(m_Visualization->GetCropIsosurface());
  QString keepPlanesString = QString().sprintf(intFormat, m_Visualization->GetKeepPlanesAboveBelowImagePlane());
  gui->keepPlanesEdit->setText(keepPlanesString);

  gui->showDataOutlineCheckBox->setChecked(m_Visualization->GetShowOutline());

  RefreshVisualization();
}


void NetworkExtractor::RefreshVisualization() {

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
  }

  gui->qvtkWidget->GetRenderWindow()->GetInteractor()->SetDesiredUpdateRate(4.0);
  gui->qvtkWidget->GetRenderWindow()->Render();
}


void NetworkExtractor::UpdateProgress(float progress) const {
  int progressValue = static_cast<int>(progress*100.0);
  gui->progressBar->setValue(progressValue);
}


void NetworkExtractor::closeEvent(QCloseEvent* event) {
  Exit();

  // If we made it past the call above, the user clicked cancel.
  event->ignore();
}


void NetworkExtractor::Exit() {
  // Ask if user really wants to quit.
  QMessageBox messageBox;
  messageBox.setText("Do you really want to exit?");
  messageBox.setInformativeText("If you exit now, all unsaved settings will be lost.");
  messageBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
  messageBox.setDefaultButton(QMessageBox::Cancel);
  int selected = messageBox.exec();

  if (selected == QMessageBox::Ok) {
    WriteProgramSettings();
    qApp->exit();
  }
}


void NetworkExtractor::WriteProgramSettings() {
  QSettings settings;

  settings.beginGroup("MainWindow");
  settings.setValue("WindowSettings", saveState());
  settings.setValue("Geometry", saveGeometry());
  settings.endGroup();
}


void NetworkExtractor::ReadProgramSettings() {
  QSettings settings;

  settings.beginGroup("MainWindow");
  restoreState(settings.value("WindowSettings").toByteArray());
  restoreGeometry(settings.value("Geometry").toByteArray());
  settings.endGroup();
}


int NetworkExtractor::IsoValueSliderPosition(double value) {
  double dataMin = m_DataModel->GetFilteredDataMinimum();
  double dataMax = m_DataModel->GetFilteredDataMaximum();
  double sliderMax = static_cast<double>(gui->isoValueSlider->maximum());
  return static_cast<int>(sliderMax*((value-dataMin)/(dataMax-dataMin)));
}


double NetworkExtractor::IsoValueSliderValue(int position) {
  double dataMin = m_DataModel->GetFilteredDataMinimum();
  double dataMax = m_DataModel->GetFilteredDataMaximum();
  double sliderMax = static_cast<double>(gui->isoValueSlider->maximum());
  return (static_cast<double>(position) / sliderMax) 
    * (dataMax - dataMin) + dataMin;
}
