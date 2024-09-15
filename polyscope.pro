#-------------------------------------------------
#
# Project created by QtCreator 2019-03-11T10:52:35
#
#-------------------------------------------------

QT       += core gui widgets
QT       += opengl openglwidgets

TARGET = polyscope
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11
CONFIG      +=  warn_on thread

INCLUDEPATH += $$quote(./qwtplot3d/include)

# the following adds version information to the build
# this can be referenced in the source code using REVISION
# in revision.h in the project root directory

macx {
    equals(QMAKE_HOST.arch, arm64) {
        SVN=/opt/homebrew/bin/svn
    } else {
        SVN=/usr/local/bin/svn
    }
}

unix: !macx {
    SVN=svn
}

win32 {
    SVN=$$quote(svn.exe)
}


SVN_VERSION = $$system($$SVN info $$PWD --show-item revision)

message( "SVN_VERSION = " $$SVN_VERSION)

win32 {
    RC = $$system( echo const int REVISION=$${SVN_VERSION}; > revision.h )
} else {
    RC = $$system( echo const int REVISION=$${SVN_VERSION}\; > revision.h )
}





SOURCES += \
    additive.cpp \
    additiveclusterlist.cpp \
    additivelist.cpp \
    additivetablewidget.cpp \
    comboboxdelegate.cpp \
    definedtablewidget.cpp \
    exportgriddialog.cpp \
    exposuredialog.cpp \
        main.cpp \
        mainwindow.cpp \
    grid.cpp \
    grow.cpp \
    interface.cpp \
    monomer.cpp \
    monomerlist.cpp \
    monomersequence.cpp \
    monomertablewidget.cpp \
    randomtablewidget.cpp \
    sequencetablewidget.cpp \
    vector.cpp \
    qwtplot3d/src/qwt3d_appearance.cpp \
    qwtplot3d/src/qwt3d_autoscaler.cpp \
    qwtplot3d/src/qwt3d_axis.cpp \
    qwtplot3d/src/qwt3d_color_std.cpp \
    qwtplot3d/src/qwt3d_colorlegend.cpp \
    qwtplot3d/src/qwt3d_coordsys.cpp \
    qwtplot3d/src/qwt3d_data.cpp \
    qwtplot3d/src/qwt3d_drawable.cpp \
    qwtplot3d/src/qwt3d_enrichment_std.cpp \
    qwtplot3d/src/qwt3d_extglwidget.cpp \
    qwtplot3d/src/qwt3d_function.cpp \
    qwtplot3d/src/qwt3d_graphplot.cpp \
    qwtplot3d/src/qwt3d_gridmapping.cpp \
    qwtplot3d/src/qwt3d_gridplot.cpp \
    qwtplot3d/src/qwt3d_io_reader.cpp \
    qwtplot3d/src/qwt3d_io.cpp \
    qwtplot3d/src/qwt3d_label.cpp \
    qwtplot3d/src/qwt3d_lighting.cpp \
    qwtplot3d/src/qwt3d_meshplot.cpp \
    qwtplot3d/src/qwt3d_parametricsurface.cpp \
    qwtplot3d/src/qwt3d_plot3d.cpp \
    qwtplot3d/src/qwt3d_scale.cpp \
    qwtplot3d/src/qwt3d_surfaceplot.cpp \
    qwtplot3d/src/qwt3d_types.cpp \
    qwtplot3d/src/qwt3d_volumeplot.cpp \
    chaingraph.cpp \
    colorring.cpp \
    chainlist.cpp \
    random.cpp
HEADERS += \
    additive.h \
    additiveclusterlist.h \
    additivelist.h \
    additivetablewidget.h \
    coloration.h \
    comboboxdelegate.h \
    definedtablewidget.h \
    exportgriddialog.h \
    exposuredialog.h \
        mainwindow.h \
    grid.h \
    grow.h \
    monomer.h \
    monomerlist.h \
    monomersequence.h \
    monomertablewidget.h \
    randomtablewidget.h \
    sequencetablewidget.h \
    vector.h \
    interface.h \
    qwtplot3d/include/qwt3d_appearance.h \
    qwtplot3d/include/qwt3d_autoscaler.h \
    qwtplot3d/include/qwt3d_axis.h \
    qwtplot3d/include/qwt3d_color_std.h \
    qwtplot3d/include/qwt3d_color.h \
    qwtplot3d/include/qwt3d_colorlegend.h \
    qwtplot3d/include/qwt3d_coordsys.h \
    qwtplot3d/include/qwt3d_data.h \
    qwtplot3d/include/qwt3d_drawable.h \
    qwtplot3d/include/qwt3d_enrichment_std.h \
    qwtplot3d/include/qwt3d_enrichment.h \
    qwtplot3d/include/qwt3d_extglwidget.h \
    qwtplot3d/include/qwt3d_function.h \
    qwtplot3d/include/qwt3d_global.h \
    qwtplot3d/include/qwt3d_graphplot.h \
    qwtplot3d/include/qwt3d_gridmapping.h \
    qwtplot3d/include/qwt3d_gridplot.h \
    qwtplot3d/include/qwt3d_helper.h \
    qwtplot3d/include/qwt3d_io_reader.h \
    qwtplot3d/include/qwt3d_io.h \
    qwtplot3d/include/qwt3d_label.h \
    qwtplot3d/include/qwt3d_mapping.h \
    qwtplot3d/include/qwt3d_meshplot.h \
    qwtplot3d/include/qwt3d_openglhelper.h \
    qwtplot3d/include/qwt3d_parametricsurface.h \
    qwtplot3d/include/qwt3d_plot3d.h \
    qwtplot3d/include/qwt3d_portability.h \
    qwtplot3d/include/qwt3d_scale.h \
    qwtplot3d/include/qwt3d_surfaceplot.h \
    qwtplot3d/include/qwt3d_types.h \
    qwtplot3d/include/qwt3d_valueptr.h \
    qwtplot3d/include/qwt3d_volumeplot.h \
    chaingraph.h \
    colorring.h \
    chainlist.h \
    random.h

FORMS += \
        exportgriddialog.ui \
        exposuredialog.ui \
        mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target



win32{
  win32-msvc2008 | win32-msvc2010 | win32-msvc2012 | win32-msvc2013 | win32-msvc2015 {
    QMAKE_CXXFLAGS += -MP
    QMAKE_CXXFLAGS += $$QMAKE_CFLAGS_STL
  }

    LIBS += -lopengl32 -lglu32 -lgdi32
}

unix:!macx { LIBS +=  -lGLU }

macx: LIBS +=  -framework OpenGL

linux-g++:QMAKE_CXXFLAGS += -fno-exceptions

RESOURCES += \
    images.qrc

DISTFILES += \
    LICENSE.TXT
