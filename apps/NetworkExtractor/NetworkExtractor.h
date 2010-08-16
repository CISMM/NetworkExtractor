#ifndef _NETWORK_EXTRACTOR_QT_H_
#define _NETWORK_EXTRACTOR_QT_H_

#include <QErrorMessage>
#include <QMainWindow>
#include <QStandardItemModel>

#include "ui_NetworkExtractor.h"

#include "DataModel.h"
#include "Visualization.h"
#include "Types.h"

// Forward class declarations
class vtkRenderer;


class NetworkExtractor : public QMainWindow
{
    Q_OBJECT

public:

  // Constructor/Destructor
  NetworkExtractor(QWidget* parent = 0);
  virtual ~NetworkExtractor();

  void UpdateProgress(float progress) const;

protected:
  Ui_MainWindow* gui;

  void Exit();
  void WriteProgramSettings();
  void ReadProgramSettings();

  // Override the closeEvent handler.
  void closeEvent(QCloseEvent* event);

  int IsoValueSliderPosition(double value);
  double IsoValueSliderValue(int position);

public slots:

  // Use Qt's auto-connect magic to tie GUI widgets to slots.
  // Names of the methods must follow the naming convention
  // on_<widget name>_<signal name>(<signal parameters>).
  virtual void on_actionOpenImage_triggered();
  virtual void on_actionSaveFilteredImage_triggered();
  virtual void on_actionSaveFiberOrientationImage_triggered();

  virtual void on_actionSaveFiberOrientationData_triggered();

  virtual void on_actionOpenSession_triggered();
  virtual void on_actionSaveSession_triggered();

  virtual void on_actionSavePicture_triggered();
  virtual void on_actionSaveRotationAnimation_triggered();
  virtual void on_actionSaveGeometry_triggered();
  virtual void on_actionExit_triggered();

  virtual void on_actionResetView_triggered();
  virtual void on_actionOpenView_triggered();
  virtual void on_actionSaveView_triggered();

  virtual void on_actionAboutNetworkExtractor_triggered();

  virtual void on_imageFilterComboBox_currentIndexChanged(QString filterText);

#if 0
  virtual void on_saveConnectedComponentsData_clicked();
  virtual void on_saveVolumeFractionEstimateData_clicked();
#endif

  virtual void on_showDirectionArrowCheckBox_toggled(bool state);
  virtual void on_azimuthEdit_textEdited(QString text);
  virtual void on_inclinationEdit_textEdited(QString text);
  virtual void on_saveAngleHistogram_clicked();

  virtual void on_showIsosurfaceCheckBox_toggled(bool show);
  virtual void on_isoValueEdit_textEdited(QString text);
  virtual void on_isoValueSlider_sliderMoved(int value);
  
  virtual void on_showZPlaneCheckbox_toggled(bool show);
  virtual void on_zPlaneEdit_textEdited(QString text);
  virtual void on_zPlaneSlider_sliderMoved(int value);

  virtual void on_showDataOutlineCheckBox_toggled(bool show);

  virtual void on_cropIsosurfaceCheckBox_toggled(bool crop);

  virtual void on_applyButton_clicked();

  virtual void handle_tableModel_dataChanged(const QModelIndex& topLeft,
    const QModelIndex& bottomRight);

protected:
  QStandardItemModel *m_TableModel;
  DataModel          *m_DataModel;
  Visualization      *m_Visualization;

  void OpenFile(std::string fileName);

  void RefreshUI();

  void RefreshVisualization();

  void SetZPlane(int plane);

protected slots:

private:
  vtkRenderer* m_Renderer;

  DataModel::FilterType m_FilterType;

  QErrorMessage m_ErrorDialog;
   
};


#endif // _NETWORK_EXTRACTOR_QT_H_
