#include <GL/glew.h>
#include "Window.h"
 #include <QGraphicsScene>
using namespace std;

void Window::setMesh(bool b){
  ParameterHandler* params = ParameterHandler::Instance();
  params -> SetMesh(b);
  
  try {
    viewer = new GLViewer;
  } catch (GLViewer::Exception e) {
    cerr << e.getMessage () << endl;
    exit (1);
  }
  
  removeDockWidget(controlDockWidget); 
  setCentralWidget (viewer);
  controlDockWidget -> setMaximumWidth(150 );
  restoreDockWidget ( controlDockWidget );
}

void Window::setDisplacement(bool b){
  ParameterHandler* params = ParameterHandler::Instance();
  updateImages();
  setCentralWidget (gridLayoutWidget);
}

/*!
 *  \brief  Set the new scene selected from the box
 *  \param  scene Number of the scene
 */
void Window::setFrame1(int iFrame) {
    ParameterHandler* params = ParameterHandler::Instance();
    params -> SetFrame1(iFrame);
 
   if(params -> GetMesh()){
      viewer -> reset();
      setCentralWidget (viewer);
    }
    else{
      updateImages();
    }
}

void Window::calcDisp() {
    ParameterHandler* params = ParameterHandler::Instance();
    std::string RES_IMG_PATH(Config::OutputPath() + "CapturedFrames/");
    std::string frameID1 = std::to_string(params -> GetFrame1());
    std::string frameID2 = std::to_string(params -> GetFrame2());
 
    Image frame1(RES_IMG_PATH + "image_"+ frameID1 + ".pgm");
    Image frame2(RES_IMG_PATH + "image_"+ frameID2 + ".pgm");
    Image dispX( frame1.GetWidth(), frame1.GetHeight(), 255 );
    Image dispY( frame1.GetWidth(), frame1.GetHeight(), 255 );
    
    try {
      Image::TrackPixels(frame1, frame2, 17, 17, 9, 9, dispX, dispY );
      dispX.CreateAsciiPgm(Config::OutputPath() + "TrackinF"+ frameID1 + "F"+ frameID2+"x.pgm");
      dispY.CreateAsciiPgm(Config::OutputPath() + "TrackinF"+ frameID1 + "F"+ frameID2+"y.pgm");
    } catch (BadIndex bi) {
      std::cout << bi.what();
    }
  
}

void Window::updateImages() {
    ParameterHandler* params = ParameterHandler::Instance();
    std::string RES_IMG_PATH(Config::OutputPath() + "CapturedFrames/");
    std::string frameID1 = std::to_string(params -> GetFrame1());
    std::string frameID2 = std::to_string(params -> GetFrame2());

    img1 = new QLabel;
    img1 -> setMaximumSize(QSize(320, 240));
    img1 -> setPixmap(QPixmap(QString::fromUtf8(((RES_IMG_PATH + "image_"+ frameID1 + ".pgm").c_str()))));

    img2 = new QLabel;
    img2 -> setMaximumSize(QSize(320, 240));
    img2 -> setPixmap(QPixmap(QString::fromUtf8(((RES_IMG_PATH + "image_"+ frameID2 + ".pgm").c_str()))));

    dispX = new QLabel;
    dispX -> setMaximumSize(QSize(320, 240));
    dispX -> setPixmap(QPixmap(QString::fromUtf8(((Config::OutputPath() + "TrackinF"+ frameID1 + "F"+ frameID2 + "x.pgm").c_str()))));

    dispY = new QLabel;
    dispY -> setMaximumSize(QSize(320, 240));
    dispY -> setPixmap(QPixmap(QString::fromUtf8(((Config::OutputPath() + "TrackinF"+ frameID1 + "F"+ frameID2 + "y.pgm").c_str()))));

    gridLayoutWidget = new QWidget;
    gridLayout = new QGridLayout(gridLayoutWidget);
    gridLayout->setContentsMargins(0, 0, 0, 0);
    gridLayout->addWidget(img1, 0, 0, 1, 1);
    gridLayout->addWidget(img2, 0, 1, 1, 1);
    gridLayout->addWidget(dispX, 1, 0, 1, 1);
    gridLayout->addWidget(dispY, 1, 1, 1, 1);
    setCentralWidget(gridLayoutWidget);    
}

