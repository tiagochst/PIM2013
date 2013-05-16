#ifndef WINDOW_H
#define WINDOW_H

#include "GLViewer.h"
#include <QMainWindow>
#include <QAction>
#include <QToolBar>
#include <QCheckBox>
#include <QGroupBox>
#include <QComboBox>
#include <QSlider>
#include <QLCDNumber>
#include <QSpinBox>
#include <QImage>
#include <QLabel>
#include <QCheckBox>
#include <QRadioButton>
#include <QDockWidget>
#include <QGroupBox>
#include <QButtonGroup>
#include <QMenuBar>
#include <QApplication>
#include <QLayout>
#include <QLabel>
#include <QProgressBar>
#include <QColorDialog>
#include <QLCDNumber>
#include <QPixmap>
#include <QFrame>
#include <QSplitter>
#include <QMenu>
#include <QScrollArea>
#include <QCoreApplication>
#include <QFont>
#include <QSizePolicy>
#include <QImageReader>
#include <QStatusBar>
#include <QPushButton>
#include <QMessageBox>
#include <QFileDialog>
#include <QStatusBar>
#include <QtGui/QFormLayout>
#include <vector>
#include <string>
#include <iostream>
#include <cstdio>
#include <cstdlib>

//#include "QTUtils.h"

/*!
 *  \brief A class that creates the User Interface. 
 *
 *  Contains:
 *  - Interface items 
 *  - SLOTs functions that are used to handle user actions 
 */
class Window : public QMainWindow {
    Q_OBJECT
public:
    Window();
    virtual ~Window();
    
public slots :
    void about ();
    void createMesh();
    void GLViewerHelp ();


private :
    void initControlWidget ();
    QGroupBox   * controlWidget;    //!< The Left dock group box.
    GLViewer    * viewer;            //!< The GLViewer (central window)
    QPushButton * createMeshPB;
  
private Q_SLOTS:
  // void rendererFinished ();
};

#endif // WINDOW_H
