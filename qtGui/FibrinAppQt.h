#ifndef _FIBRIN_APP_QT_H_
#define _FIBRIN_APP_QT_H_

#include <qerrormessage.h>
#include <qmainwindow.h>
#include <qstandarditemmodel.h>
#include "ui_FibrinAppQt.h"

#include "DataModel.h"
#include "Visualization.h"
#include "Types.h"

// Forward class declarations
class vtkRenderer;


class FibrinAppQt : public QMainWindow, private Ui_MainWindow
{
    Q_OBJECT

public:

  // Convenience typedefs
  typedef DataModel<Float3DImageType> DataModelType;

  // Constructor/Destructor
  FibrinAppQt(QWidget* parent = 0);
  virtual ~FibrinAppQt();

  void UpdateProgress(float progress) const;

protected:
  void closeEvent(QCloseEvent* event);

  void writeProgramSettings();
  void readProgramSettings();

  int isoValueSliderPosition(double value);
  double isoValueSliderValue(int position);

public slots:

  // Use Qt's auto-connect magic to tie GUI widgets to slots.
  // Names of the methods must follow the naming convention
  // on_<widget name>_<signal name>(<signal parameters>).
  virtual void on_actionOpenImage_triggered();
  virtual void on_actionSaveFilteredImage_triggered();
  virtual void on_actionSaveFiberOrientationImage_triggered();

  virtual void on_actionSaveFiberOrientationData_triggered();

  virtual void on_actionSavePicture_triggered();
  virtual void on_actionSaveRotationAnimation_triggered();
  virtual void on_actionSaveGeometry_triggered();
  virtual void on_actionExit_triggered();

  virtual void on_actionResetView_triggered();
  virtual void on_actionOpenView_triggered();
  virtual void on_actionSaveView_triggered();

  virtual void on_imageFilterComboBox_currentIndexChanged(QString filterText);

  virtual void on_saveConnectedComponentsData_clicked();
  virtual void on_saveVolumeFractionEstimateData_clicked();

  virtual void on_showIsosurfaceCheckbox_toggled(bool show);
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
  QStandardItemModel* tableModel;
  DataModelType* dataModel;
  Visualization* visualization;

  void refreshUI();

protected slots:

private:
  vtkRenderer* ren;

  DataModelType::FilterType filterType;

  QErrorMessage errorDialog;
   
};


#endif // _FIBRIN_APP_QT_H_
