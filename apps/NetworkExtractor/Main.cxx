// Qt includes
#include <qapplication.h>

#include "NetworkExtractor.h"

int main( int argc, char** argv )
{
  // Qt Stuff
  QApplication app( argc, argv );

  NetworkExtractor mainwindow;
  mainwindow.show();

  return app.exec();
}

