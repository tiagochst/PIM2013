#include <GL/glew.h>
#include "Window.h"
#include <sstream>
#include <QGraphicsScene>
using namespace std;

template<typename T>
std::string toString ( const T& val ) {
    std::stringstream ss;

    ss << val;

    return ss.str ();
}

void Window::setMesh(bool b){
  ParameterHandler* params = ParameterHandler::Instance();
  params -> SetMesh(b);

  if(b){
    std::cout << "setmesh" << std::endl;
    viewer -> reset();
    centerWidget->setCurrentIndex ( viewerIdx );
  }
}

void Window::setDisplacement(bool b){
  if(b){
    updateImages();
    centerWidget->setCurrentIndex ( gridIdx );
  }
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
	viewer -> updateGL();
        centerWidget->setCurrentIndex (viewerIdx);
    }
    else{
        updateImages();
    }
}

void Window::calcDisp() {
    ParameterHandler* params = ParameterHandler::Instance();
    std::string RES_IMG_PATH(Config::FramesPath());
    std::string frameID1 = toString(params -> GetFrame1());
    std::string frameID2 = toString(params -> GetFrame2());

    Image frame1(RES_IMG_PATH + "image_"+ frameID1 + ".pgm");
    Image frame2(RES_IMG_PATH + "image_"+ frameID2 + ".pgm");
    Image dispX( frame1.GetWidth(), frame1.GetHeight(), 255 );
    Image dispY( frame1.GetWidth(), frame1.GetHeight(), 255 );

    try {
        Image::TrackPixels(frame1, frame2, 17, 17, 9, 9, dispX, dispY );
        dispX.CreateAsciiPgm(Config::OutputPath() + "TrackingF"+ frameID1 + "F"+ frameID2+"x.pgm");
        dispY.CreateAsciiPgm(Config::OutputPath() + "TrackingF"+ frameID1 + "F"+ frameID2+"y.pgm");
    } catch (BadIndex bi) {
        std::cout << bi.what();
    }
    updateImages ();
}

void Window::updateImages() {
    ParameterHandler* params = ParameterHandler::Instance();
    std::string RES_IMG_PATH(Config::FramesPath());
    std::string frameID1 = toString(params -> GetFrame1());
    std::string frameID2 = toString(params -> GetFrame2());

   /* Reads image 1 */
    if (img1) delete img1;
    img1 = new QLabel;
    img1 -> setMaximumSize(QSize(320, 240));
    QPixmap pic1(QString::fromUtf8(((RES_IMG_PATH + "image_"+ frameID1 + ".pgm").c_str())));
    if(!pic1.isNull())
      img1 -> setPixmap(pic1.scaled(320, 240, Qt::IgnoreAspectRatio, Qt::FastTransformation));

    /* Reads image 2 */
    if (img2) delete img2;
    img2 = new QLabel;
    img2 -> setMaximumSize(QSize(320, 240));
    QPixmap pic2(QString::fromUtf8(((RES_IMG_PATH + "image_"+ frameID2 + ".pgm").c_str())));
    if(!pic2.isNull())
      img2 -> setPixmap(pic2.scaled(320, 240, Qt::IgnoreAspectRatio, Qt::FastTransformation));

    /* Reads X displacement */
    if (dispX) delete dispX;
    dispX = new QLabel;
    dispX -> setMaximumSize(QSize(320, 240));
    QPixmap pic3(QPixmap(QString::fromUtf8(((Config::OutputPath() + "TrackinF"+ frameID1 + "F"+ frameID2 + "x.pgm").c_str()))));
    if(!pic3.isNull())
      dispX -> setPixmap(pic3.scaled( 320, 240, Qt::IgnoreAspectRatio, Qt::FastTransformation));

    /* Reads Y displacement */
    if (dispY) delete dispY;
    dispY = new QLabel;
    dispY -> setMaximumSize(QSize(320, 240));
    QPixmap pic4(QPixmap(QString::fromUtf8(((Config::OutputPath() + "TrackinF"+ frameID1 + "F"+ frameID2 + "y.pgm").c_str()))));
    if(!pic4.isNull())
      dispY -> setPixmap(pic4.scaled( 320, 240, Qt::IgnoreAspectRatio, Qt::FastTransformation));
    
    /* Set image in a 2x2 grid*/
    if (gridLayout) delete gridLayout;
    gridLayout = new QGridLayout(gridLayoutWidget);
    gridLayout->setContentsMargins(0, 0, 0, 0);
    gridLayout->addWidget(img1, 0, 0, 1, 1);
    gridLayout->addWidget(img2, 0, 1, 1, 1);
    gridLayout->addWidget(dispX, 1, 0, 1, 1);
    gridLayout->addWidget(dispY, 1, 1, 1, 1);
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
    static const std::string IMG_LIST_PATH(Config::FramesPath() + "list.txt");

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
        if(fileName.endsWith(".pgm"))
        {
            QStringList id = fileName.split("."); // Slipt in image_ and ID.pgm
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
Window::Window ()
    :   QMainWindow (NULL),
        controlWidget(NULL),
        viewer(NULL),
        viewerIdx(0),
        createMeshPB(NULL),
        snapshotButton(NULL),
        frame1ComboBox(NULL),
        frame2ComboBox(NULL),
        img1(NULL),
        img2(NULL),
        dispX(NULL),
        dispY(NULL),
        gridLayoutWidget(NULL),
        gridIdx(0),
        gridLayout(NULL),
        centerWidget(NULL)
{



    /* creates the list of images in the system*/
  static const std::string IMAGE_LIST(" ls -B --ignore=list.txt --ignore=depth* --ignore=*.ply " + Config::FramesPath() + "| sed -r 's/^.{6}//' |sort -g >" + Config::FramesPath() + "list.txt");

    system(IMAGE_LIST.c_str());
   
    try {
        viewer = new GLViewer;
    } catch (GLViewer::Exception e) {
        cerr << e.getMessage () << endl;
        exit (1);
    }
    //    connect(this, SIGNAL(frameChanged()), viewer,SLOT(update()));
    gridLayoutWidget = new QWidget ();

    centerWidget = new QStackedWidget();
    setCentralWidget (centerWidget);

    viewerIdx = centerWidget->addWidget ( viewer );
    gridIdx = centerWidget->addWidget ( gridLayoutWidget );

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

    /*Default option*/
    meshRB -> setChecked(true);
    calcDispPB -> setDisabled(true);
    frame2ComboBox -> setDisabled(true);

    snapshotButton  = new QPushButton ("Save preview", previewGroupBox);
    connect (snapshotButton, SIGNAL (clicked ()) , this, SLOT (saveGLImage ()));
    
    /* Mesh showing: Disabling image 2 selection */
    connect(meshRB, SIGNAL(toggled(bool)), frame2ComboBox, SLOT(setDisabled(bool)));
    connect(displacementRB, SIGNAL(toggled(bool)), snapshotButton, SLOT(setDisabled(bool)));
    connect(meshRB, SIGNAL(toggled(bool)), calcDispPB, SLOT(setDisabled(bool)));

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

