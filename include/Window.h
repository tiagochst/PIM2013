#ifndef WINDOW_H
#define WINDOW_H

#include "GLViewer.h"
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QDockWidget>
#include <QtGui/QFormLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QWidget>
#include <QMainWindow>
#include <QToolBar>
#include <QCheckBox>
#include <QGroupBox>
#include <QSlider>
#include <QLCDNumber>
#include <QSpinBox>
#include <QImage>
#include <QCheckBox>
#include <QDockWidget>
#include <QGroupBox>
#include <QButtonGroup>
#include <QMenuBar>
#include <QApplication>
#include <QLayout>
#include <QLabel>
#include <QProgressBar>
#include <QColorDialog>
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
#include <QMessageBox>
#include <QFileDialog>
#include <QStatusBar>
#include <vector>
#include <string>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <QTextStream>
#include "Config.h"
#include "ParameterHandler.h"

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
    void saveGLImage ();
    void setFrame1(int iFrame);
    void setFrame2(int iFrame);
    void setMesh();
    void setDisplacement();

private :
    void initControlWidget ();
    void addImageItems();

    QGroupBox    * controlWidget;    //!< The Left dock group box.
    GLViewer     * viewer;            //!< The GLViewer (central window)
    QPushButton  * createMeshPB;
    QPushButton  * snapshotButton;
    QComboBox    * frame1ComboBox;
    QComboBox    * frame2ComboBox;
    QLabel       * myLabel;
    QWidget      * centralwidget;
};


#endif // WINDOW_H
