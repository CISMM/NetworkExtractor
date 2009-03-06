#include <qapplication.h>
#include <qfiledialog.h>
#include <qmenubar.h>

#include "FibrinAppQT.h"
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkCylinderSource.h>
#include <vtkPolyDataMapper.h>


// Constructor
FibrinAppQt::FibrinAppQt(QWidget* p) 
 : QMainWindow(p)
{
  setupUi(this);

  // QT/VTK interact
  ren = vtkRenderer::New();
  qvtkWidget->GetRenderWindow()->AddRenderer(ren);

  connect(actionOpenImage, SIGNAL(triggered()), this, SLOT(fileOpen()));
  connect(actionExit, SIGNAL(triggered()), this, SLOT(fileExit()));
  
};
   
// Action to be taken upon file open 
void FibrinAppQt::fileOpen()
{
  // Geometry
  source = vtkCylinderSource::New();

  // Mapper
  mapper = vtkPolyDataMapper::New();
  mapper->ImmediateModeRenderingOn();
  mapper->SetInputConnection(source->GetOutputPort());

  // Actor in scene
  actor = vtkActor::New();
  actor->SetMapper(mapper);

  // Add Actor to renderer
  ren->AddActor(actor);

  // Reset camera
  ren->ResetCamera();

  ren->GetRenderWindow()->Render();
}

void FibrinAppQt::fileExit() {
  qApp->exit();
}

