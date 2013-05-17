#include <GL/glew.h>
#include "Window.h"

using namespace std;


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
            frameComboBox -> addItem(id.at(0),QVariant::Char); // Show the ID
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
    static const std::string IMAGE_LIST(" ls " + Config::OutputPath() + "CapturedFrames/ >" + Config::OutputPath() + "CapturedFrames/list.txt");
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


    /* Defining size policy of the windows */
    QSizePolicy sizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);

    /* Left Doc: exection buttons */
    QDockWidget * controlDockWidget = new QDockWidget (this);
    initControlWidget ();
    controlDockWidget->setWidget (controlWidget);
    sizePolicy.setHeightForWidth(controlDockWidget->sizePolicy().hasHeightForWidth());
    controlDockWidget->setSizePolicy(sizePolicy);
    addDockWidget (Qt::LeftDockWidgetArea, controlDockWidget);
    controlDockWidget->setFeatures (QDockWidget::AllDockWidgetFeatures);
    statusBar()->showMessage("");

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
    

    /* Creating tables for general parameters */
    frameComboBox = new QComboBox (previewGroupBox);
    addImageItems();

    QLabel      * frameLabel;
    frameLabel = new QLabel(tr("Frame:"));
    frameLabel -> setBuddy(frameComboBox);

    QWidget *generalLayoutWidget = new QWidget(previewGroupBox);
    QFormLayout *generalFormLayout = new QFormLayout(generalLayoutWidget);
    generalFormLayout -> setContentsMargins(0, 0, 0, 0);
    generalFormLayout -> setWidget(0, QFormLayout::LabelRole, frameLabel);
    generalFormLayout -> setWidget(0, QFormLayout::FieldRole, frameComboBox);

    createMeshPB  = new QPushButton ("Create Mesh", previewGroupBox);
    connect (createMeshPB, SIGNAL (clicked ()) , this, SLOT (createMesh()));

    QRadioButton * displacementRB =  new QRadioButton("Displacement", previewGroupBox);
    QRadioButton * meshRB = new QRadioButton("Mesh", previewGroupBox);
    meshRB -> setChecked(true);

    snapshotButton  = new QPushButton ("Save preview", previewGroupBox);
    connect (snapshotButton, SIGNAL (clicked ()) , this, SLOT (saveGLImage ()));
    
    /* Add widgets to layout*/
    previewLayout -> addWidget (generalLayoutWidget);
    previewLayout -> addWidget (displacementRB);
    previewLayout -> addWidget (meshRB);
    previewLayout -> addWidget (createMeshPB);
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

