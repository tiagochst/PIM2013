#include <GL/glew.h>
#include "Window.h"

using namespace std;

/*!
 *  \brief  Creates the UI (upper menu, left and right dock and GLViewer)
 */
Window::Window () : QMainWindow (NULL) {

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

    /* Left Doc: execution buttons */
    QDockWidget * controlDockWidget = new QDockWidget (this);
    initControlWidget ();
    controlDockWidget->setWidget (controlWidget);
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
  
    controlWidget = new QGroupBox ();
    QVBoxLayout * layout = new QVBoxLayout (controlWidget);
    QGroupBox * previewGroupBox = new QGroupBox ("Preview", controlWidget);
    QVBoxLayout * previewLayout = new QVBoxLayout (previewGroupBox);

    createMeshPB  = new QPushButton ("Create Mesh", previewGroupBox);
    connect (createMeshPB, SIGNAL (clicked ()) , this, SLOT (createMesh()));
    previewLayout -> addWidget (createMeshPB);
    layout        -> addWidget (previewGroupBox);

}

/*!
 *  \brief  Create the vertex of a mesh from the image and deeper image 
 */
void Window::createMesh () {
    std::cout << "TBD ;D" << std::endl;
    
}

