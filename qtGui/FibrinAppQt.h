#ifndef _FIBRIN_APP_QT_H_
#define _FIBRIN_APP_QT_H_

#include <qmainwindow.h>
#include <qstandarditemmodel.h>
#include "ui_FibrinAppQT.h"

#include "DataModel.h"
#include "Visualization.h"
#include "Types.h"

// Forward class declarations
class vtkRenderer;


class FibrinAppQt : public QMainWindow, private Ui_MainWindow
{
    Q_OBJECT

public:

  typedef std::string FilterType;
  static const FilterType NO_FILTER_STRING;
  static const FilterType VESSELNESS_FILTER_STRING;
  static const FilterType JUNCTIONNESS_FILTER_STRING;

  // Constructor/Destructor
  FibrinAppQt(QWidget* parent = 0);
  virtual ~FibrinAppQt();

  void UpdateProgress(float progress) const;

public slots:

  virtual void fileOpenImage();
  virtual void fileSaveFilteredImage();
  virtual void fileSavePicture();
  virtual void fileSaveRotationAnimation();
  virtual void fileSaveGeometry();

  virtual void fileExit();

  virtual void viewResetView();
  virtual void viewOpenView();
  virtual void viewSaveView();

  virtual void isoValueEditHandler(QString text);
  virtual void isoValueSliderHandler(int value);

  virtual void showDataOutlineHandler(bool show);

  virtual void applyButtonHandler();

protected:
  DataModel<Float3DImageType>* dataModel;
  Visualization* visualization;

  void refreshUI();

protected slots:

private:
  QStandardItemModel* tableModel;
  vtkRenderer* ren;

  FilterType filterType;
   
};

#endif // _FIBRIN_APP_QT_H_