void Window::setFrame2(int iFrame) {
    ParameterHandler* params = ParameterHandler::Instance();
    params -> SetFrame2(iFrame);
    updateImages();
}

/*!
 *  \brief Save image of the GLViewer
 */
void Window::saveGLImage () {
    viewer->saveSnapshot (false, false);
}

/*!
 *  \brief Get all the frames id from the directory and add to comboBox 
 */
void Window::addImageItems()
{
    static const std::string IMG_LIST_PATH(Config::OutputPath() + "CapturedFrames/list.txt");

    QFile imageList(IMG_LIST_PATH.c_str());
    QString fileName;
    
    /* Verify if the file readable*/
    if(!imageList.open(QIODevice::ReadOnly ))
        return;

    /* Reads the file */
    QTextStream in(& imageList);

    while(!in.atEnd())
    {
        fileName = in.readLine();
        if(fileName.contains("image_") && fileName.endsWith(".pgm"))
        {
            QStringList list = fileName.split("_"); // Slipt in image_ and ID.pgm
            QStringList id = list.at(1).split("."); // Slipt in id and pgm
	    /* Show the ID in the combo box*/
            frame1ComboBox -> addItem(id.at(0),QVariant::Char); 
            frame2ComboBox -> addItem(id.at(0),QVariant::Char); 
        }
    }

    /* Close the file */
    imageList.close();
    return;
    
}

/*!
 *  \brief  Creates the UI (upper menu, left and right dock and GLViewer)
 */
Window::Window () : QMainWindow (NULL) {

    /* creates the list of images in the system*/
    static const std::string IMAGE_LIST(" ls -B --ignore=list.txt " + Config::OutputPath() + "CapturedFrames/ >" + Config::OutputPath() + "CapturedFrames/list.txt");

    system(IMAGE_LIST.c_str());
   
    try {
        viewer = new GLViewer;
    } catch (GLViewer::Exception e) {
        cerr << e.getMessage () << endl;
        exit (1);
    }
    setCentralWidget (viewer);

    /* Adding settings to upper menu */
    /* Adding quit and about buttons to upper menu */
    QMenu *fileMenu = menuBar()->addMenu(tr("&Help"));

    QAction *glViewerHelp = new QAction(tr("&GLViewer Help"), this);
    fileMenu->addAction(glViewerHelp);
    connect(glViewerHelp, SIGNAL(triggered()),
            viewer, SLOT(help()));

    QAction *openAct = new QAction(tr("&About..."), this);
    fileMenu->addAction(openAct);
    connect(openAct, SIGNAL(triggered()),
            this, SLOT(about()));

    fileMenu->addSeparator();

    QAction *exitAct = new QAction(tr("E&xit"), this);
    fileMenu->addAction(exitAct);
    connect(exitAct, SIGNAL(triggered()),
            qApp, SLOT(closeAllWindows()));

    createDock();
    statusBar()->showMessage("");

}

void Window::createDock () {

    /* Defining size policy of the windows */
    QSizePolicy sizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);

    /* Left Doc: exection buttons */
    controlDockWidget = new QDockWidget (this);
    initControlWidget ();
    controlDockWidget->setWidget (controlWidget);
    sizePolicy.setHeightForWidth(controlDockWidget->sizePolicy().hasHeightForWidth());
    controlDockWidget->setSizePolicy(sizePolicy);
    addDockWidget (Qt::LeftDockWidgetArea, controlDockWidget);
    controlDockWidget->setFeatures (QDockWidget::AllDockWidgetFeatures);

}
Window::~Window () {

}

/*!
 *  \brief  GLviewer help
 */
void Window::GLViewerHelp () {
}


/*!
 *  \brief  Show a content about this program 
 */
