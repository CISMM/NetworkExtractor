README.txt
Last updated: 1/5/2010, Cory Quammen <cquammen@cs.unc.edu>

--------------
 DEPENDENCIES
--------------

VTK 5.4.2
---------

Configure with:

BUILD_SHARED_LIBS  On
VTK_USE_GUISUPPORT On
VTK_USE_QT         On
VTK_USE_INFOVIS    On
VTK_USE_VIEWS      On


ITK 3.16
--------

Configure with:

BUILD_SHARED_LIBS                       On
ITK_USE_REVIEW                          On

QT 4.5.3
--------

Obtain from ftp://ftp.qt.nokia.com/qt/source/qt-all-opensource-src-4.5.3.zip

Configure with:

configure -platform win32-msvc2005 -no-webkit -no-dbus -no-phonon -debug-and-release -opensource -shared

The -no-webkit, -no-dbus, and -no-phonon options are not required.
