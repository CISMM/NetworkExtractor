#include "FibrinAppQt.h"

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

#include <XMLHelper.h>
#include <libxml/encoding.h>
#include <libxml/xmlreader.h>
#include <libxml/xmlwriter.h>


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

  this->filterType = DataModelType::NO_FILTER_STRING;

  // QT/VTK interact
  this->ren = vtkRenderer::New();
  this->qvtkWidget->GetRenderWindow()->AddRenderer(ren);

  // Instantiate data model.
  this->dataModel = new DataModel<Float3DImageType>();
  this->dataModel->SetProgressCallback(ProgressCallback);

  // Instantiate visualization pipelines.
  this->visualization = new Visualization();

  this->imageFilterComboBox->addItem(QString(DataModelType::NO_FILTER_STRING.c_str()));
  //this->imageFilterComboBox->addItem(QString(DataModelType::FRANGI_FIBERNESS_FILTER_STRING.c_str()));
  this->imageFilterComboBox->addItem(QString(DataModelType::MULTISCALE_FIBERNESS_FILTER_STRING.c_str()));
  this->imageFilterComboBox->addItem(QString(DataModelType::MULTISCALE_FIBERNESS_THRESHOLD_FILTER_STRING.c_str()));
  this->imageFilterComboBox->addItem(QString(DataModelType::MULTISCALE_SKELETONIZATION_FILTER_STRING.c_str()));
  //this->imageFilterComboBox->addItem(QString(DataModelType::JUNCTIONNESS_FILTER_STRING.c_str()));
  //this->imageFilterComboBox->addItem(QString(DataModelType::JUNCTIONNESS_LOCAL_MAX_FILTER_STRING.c_str()));

  // Create and populate table model.
  int LEFT_COLUMN = 0;
  int RIGHT_COLUMN = 1;
  this->tableModel = new QStandardItemModel(8, 2, this);
  this->tableModel->setHeaderData(LEFT_COLUMN,  Qt::Horizontal, tr("Property"));
  this->tableModel->setHeaderData(RIGHT_COLUMN, Qt::Horizontal, tr("Value"));

  QStandardItem* labelItems[8];
  labelItems[0] = new QStandardItem(tr("Intensity minimum"));
  labelItems[1] = new QStandardItem(tr("Intensity maximum"));
  labelItems[2] = new QStandardItem(tr("X dimension (pixels)"));
  labelItems[3] = new QStandardItem(tr("Y dimension (pixels)"));
  labelItems[4] = new QStandardItem(tr("Z dimension (slices)"));
  labelItems[5] = new QStandardItem(tr("X pixel size (µm)"));
  labelItems[6] = new QStandardItem(tr("Y pixel size (µm)"));
  labelItems[7] = new QStandardItem(tr("Z slice spacing (µm)"));

  for (int i = 0; i < 8; i++) {
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

#if 0
  // Initialize the XML library.
  LIBXML_TEST_VERSION
#endif

  // Set up error dialog box.
  this->errorDialog.setModal(true);
}


// Destructor
FibrinAppQt::~FibrinAppQt() {
  delete this->dataModel;
  delete this->visualization;
  delete this->tableModel;

#if 0
  // Cleanup the XML library.
  xmlCleanupParser();
#endif
}


