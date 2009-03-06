#ifndef _FIBRIN_APP_QT_H_
#define _FIBRIN_APP_QT_H_

#include "qmainwindow.h"
#include "ui_FibrinAppQT.h"

// Forward class declarations
class vtkCylinderSource;
class vtkPolyDataMapper;
class vtkExodusReader;
class vtkDataSetMapper;
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

     virtual void fileOpen();
     virtual void fileExit();

protected:

protected slots:

private:
     vtkCylinderSource* source;
     vtkPolyDataMapper* mapper;
     vtkActor* actor;
     vtkRenderer* ren;
   
};

#endif // _FIBRIN_APP_QT_H_

