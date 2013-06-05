#include <GL/glew.h>
#include "Window.h"
#include <sstream>
#include <QGraphicsScene>
#include <QImage>

using namespace std;

template<typename T>
std::string toString ( const T& val ) {
    std::stringstream ss;

    ss << val;

    return ss.str ();
}

void Window::startCapture() {
    ParameterHandler* params = ParameterHandler::Instance();
    params->SetCaptureMode ( true );
    startCaptureButton->setEnabled ( false );
    controlWidget->setEnabled ( false );
    progressDialog = new QProgressDialog (
        QString ("Processing..."),
        QString ("Cancel"),
        0,
        100 
    );
    FileWriterServices* fws = FileWriterServices::Instance ();
    connect (
                   fws, SIGNAL (       Progress ( int ) ),
        progressDialog, SLOT   (       setValue ( int ) )
    );
    connect (
                   fws, SIGNAL (            Finished () ),
                  this, SLOT   ( enableCaptureButton () )
    );
    progressDialog->show ();
}

void Window::enableCaptureButton () {
    startCaptureButton->setEnabled ( true );
    controlWidget->setEnabled ( true );
    if ( progressDialog ) {
        delete progressDialog;
        progressDialog = 0x0;
    }
}

void Window::setMesh(bool b){
  ParameterHandler* params = ParameterHandler::Instance();
  params -> SetMesh(b);

  if(b){
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

void Window::addAnchorListItems(){
  //TODO: abri arquivo ler quais sao anchor e comparar com lista de frames colocar na esquedar ou direita 

    static const std::string IMG_LIST_PATH(Config::FramesPath() + "list.txt");
    static const std::string ANCHOR_LIST_PATH(Config::FramesPath() + "anchorList.txt");
    
    QFile imageList(IMG_LIST_PATH.c_str());
    QFile anchorSavedList(ANCHOR_LIST_PATH.c_str());
    
    QString fileName;
    QString anchorID;
    
    /* Verify if the file with frames is readable*/
    if(!imageList.open(QIODevice::ReadOnly )) return;
    
    /* Verify if the file with anchor frames is readable*/
    if(!anchorSavedList.open(QIODevice::ReadOnly )){
      
      QTextStream in(& imageList);
      
      while(!in.atEnd())
	{
	  fileName = in.readLine();
	  if(fileName.endsWith(".pgm"))
	    {
            QStringList id = fileName.split("."); // Slipt in ID and pgm
            QString str = id.at(0);

	    /* Code used for sorting */
	    QString zero = "0";
	    if(str.size() == 1){ 
	      zero.append(str);
	      str = zero;
	    }
            /* Show the ID in the list box*/
	    candidateAnchorList->addItem(str);
	    }
	}
      
    }
    else {

      QTextStream in(& imageList);
      QTextStream anchor(& anchorSavedList);
      
      if(!anchor.atEnd())
	anchorID = anchor.readLine();
      
      while(!in.atEnd())
	{
	  fileName = in.readLine();
	  if(fileName.endsWith(".pgm"))
	    {
	      QStringList id = fileName.split("."); // Slipt in ID and pgm
	      QString str = id.at(0);
	      
	      /* Code used for sorting */
	      QString zero = "0";
	      if(str.size() == 1){ 
		zero.append(str);
		str = zero;
	      }
      
	      /* Show the ID in the combo box*/
	      if(!str.compare(anchorID)){ 
		anchorList->addItem(str);
		anchorID = anchor.readLine();
	      }
	      else{
		candidateAnchorList->addItem(str);
	      }
	    }
	}
      
      anchorSavedList.close();

    }

    /* Close the file */
    imageList.close();

    return;

}

void Window::saveAnchors(){
  //TODO: Create a file with the items in anchorList
    static const std::string ANCHOR_LIST_PATH(Config::FramesPath() + "anchorList.txt");

    QFile anchorFile(ANCHOR_LIST_PATH.c_str());
    QString fileName;
    
    /* Verify if the file readable*/
    if(!anchorFile.open(QIODevice::WriteOnly))
        return;

    for(int i= 0; i < anchorList -> count(); i++){
      fileName = anchorList -> item(i) -> text();
      fileName.append("\n");
      anchorFile.write(fileName.toUtf8());
    }

    anchorFile.close();
}

void Window::updateAnchorPreview(){

  std::string RES_IMG_PATH(Config::FramesPath());
  /* Get list of selected items, we will preview only the first one in the list*/
  QList<QListWidgetItem *>  candidates = candidateAnchorList -> selectedItems();
  QList<QListWidgetItem *>  anchors = anchorList -> selectedItems();

  QString id1,id2;
  if(candidates.count() > 0){
    id1 = (candidates[0] -> text());
  }
  else{
    id1 = "-1";
  }
  if(anchors.count() > 0){
    id2 = (anchors[0] -> text());
  }
  else{
    id2 = "-1";
  }

  /* Removes 0 from the left to align with write file names pattern */
  bool ok;
  int id = id1.toInt (&ok,10);
  std::string frameID1 = toString(id);
  id = id2.toInt   (&ok,10);
  std::string frameID2= toString(id);


  QPixmap anchorCandidateImg(QString::fromUtf8(((RES_IMG_PATH + "image_"+ frameID1 + ".pgm").c_str())));
  if(!anchorCandidateImg.isNull())
    anchorCandidate -> setPixmap(anchorCandidateImg.scaled(180, 150, Qt::IgnoreAspectRatio, Qt::FastTransformation));

  QPixmap anchorImg(QString::fromUtf8(((RES_IMG_PATH + "image_"+ frameID2 + ".pgm").c_str())));
  if(!anchorImg.isNull()){
    anchor -> setPixmap(anchorImg.scaled(180, 150, Qt::IgnoreAspectRatio, Qt::FastTransformation));
  }

}

void Window::initAnchorSelection(){

  if (candidateAnchorList) delete candidateAnchorList;
  candidateAnchorList = new QListWidget(anchorSelection);
  candidateAnchorList -> setGeometry(QRect(50, 220, 180, 150));

  if (anchorList) delete anchorList;
  anchorList = new QListWidget(anchorSelection);
  anchorList -> setGeometry(QRect(371, 220, 180, 150));

  addAnchorListItems();

  if (anchorCandidate) delete anchorCandidate;
  anchorCandidate = new QLabel(anchorSelection);
  anchorCandidate -> setGeometry(QRect(50, 50, 180, 150));
  anchorCandidate -> setMaximumSize(QSize(180, 150));

  if (anchor) delete anchor;
  anchor = new QLabel(anchorSelection);
  anchor -> setGeometry(QRect(371, 50, 180, 150));
  anchor -> setMaximumSize(QSize(180, 150));

  updateAnchorPreview ();

  connect(
	  candidateAnchorList, SIGNAL(itemSelectionChanged () ),
	                 this, SLOT  ( updateAnchorPreview () )
  );

  connect(
	  anchorList, SIGNAL(itemSelectionChanged () ),
                this, SLOT  ( updateAnchorPreview () )
  );

}

void Window::setAnchor(bool b){
  if(b){
    initAnchorSelection();
    centerWidget -> setCurrentIndex ( anchorIdx);
  }
  else{ 
    //save anchors listed in file 
    saveAnchors();
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
    }
    else{
        updateImages();
    }
}

/* Move Item from candidate list to anchor List*/
void Window::addNewAnchorItem(){

  /*Get selected row*/
  int row = candidateAnchorList -> currentRow();

  /*Remove row from list and add to another list */
  QListWidgetItem * selectedFrame = candidateAnchorList ->  takeItem(row);
  anchorList -> addItem(selectedFrame -> text());

  /* Sort list*/
  anchorList -> sortItems();
}

/* Move item from anchor list to candidate List*/
void Window::removeAnchorItem(){

  /*Get selected row*/
  int row = anchorList -> currentRow();

  /*Remove row from list and add to another list */
  QListWidgetItem * selectedFrame = anchorList ->  takeItem(row);

  candidateAnchorList -> addItem(selectedFrame -> text());

  /* Sort list*/
  candidateAnchorList -> sortItems();

}

void Window::calcDisp() {
    ParameterHandler* params = ParameterHandler::Instance();

    const unsigned int& wSize = params->GetWindowSize ();
    const unsigned int& nSize = params->GetNeighbourhoodSize ();

    std::string RES_IMG_PATH(Config::FramesPath());
    std::string frameID1 = toString(params -> GetFrame1());
    std::string frameID2 = toString(params -> GetFrame2());

    Image frame1(RES_IMG_PATH + "image_"+ frameID1 + ".pgm");
    Image frame2(RES_IMG_PATH + "image_"+ frameID2 + ".pgm");
    Image dispX( frame1.GetWidth(), frame1.GetHeight(), wSize );
    Image dispY( frame1.GetWidth(), frame1.GetHeight(), wSize );

    try {
        Image::TrackPixels (
            frame1,
            frame2,
            wSize,
            wSize,
            nSize,
            nSize,
            dispX,
            dispY
        );
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
    QPixmap pic3(QPixmap(QString::fromUtf8(((Config::OutputPath() + "TrackingF"+ frameID1 + "F"+ frameID2 + "x.pgm").c_str()))));
    if(!pic3.isNull())
      dispX -> setPixmap(pic3.scaled( 320, 240, Qt::IgnoreAspectRatio, Qt::FastTransformation));

    /* Reads Y displacement */
    if (dispY) delete dispY;
    dispY = new QLabel;
    dispY -> setMaximumSize(QSize(320, 240));
    QPixmap pic4(QPixmap(QString::fromUtf8(((Config::OutputPath() + "TrackingF"+ frameID1 + "F"+ frameID2 + "y.pgm").c_str()))));
    if(!pic4.isNull())
      dispY -> setPixmap(pic4.scaled( 320, 240, Qt::IgnoreAspectRatio, Qt::FastTransformation));
    
    /* Set image in a 2x2 grid*/
    if (gridLayout) delete gridLayout;
    gridLayout = new QGridLayout(gridLayoutWidget);
    gridLayout -> setContentsMargins(0, 0, 0, 0);
    gridLayout -> addWidget( img1, 0, 0, 1, 1);
    gridLayout -> addWidget( img2, 0, 1, 1, 1);
    gridLayout -> addWidget(dispX, 1, 0, 1, 1);
    gridLayout -> addWidget(dispY, 1, 1, 1, 1);
}

void Window::setFrame2(int iFrame) {
    ParameterHandler* params = ParameterHandler::Instance();
    params->SetFrame2(iFrame);
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
            QStringList id = fileName.split("."); // Slipt in ID and pgm
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
        centerWidget(NULL),
        controlWidget(NULL),
        createMeshPB(NULL),
        calcDispPB(NULL),
        snapshotButton(NULL),
	startCaptureButton(NULL),
        frame1ComboBox(NULL),
	frame2ComboBox(NULL),
        viewer(NULL),
        viewerIdx(0),
	cameraTimer(NULL),
	progressDialog(NULL),
        img1(NULL),
        img2(NULL),
        dispX(NULL),
        dispY(NULL),
        gridLayoutWidget(NULL),
        gridIdx(0),
        gridLayout(NULL),
        anchorList(NULL),
        candidateAnchorList(NULL),
        addAnchor(NULL),
        removeAnchor(NULL),
        anchor(NULL),
        anchorCandidate(NULL),
        anchorSelection(NULL),
        anchorIdx(0)
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
    gridLayoutWidget = new QWidget (this);
    anchorSelection  = new QWidget (this);

    centerWidget = new QStackedWidget();
    setCentralWidget (centerWidget);

    viewerIdx = centerWidget -> addWidget (           viewer );
    gridIdx   = centerWidget -> addWidget ( gridLayoutWidget );
    anchorIdx = centerWidget -> addWidget (   anchorSelection );

    /* Adding settings to upper menu */
    /* Adding quit and about buttons to upper menu */
    QMenu *fileMenu = menuBar()->addMenu(tr("&Help"));

    QAction *glViewerHelp = new QAction(tr("&GLViewer Help"), this);
    fileMenu -> addAction(glViewerHelp);

    connect(
            glViewerHelp, SIGNAL( triggered() ),
                  viewer, SLOT  (      help() )
    );

    QAction *openAct = new QAction(tr("&About..."), this);
    fileMenu -> addAction(openAct);

    connect(
               openAct, SIGNAL( triggered() ),
               this   , SLOT  (     about() )
    );

    fileMenu -> addSeparator();

    QAction *exitAct = new QAction(tr("E&xit"), this);
    fileMenu -> addAction(exitAct);

    connect(
            exitAct, SIGNAL(      triggered() ),
               this, SLOT  ( exitPreprocess() )
    );

    connect(
              this, SIGNAL(         exiting() ),
              qApp, SLOT  ( closeAllWindows() )
    );

    createDock();
    statusBar()->showMessage("");

}

void Window::exitPreprocess () {
    FileWriterServices::DeleteInstance ();
    emit ( exiting () );
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

    QGroupBox *  previewGroupBox = new QGroupBox   ("Preview", controlWidget);
    QVBoxLayout *   previewLayout = new QVBoxLayout (previewGroupBox);

    /* Creating tables for frame selection */
    frame1ComboBox = new QComboBox (previewGroupBox);
    frame2ComboBox = new QComboBox (previewGroupBox);
    addImageItems();

    QLabel          *   frame1Label = new QLabel(tr("Frame:"));
    frame1Label -> setBuddy(frame1ComboBox);

    QLabel          *   frame2Label = new QLabel(tr("Frame 2:"));
    frame2Label -> setBuddy(frame2ComboBox);

    QWidget *generalLayoutWidget   = new QWidget(previewGroupBox);
    QFormLayout *generalFormLayout = new QFormLayout(generalLayoutWidget);
    generalFormLayout -> setContentsMargins(0, 0, 0, 0);
    generalFormLayout -> setWidget(0, QFormLayout::LabelRole, frame1Label);
    generalFormLayout -> setWidget(0, QFormLayout::FieldRole, frame1ComboBox);
    generalFormLayout -> setWidget(1, QFormLayout::LabelRole, frame2Label);
    generalFormLayout -> setWidget(1, QFormLayout::FieldRole, frame2ComboBox);

    /* Creation of left buttons*/
    createMeshPB       = new QPushButton ("Create Mesh", previewGroupBox);
    calcDispPB         = new QPushButton ("Calc Displacement", previewGroupBox);
    snapshotButton     = new QPushButton ("Save preview", previewGroupBox);
    startCaptureButton = new QPushButton ("Start Capture", previewGroupBox);

    QButtonGroup * modeButtonGroup = new QButtonGroup (previewGroupBox);
    modeButtonGroup->setExclusive (true);
    displacementRB =  new QRadioButton("Displacement", previewGroupBox);
    meshRB   = new QRadioButton("Mesh", previewGroupBox);
    anchorRB = new QRadioButton("Anchor", previewGroupBox);
    modeButtonGroup->addButton (displacementRB);
    modeButtonGroup->addButton (meshRB);
    modeButtonGroup->addButton (anchorRB);

    /* Anchor buttons*/
    addAnchor    = new QPushButton (">",anchorSelection);
    removeAnchor = new QPushButton ("<",anchorSelection);
    addAnchor    -> setGeometry(QRect(280, 260, 41, 31));
    removeAnchor -> setGeometry(QRect(280, 300, 41, 31));

    /********** Connections ***********/

    /*** Situation: Mesh showing *****/
    /* Description: TODO */
    connect (
             createMeshPB, SIGNAL (       clicked () ),
                     this, SLOT   (    createMesh () )
    );

    /* Description: Print screen */
    connect (
            snapshotButton, SIGNAL (      clicked () ),
                      this, SLOT   (  saveGLImage () )
    );

    /* Description: Capture Frames */
    connect (
        startCaptureButton, SIGNAL (      clicked () ),
                      this, SLOT   ( startCapture () )
    );

    /* Description: Disabling image 2 selection */
    connect(
                    meshRB, SIGNAL (     toggled (bool) ), 
            frame2ComboBox, SLOT   ( setDisabled (bool) )
    );

    /* Description: Disabling calculate displacement push button */
    connect(
                    meshRB, SIGNAL (       toggled (bool) ), 
                calcDispPB, SLOT   (   setDisabled (bool) )
    );

    /* Description: Change of situation 
                    Mesh selected -> update screen */
    connect(
                    meshRB, SIGNAL( toggled (bool) ), 
		      this, SLOT  ( setMesh (bool) )
    );

    /*** Situation: Displacement showing *****/
    /* Description: Calculate displacement */
    connect (
               calcDispPB, SIGNAL (      clicked  () ), 
                     this, SLOT   (      calcDisp () )
    );

    /* Description: Disabling Mesh snapshot */
    connect(
            displacementRB, SIGNAL (      toggled (bool) ),
            snapshotButton, SLOT   (  setDisabled (bool) )
    );

  
    /* Description: Disabling capture button */
    connect(
                displacementRB, SIGNAL (     toggled (bool) ),
            startCaptureButton, SLOT   ( setDisabled (bool) )
    );

    /* Description: Disabling create mesh */
    connect(
               displacementRB, SIGNAL (     toggled (bool) ),
                 createMeshPB, SLOT   ( setDisabled (bool) )
    );

    /* Description: Changing Frame 1 */
    connect (
               frame1ComboBox, SIGNAL ( currentIndexChanged (int) ), 
                         this, SLOT   (           setFrame1 (int) )
    );

    /* Description: Changing Frame 2 */
    connect (
               frame2ComboBox, SIGNAL ( currentIndexChanged (int) ), 
                         this, SLOT   (           setFrame2 (int) )
    );

    /* Description: Change of situation 
                    Displacement selected -> update screen */
    connect(
            displacementRB, SIGNAL (         toggled (bool) ),
                      this, SLOT   ( setDisplacement (bool) )
    );


    /*** Situation: Anchor selection *****/

    /* Description: Calculate displacement */
    connect (
                addAnchor, SIGNAL (      clicked  () ), 
                     this, SLOT   (      addNewAnchorItem () )
    );

    /* Description: Calculate displacement */
    connect (
             removeAnchor, SIGNAL (      clicked  () ), 
                     this, SLOT   (      removeAnchorItem () )
    );

    /* Description: Change of situation 
                    Displacement selected -> update screen */
    connect(
	           anchorRB, SIGNAL (         toggled (bool) ),
                       this, SLOT   (       setAnchor (bool) )
    );


    /*** Initial situation: Default options ***/
    meshRB         -> setChecked(true);
    calcDispPB     -> setDisabled(true);
    frame2ComboBox -> setDisabled(true);


    /* Verify if a camera is connect */
    ParameterHandler* params = ParameterHandler::Instance();
    if(params -> GetCamera()){

      cameraTimer = new QTimer();
      Camera* cam = &(Camera::Instance());

      connect (
	       cameraTimer, SIGNAL (          timeout () ),
	               cam, SLOT   ( WaitUpdateCamera () )
	       );
      connect (
	       cameraTimer, SIGNAL (          timeout () ),
	       viewer     , SLOT   (           update () )
	       );
      cameraTimer->start(16);
    }
    else {
      /* No device found: disable glviewer */
      meshRB -> setChecked(false);
      meshRB -> setDisabled(true);
      displacementRB -> setChecked(true);
      anchorRB -> setChecked(false);
      setDisplacement(true);
    }

    FileWriterServices* fws = FileWriterServices::Instance ();

    /* Add widgets to layout*/
    previewLayout->addWidget (generalLayoutWidget);
    previewLayout->addWidget (displacementRB);
    previewLayout->addWidget (meshRB);
    previewLayout->addWidget (anchorRB);
    previewLayout->addWidget (createMeshPB);
    previewLayout->addWidget (calcDispPB);
    previewLayout->addWidget (snapshotButton);
    previewLayout->addWidget (startCaptureButton);

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