// Action to be taken upon file open 
void FibrinAppQt::on_actionOpenImage_triggered() {

  // Locate file.
  QString fileName = QFileDialog::getOpenFileName(this, "Open Image Data", "", "TIF Images (*.tif);;VTK Images (*.vtk);;LSM Images (*.lsm);;");

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
  QString isoValueString = QString().sprintf("%.4f", isoValue);
  this->isoValueEdit->setText(isoValueString);
  this->isoValueSlider->setValue(this->isoValueSliderPosition(isoValue));

  // Set up visualization pipeline.
  this->visualization->SetImageInputConnection(this->dataModel->GetImageOutputPort());
  this->visualization->SetFilteredImageInputConnection(this->dataModel->GetFilteredImageOutputPort());
  this->visualization->SetIsoValue(isoValue);

  // Refresh the UI
  this->refreshUI();

  // Reset camera
  this->ren->ResetCamera();
  
  // Render
  this->qvtkWidget->GetRenderWindow()->Render();
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
  if (this->filterType == DataModelType::MULTISCALE_SKELETONIZATION_FILTER_STRING) {
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
  QString fileName = QFileDialog::getOpenFileName(this, 
    "Open Camera View File", "", "XML (*.xml);;");
  if (fileName == "")
    return;

#if 0
  int rc;
  xmlDoc* doc = xmlReadFile(fileName.toStdString().c_str(), NULL, 0);

  if (doc == NULL) {
    errorDialog.showMessage(tr("Could not parse view file."));
    return;
  }

  double x, y, z;
  xmlNode* rootElement = xmlDocGetRootElement(doc);

  xmlNode* positionNode = FindChildNodeWithName(rootElement, "Position"); //root_element->children;
  if (positionNode) {
    sscanf((char *) xmlGetProp(positionNode, BAD_CAST "x"), "%lf", &x);
    sscanf((char *) xmlGetProp(positionNode, BAD_CAST "y"), "%lf", &y);
    sscanf((char *) xmlGetProp(positionNode, BAD_CAST "z"), "%lf", &z);
    this->ren->GetActiveCamera()->SetPosition(x, y, z);
  }

  xmlNode* focalPointNode = FindChildNodeWithName(rootElement, "FocalPoint"); //positionNode->next;
  if (focalPointNode) {
    sscanf((char *) xmlGetProp(focalPointNode, BAD_CAST "x"), "%lf", &x);
    sscanf((char *) xmlGetProp(focalPointNode, BAD_CAST "y"), "%lf", &y);
    sscanf((char *) xmlGetProp(focalPointNode, BAD_CAST "z"), "%lf", &z);
    this->ren->GetActiveCamera()->SetFocalPoint(x, y, z);
  }

  xmlNode* upVectorNode = FindChildNodeWithName(rootElement, "UpVector"); //focalPointNode->next;
  if (upVectorNode) {
    sscanf((char *) xmlGetProp(upVectorNode, BAD_CAST "x"), "%lf", &x);
    sscanf((char *) xmlGetProp(upVectorNode, BAD_CAST "y"), "%lf", &y);
    sscanf((char *) xmlGetProp(upVectorNode, BAD_CAST "z"), "%lf", &z);
    this->ren->GetActiveCamera()->SetViewUp(x, y, z);
  }

  xmlFreeDoc(doc);
#endif

  this->ren->ResetCameraClippingRange();
  this->qvtkWidget->GetRenderWindow()->Render();
}


void FibrinAppQt::on_actionSaveView_triggered() {

  QString fileName = QFileDialog::getSaveFileName(this, 
    "Save Camera View File", "", "XML (*.xml);;");
  if (fileName == "")
    return;

#if 0
  xmlTextWriterPtr writer;
  int rc;
  char charBuffer[20];

  // Create a new xmlWriter for file path with no compression.
  writer = xmlNewTextWriterFilename(fileName.toStdString().c_str(), 0);
  if (writer == NULL) {
    errorDialog.showMessage(tr("Could not write view file."));
  }

  // Start the document with XML default for version and encoding (ISO 8859-1).
  rc = xmlTextWriterStartDocument(writer, NULL, "ISO-8859-1", NULL);

  // Start an element named "Camera". This will be the root element of the document.
  rc = xmlTextWriterStartElement(writer, BAD_CAST "Camera");

  // Start a camera position element.
  rc = xmlTextWriterStartElement(writer, BAD_CAST "Position");
  double* position = this->ren->GetActiveCamera()->GetPosition();
  sprintf(charBuffer, "%.3f", position[0]);
  rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "x", BAD_CAST charBuffer);
  sprintf(charBuffer, "%.3f", position[1]);
  rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "y", BAD_CAST charBuffer);
  sprintf(charBuffer, "%.3f", position[2]);
  rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "z", BAD_CAST charBuffer);
  rc = xmlTextWriterEndElement(writer); // Position element

  // Start a focal point element.
  rc = xmlTextWriterStartElement(writer, BAD_CAST "FocalPoint");
  double* focalPoint = this->ren->GetActiveCamera()->GetFocalPoint();
  sprintf(charBuffer, "%.3f", focalPoint[0]);
  rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "x", BAD_CAST charBuffer);
  sprintf(charBuffer, "%.3f", focalPoint[1]);
  rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "y", BAD_CAST charBuffer);
  sprintf(charBuffer, "%.3f", focalPoint[2]);
  rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "z", BAD_CAST charBuffer);
  rc = xmlTextWriterEndElement(writer); // Focal point element

  // Start an up vector element.
  rc = xmlTextWriterStartElement(writer, BAD_CAST "UpVector");
  double* upVector = this->ren->GetActiveCamera()->GetViewUp();
  sprintf(charBuffer, "%.3f", upVector[0]);
  rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "x", BAD_CAST charBuffer);
  sprintf(charBuffer, "%.3f", upVector[1]);
  rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "y", BAD_CAST charBuffer);
  sprintf(charBuffer, "%.3f", upVector[2]);
  rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "z", BAD_CAST charBuffer);
  rc = xmlTextWriterEndElement(writer); // Up vector element.

  rc = xmlTextWriterEndElement(writer); // View element
  rc = xmlTextWriterEndDocument(writer);

  xmlFreeTextWriter(writer);
