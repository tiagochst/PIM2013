#include <QCoreApplication>
#include <QApplication>
#include "Window.h"
#include <QDir>
#include <QPixmap>
#include <QSplashScreen>
#include <QPlastiqueStyle>
#include <QCleanlooksStyle>
#include <string>
#include <iostream>
#include "MathUtils.h"
#include "Image.h"
#include "ImageBase.h"
#include "SubImage.h"
#include "PointSet.h"
#include "Camera.h"
#include "Config.h"
#include "Rectangle.h"
#include "ParameterHandler.h"
#include "PixelTracker.h"

class QMyApplication
    :   public QApplication
{
public:
    QMyApplication(int argc, char**argv) : QApplication(argc,argv) {}
    bool notify(QObject* receiver, QEvent* event) {
        //    try {
        return QApplication::notify(receiver,event);
        // } catch (...) {
        //std::cerr << "Unhandled exception caught" <<std::endl;
        //}
        //return false;
    }
};

/* Class Camera test: Kinect's  */
xn::Context        g_context;
xn::ScriptNode  g_scriptNode;
xn::EnumerationErrors errors;
int KinectSetup(int argc, char**argv)
{
    XnStatus                  rc;
    
    /* Create a context with default settings */
    rc = g_context.InitFromXmlFile (
        ( Config::ConfigPath() + SAMPLE_XML_PATH ).c_str(),
        g_scriptNode,
        &errors
    );

    /* Verify if device is connected */
    if (rc == XN_STATUS_NO_NODE_PRESENT)
    {
        XnChar strError[1024];
        errors.ToString(strError, 1024);
        printf("%s\n", strError);
        return (rc);
    }
    else 
    {
        CHECK_RC(rc,"Open");
    }
    
    Camera& viewer = Camera::CreateInstance(g_context);

    return rc;
}

int KinectInit(int argc, char** argv)
{
    XnStatus rc = KinectSetup ( argc, argv );
    CHECK_RC(rc,"Kinect Setup");

    Camera& viewer = Camera::Instance();

    rc = viewer.Init(argc, argv);
    CHECK_RC(rc,"Viewer Init");

    rc = viewer.Run();
    CHECK_RC(rc,"Viewer run");

    return 1;
}

int main(int argc, char** argv) {
    Config::LoadConfigs(Config::RootPath() + "settings");

    static const std::string RES_PTSET_PATH(Config::ResourcesPath() + "PointSets/");
    static const std::string RES_IMG_PATH(Config::ResourcesPath() + "Images/");

    XnStatus rc = KinectSetup ( argc, argv );
    //CHECK_RC(rc, "Error setting up Kinect Camera");

    if (rc == XN_STATUS_OK){
        ParameterHandler* params = ParameterHandler::Instance();
        params -> SetCamera(true);
        Camera& cam = Camera::Instance ();
        cam.Setup(argc, argv);
    }

    QMyApplication program (argc, argv);

    QMyApplication::setStyle ( new QPlastiqueStyle() );
    program.setAttribute(Qt::AA_DontUseNativeMenuBar,true);
    Window * progWindow = new Window ();
    progWindow->setWindowTitle ("PIM380: A facial reconstruction program.");
    progWindow -> show();
    program.connect (&program, SIGNAL (lastWindowClosed()), &program, SLOT (quit()));
    
    return program.exec ();
}