void Window::about () {
    QMessageBox::about (this, 
                        "About This Program", 
                        "<b>PIM380</b>"
                        "<br> by <i>Vinicius Dias Gardelli</i> "
                        "<br> and <i>Tiago Chedraoui Silva</i>.");
}


void Window::initControlWidget () {

    /* Defining size policy of the windows */
    QSizePolicy sizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);

    controlWidget = new QGroupBox ();
    QVBoxLayout * layout = new QVBoxLayout (controlWidget);

    QGroupBox * previewGroupBox = new QGroupBox ("Preview", controlWidget);
    QVBoxLayout * previewLayout = new QVBoxLayout (previewGroupBox);
    
    /* Creating tables for frame selection */
    frame1ComboBox = new QComboBox (previewGroupBox);
    frame2ComboBox = new QComboBox (previewGroupBox);
    addImageItems();
    connect (frame1ComboBox, SIGNAL (currentIndexChanged(int)), this, SLOT (setFrame1 (int)));
    connect (frame2ComboBox, SIGNAL (currentIndexChanged(int)), this, SLOT (setFrame2 (int)));

    QLabel      * frame1Label;
    frame1Label = new QLabel(tr("Frame:"));
    frame1Label -> setBuddy(frame1ComboBox);

    QLabel      * frame2Label;
    frame2Label = new QLabel(tr("Frame 2:"));
    frame2Label -> setBuddy(frame2ComboBox);

    QWidget *generalLayoutWidget = new QWidget(previewGroupBox);
    QFormLayout *generalFormLayout = new QFormLayout(generalLayoutWidget);
    generalFormLayout -> setContentsMargins(0, 0, 0, 0);
    generalFormLayout -> setWidget(0, QFormLayout::LabelRole, frame1Label);
    generalFormLayout -> setWidget(0, QFormLayout::FieldRole, frame1ComboBox);
    generalFormLayout -> setWidget(1, QFormLayout::LabelRole, frame2Label);
    generalFormLayout -> setWidget(1, QFormLayout::FieldRole, frame2ComboBox);
    /* End of  frame selection */

    createMeshPB  = new QPushButton ("Create Mesh", previewGroupBox);
    connect (createMeshPB, SIGNAL (clicked ()) , this, SLOT (createMesh()));

    calcDispPB  = new QPushButton ("Calc Displacement", previewGroupBox);
    connect (calcDispPB, SIGNAL (clicked ()) , this, SLOT (calcDisp()));

    QRadioButton * displacementRB =  new QRadioButton("Displacement", previewGroupBox);
    QRadioButton * meshRB = new QRadioButton("Mesh", previewGroupBox);
    meshRB -> setChecked(true);
    frame2ComboBox -> setDisabled(true);

    snapshotButton  = new QPushButton ("Save preview", previewGroupBox);
    connect (snapshotButton, SIGNAL (clicked ()) , this, SLOT (saveGLImage ()));
    
    /* Mesh showing: Disabling image 2 selection */
    connect(meshRB, SIGNAL(toggled(bool)), frame2ComboBox, SLOT(setDisabled(bool)));

    ParameterHandler* params = ParameterHandler::Instance();
    connect(meshRB, SIGNAL(toggled(bool)), this, SLOT(setMesh(bool)));
    connect(displacementRB, SIGNAL(toggled(bool)), this, SLOT(setDisplacement(bool)));

    /* Add widgets to layout*/
    previewLayout -> addWidget (generalLayoutWidget);
    previewLayout -> addWidget (displacementRB);
    previewLayout -> addWidget (meshRB);
    previewLayout -> addWidget (createMeshPB);
    previewLayout -> addWidget (calcDispPB);
    previewLayout -> addWidget (snapshotButton);

    /* Add widgets to left dock layout*/
    layout -> addWidget (previewGroupBox);
    layout -> addStretch (0);
}

/*!
 *  \brief  Create the vertex of a mesh from the image and deeper image 
 */
void Window::createMesh () {
    std::cout << "TBD ;D" << std::endl;
}