#endif

}


void FibrinAppQt::on_imageFilterComboBox_currentIndexChanged(QString filterText) {
  this->fibernessSettingsWidget->setVisible(false);
  this->multiscaleFibernessSettingsWidget->setVisible(false);
  this->multiscaleFibernessThresholdSettingsWidget->setVisible(false);
  this->junctionnessSettingsWidget->setVisible(false);
  this->junctionnessLocalMaxSettingsWidget->setVisible(false);

  if (filterText.toStdString() == DataModelType::NO_FILTER_STRING) {

  } else if (filterText.toStdString() == DataModelType::FRANGI_FIBERNESS_FILTER_STRING) {
    this->fibernessSettingsWidget->setVisible(true);
  } else if (filterText.toStdString() == DataModelType::MULTISCALE_FIBERNESS_FILTER_STRING) {
    this->multiscaleFibernessSettingsWidget->setVisible(true);
  } else if (filterText.toStdString() == DataModelType::MULTISCALE_FIBERNESS_THRESHOLD_FILTER_STRING) {
    this->multiscaleFibernessSettingsWidget->setVisible(true);
    this->multiscaleFibernessThresholdSettingsWidget->setVisible(true);
  } else if (filterText.toStdString() == DataModelType::MULTISCALE_SKELETONIZATION_FILTER_STRING) {
    this->multiscaleFibernessSettingsWidget->setVisible(true);
    this->multiscaleFibernessThresholdSettingsWidget->setVisible(true);
  } else if (filterText.toStdString() == DataModelType::JUNCTIONNESS_FILTER_STRING) {
    this->fibernessSettingsWidget->setVisible(true);
    this->junctionnessSettingsWidget->setVisible(true);
  } else if (filterText.toStdString() == DataModelType::JUNCTIONNESS_LOCAL_MAX_FILTER_STRING) {
    this->fibernessSettingsWidget->setVisible(true);
    this->junctionnessSettingsWidget->setVisible(true);
    this->junctionnessLocalMaxSettingsWidget->setVisible(true);
  }
}


void FibrinAppQt::on_showIsosurfaceCheckbox_toggled(bool show) {
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
    if (filterText.toStdString() == DataModelType::NO_FILTER_STRING) {
      this->dataModel->SetFilterToNone();
    } else if (filterText.toStdString() == DataModelType::FRANGI_FIBERNESS_FILTER_STRING) {
      this->dataModel->SetFilterToFrangiFiberness();
    } else if (filterText.toStdString() == DataModelType::MULTISCALE_FIBERNESS_FILTER_STRING) {
      this->dataModel->SetFilterToMultiscaleFiberness();
    } else if (filterText.toStdString() == DataModelType::MULTISCALE_FIBERNESS_THRESHOLD_FILTER_STRING) {
      this->dataModel->SetFilterToMultiscaleFibernessThreshold();
    } else if (filterText.toStdString() == DataModelType::MULTISCALE_SKELETONIZATION_FILTER_STRING) {
      this->dataModel->SetFilterToMultiscaleSkeletonization();
    } else if (filterText.toStdString() == DataModelType::JUNCTIONNESS_FILTER_STRING) {
      this->dataModel->SetFilterToJunctionness();
    } else if (filterText.toStdString() == DataModelType::JUNCTIONNESS_LOCAL_MAX_FILTER_STRING) {
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

  refreshUI();
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
  }

  this->qvtkWidget->GetRenderWindow()->Render();
}


void FibrinAppQt::refreshUI() {

  ///////////////// Update GUI /////////////////
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

  double isoValue = this->visualization->GetIsoValue();
  QString isoValueString = QString().sprintf(decimalFormat, isoValue);
  this->isoValueEdit->setText(isoValueString);
  this->isoValueSlider->setValue(this->isoValueSliderPosition(isoValue));

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
