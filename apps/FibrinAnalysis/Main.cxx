// Qt includes
#include <qapplication.h>

#include "FibrinAnalysis.h"

int main( int argc, char** argv )
{
  // Qt Stuff
  QApplication app( argc, argv );

  FibrinAnalysis mainwindow;
  mainwindow.show();

  return app.exec();
}

