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
#include <QtGui/QStackedWidget>
#include <QMainWindow>
#include <QToolBar>
#include <QCheckBox>
#include <QSlider>
#include <QLCDNumber>
#include <QSpinBox>
#include <QImage>
#include <QCheckBox>
#include <QDockWidget>
#include <QMenuBar>
#include <QApplication>
#include <QLayout>
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
#include <QDebug>
#include <QProgressDialog>
#include <QtGui/QLabel>
#include <QtGui/QListWidget>
#include "Config.h"
#include "ParameterHandler.h"
#include "Camera.h"
#include "FileWriterServices.h"

const int MAXFRAMES = 999;

/*!
 *  \brief A class used to treats the reference frame for
 *         automatic anchoring frames. 
 */
class AnchorLabel : public QLabel
{
Q_OBJECT

signals:
   void mousePressed();
   void mousePressed(int framID);

public:
    AnchorLabel( const QString& text, QWidget* parent = 0);
    AnchorLabel( QWidget* parent = 0);
    ~AnchorLabel(){}
    int  frameID;
    int  oldFrameStyle; 

public slots:
    void slotClicked();	

protected:
    void mousePressEvent( QMouseEvent* ev );

};


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

signals:
    void exiting ();

public slots:
    void about ();
    void createMesh();
    void GLViewerHelp ();
    void saveGLImage ();
    void setFrame1(int iFrame);
    void setFrame2(int iFrame);
    void setMesh(bool b);
    void setDisplacement(bool b);
    void createDock();
    void calcDisp();
    void setManuAnchor(bool b);
    void setAutoAnchor(bool b);
    void updateImages();
    void startCapture();
    void enableCaptureButton();
    void exitPreprocess();
    void addNewAnchorItem();
    void removeAnchorItem();
    void updateManuAnchorPreview();
    void updateAutoAnchorPreview();
    void subtractShowingFrames();
    void sumShowingFrames();
    void findAutoAnchors();
    void setReferenceFrame(int iFrame);
    void setThreshold( double iThreshold);
    void setWindowSize(int iSize);
    void setNeighbourhoodSize(int  iSize);
    void setNearPlane ( const unsigned int& iNear );
    void setFarPlane ( const unsigned int& iFar );

private :
    void initControlWidget ();
    void initManuAnchorSelection ();
    void initAutoAnchorSelection ();
    void addAnchorListItems();
    void addImageItems();
    void loadAnchorFrames();
    void saveAnchors();
    void updateFrameList();

    QGroupBox       *   controlWidget;    //!< The Left dock group box.
    QStackedWidget  *   centerWidget;
    QDockWidget     *   controlDockWidget;

    /* Left dock buttons */
    QPushButton     *   createMeshPB;
    QPushButton     *   calcDispPB;
    QPushButton     *   snapshotButton;
    QPushButton     *   startCaptureButton;
    QComboBox       *   frame1ComboBox;
    QComboBox       *   frame2ComboBox;
    QRadioButton    *   displacementRB;
    QRadioButton    *   meshRB;
    QRadioButton    *   anchorManuRB;
    QRadioButton    *   anchorAutoRB;
    QWidget         *   generalLayoutWidget ;
    QFormLayout     *   generalFormLayout;
    QSpinBox        *   windowSizeSP;
    QSpinBox        *   neighbourhoodSizeSP;
    QSpinBox        *   nearPlaneSP;
    QSpinBox        *   farPlaneSP;


    /* Mesh widgets*/
    GLViewer        *   viewer;           //!< The GLViewer (central window)
    int                 viewerIdx;
    QTimer          *   cameraTimer;
    QProgressDialog *   progressDialog;

    /* Displacement widgets */
    QLabel          *   img1;
    QLabel          *   img2;
    QLabel          *   dispX;
    QLabel          *   dispY;
    QWidget         *   gridLayoutWidget;
    int                 gridIdx;
    QGridLayout     *   gridLayout;

    /* Anchor manual selection widgets*/
    QListWidget     *   anchorList;
    QListWidget     *   candidateAnchorList;
    QPushButton     *   addAnchor;
    QPushButton     *   removeAnchor;
    QLabel          *   anchor;
    QLabel          *   anchorCandidate;
    QWidget         *   anchorManuSelection;
    int                 anchorManuIdx;

    /* Anchor automatic selection widgets*/
    QList <AnchorLabel *>  referenceFrame;
    QWidget            *   anchorAutoSelection;
    int                    anchorAutoIdx;
    QPushButton        *   previousFrames;
    QPushButton        *   nextFrames;
    QPushButton        *   findAnchors;
    int                    showingFrames; 
    std::vector<int>       isAnchorFrames;
    int                    refFrameID;
    QDoubleSpinBox     *   thresholdSP;
};



#endif // WINDOW_H
