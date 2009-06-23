// Qt includes
#include <qapplication.h>

#include "FibrinAppQt.h"

int main( int argc, char** argv )
{
  // Qt Stuff
  QApplication app( argc, argv );

  FibrinAppQt mainwindow;
  mainwindow.show();

  return app.exec();
}

