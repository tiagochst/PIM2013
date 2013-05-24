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
#include "Image.h"
#include "PointSet.h"
#include "Camera.h"
#include "Config.h"
//#include "ui_mainInterface.Qt4.h"

class QMyApplication : public QApplication {
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

int KinectInit(int argc, char** argv)
{

    /* Class Camera test: Kinect's  */
    xn::Context        g_context;
    xn::ScriptNode  g_scriptNode;
    xn::EnumerationErrors errors;
    XnStatus                  rc;
    
    /* Create a context with default settings */
    rc = g_context.InitFromXmlFile( ( Config::ConfigPath() + SAMPLE_XML_PATH ).c_str(), g_scriptNode, &errors);
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
    
    rc = viewer.Init(argc, argv);
    CHECK_RC(rc,"Viewer Init");
    
    rc = viewer.Run();
    CHECK_RC(rc,"Viewer run");

    return 1;
    
}

void FindTemplateAndPrintMap(
    const Image& iBaseImage,
    const Image& iTemplate,
    Image& oCorrelationMap,
    CartesianCoordinate& oBestMatch,
    float& oBestMatchVal,
    const std::string& iMapFilename,
    const Rectangle* iSearchWindow=NULL
) {
    if (iSearchWindow != NULL ) {
        oBestMatchVal = iBaseImage.TemplateMatch( iTemplate, *iSearchWindow, oBestMatch, &oCorrelationMap );
    } else {
        oBestMatchVal = iBaseImage.TemplateMatch( iTemplate, oBestMatch, &oCorrelationMap );
    }
    std::cout   << "Possible match found at ("  << oBestMatch.x << ", " << oBestMatch.y << ") "
                << "with correlation value of "   << oBestMatchVal
                << std::endl;
    oCorrelationMap.CreateAsciiPgm(Config::OutputPath() + iMapFilename);
}

int main(int argc, char** argv) {
    Config::LoadConfigs(Config::RootPath() + "settings");

    static const std::string RES_PTSET_PATH(Config::ResourcesPath() + "PointSets/");
    static const std::string RES_IMG_PATH(Config::ResourcesPath() + "Images/");

    /* Class image test */
    // Image myImage(RES_IMG_PATH + "frame_20121108T103323.258153_rgb-ascci.pgm");

    /* Class Ply test  */
    //PointSet psAscii, psBinary;    
    //psAscii.LoadFromFile(RES_PTSET_PATH + "frame000-ascii.ply");
    //psBinary.LoadFromFile(RES_PTSET_PATH + "frame000-brut.ply");
 
    Image frame0(RES_IMG_PATH + "frame_20121108T103323.258153_rgb-brut.pgm");
    Image frame1(RES_IMG_PATH + "frame_20121108T103323.390878_rgb-brut.pgm");
    Image dispX( frame0.GetWidth(), frame0.GetHeight(), 255 );
    Image dispY( frame0.GetWidth(), frame0.GetHeight(), 255 );

    Image smallMask(Config::DataPath() + "smallMask.pgm");
    Image mask(Config::DataPath() + "mask.pgm");
    Image bigMask(Config::DataPath() + "bigMask.pgm");
    
    Image correlationMap( 1, 1, 255 );
    float correlationVal = 0.0f;
    CartesianCoordinate bestMatch;

    char c;
    puts ("Select Mode: Kinect ('k') Image Traking('t') UI (u) Other:('o')");
    c = getchar();

    if ( c == 't' ) {
   
        FindTemplateAndPrintMap(
            bigMask,
            smallMask,
            correlationMap,
            bestMatch,
            correlationVal,
            "smallMaskCorrelation.pgm"
            );
        FindTemplateAndPrintMap(
            bigMask,
            mask,
            correlationMap,
            bestMatch,
            correlationVal,
            "bigMaskCorrelation.pgm"
            );
        FindTemplateAndPrintMap(
            frame1,
            mask,
            correlationMap,
            bestMatch,
            correlationVal,
            "frame1Correlation.pgm"
            );
        Rectangle window( 150, 150, 200, 200 );
        FindTemplateAndPrintMap(
            frame1,
            mask,
            correlationMap,
            bestMatch,
            correlationVal,
            "frame1WindowedCorrelation.pgm",
            &window
            );
        
        try {
            Image::TrackPixels( frame0, frame1, 17, 17, 9, 9, dispX, dispY );
            dispX.CreateAsciiPgm(Config::OutputPath() + "TrackinF0F1x.pgm");
            dispY.CreateAsciiPgm(Config::OutputPath() + "TrackinF0F1y.pgm");
        } catch (BadIndex bi) {
            std::cout << bi.what();
        }
        Image fullSpectre( 3 * bigMask.GetWidth(), 3 * bigMask.GetHeight(), 255 );
        for ( int x = 0; x < fullSpectre.GetWidth(); x++ ) {
            for ( int y = 0; y < fullSpectre.GetHeight(); y++ ) {
                int val = bigMask.GetGreyLvl( y - bigMask.GetHeight(), x - bigMask.GetWidth() );
                
                fullSpectre.SetGreyLvl( y, x, val );
            }
        }    
        fullSpectre.CreateAsciiPgm(Config::OutputPath() + "fullSpectre.pgm");
        
        //Image ft = frame0.FourierTransform();
        //ft.CreateAsciiPgm(Config::OutputPath() + "Frame0FT.pgm");
        
        //Image corr01 = frame0.Correlation(frame1);
        //Image corr10 = frame1.Correlation(frame0);
        //corr01.CreateAsciiPgm("corr01");
        //corr10.CreateAsciiPgm("corr10");
        
        Image diff01 = frame0.Difference(frame1);
        Image diff10 = frame1.Difference(frame0);
        diff01.CreateAsciiPgm(Config::OutputPath() + "diff01.pgm");
        diff10.CreateAsciiPgm(Config::OutputPath() + "diff10.pgm");
    }
    
    if ( c == 'k' ) {
        KinectInit(argc, argv);
    }

    if ( c == 'u' ) {
        QMyApplication program (argc, argv);
        //setBoubekQTStyle (raymini);
        QMyApplication::setStyle (new QPlastiqueStyle);
        program.setAttribute(Qt::AA_DontUseNativeMenuBar,true);
        Window * progWindow = new Window ();
        progWindow->setWindowTitle ("PIM380: A facial reconstruction program.");
        progWindow->show();
        program.connect (&program, SIGNAL (lastWindowClosed()), &program, SLOT (quit()));
        
    return program.exec ();
    }
    
    return 0;
}


