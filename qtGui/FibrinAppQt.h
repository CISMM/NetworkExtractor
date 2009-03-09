#ifndef _FIBRIN_APP_QT_H_
#define _FIBRIN_APP_QT_H_

#include "ui_FibrinAppQT.h"
#include "Types.h"

#include "qmainwindow.h"

// Forward class declarations
class vtkContourFilter;
class vtkPolyDataMapper;
class vtkActor;
class vtkRenderer;


class FibrinAppQt : public QMainWindow, private Ui_MainWindow
{
    Q_OBJECT

public:

  // Constructor/Destructor
  FibrinAppQt(QWidget* parent = 0);
  ~FibrinAppQt() {};

public slots:

  virtual void fileOpenImage();
  virtual void fileOpenView();
  virtual void fileSaveView();
  virtual void fileExit();

  virtual void applyButtonHandler();

protected:

protected slots:

private:

  UShort3DImageType::Pointer imageData;
  UShort3DExporterType::Pointer exporter;

  vtkContourFilter* contourer;
  vtkPolyDataMapper* mapper;
  vtkActor* actor;
  vtkRenderer* ren;
   
};

#endif // _FIBRIN_APP_QT_H_

