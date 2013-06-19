#include "PNMImage.h"
#include "PPMImage.h"
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
#include "ImagePyramid.h"

extern void printColorChart (
    const int& width,
    const int& height,
    PPMImage& colorChart
);

void FindTemplateAndPrintMap(
    const ImageBase& iBaseImage,
    const ImageBase& iTemplate,
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

int main ( void ) {
    Config::LoadConfigs(Config::RootPath() + "settings");

    static const std::string RES_PTSET_PATH(Config::ResourcesPath() + "PointSets/");
    static const std::string RES_IMG_PATH(Config::ResourcesPath() + "Images/");

    Image frame0i(Config::OutputPath () + "CapturedFrames/image_8.pgm");
    Image frame0h ( frame0i.GetWidth(), frame0i.GetHeight(), frame0i.GetMaxGreyLevel () );
    Image frame0hinv ( frame0i.GetWidth(), frame0i.GetHeight(), frame0i.GetMaxGreyLevel () );
    Image frame0inv ( frame0i.GetWidth(), frame0i.GetHeight(), frame0i.GetMaxGreyLevel () );
    Image blank ( frame0i.GetWidth(), frame0i.GetHeight(), frame0i.GetMaxGreyLevel () );
    Image blankinv ( frame0i.GetWidth(), frame0i.GetHeight(), frame0i.GetMaxGreyLevel () );
    for ( unsigned int i = 0; i < frame0i.GetWidth (); i++ ) {
        for ( unsigned int j = 0; j < frame0i.GetHeight (); j++ ) {
            frame0h.SetNormed ( j, i, frame0i.GetNormed ( j, i ) * 0.25f );
            frame0hinv.SetNormed ( j, i, -frame0i.GetNormed ( j, i ) * 0.25f );
            frame0inv.SetNormed ( j, i, -frame0i.GetNormed ( j, i ) );
            blank.SetNormed ( j, i, 0.0f );
            blankinv.SetNormed ( j, i, -0.0f );
        }
    }
    std::cout << "Correlation F0 *  F0  : " << frame0i.Correlation ( frame0i ) << std::endl;
    std::cout << "Correlation F0 *  F0/2: " << frame0i.Correlation ( frame0h ) << std::endl;
    std::cout << "Correlation F0 * -F0/2: " << frame0i.Correlation ( frame0hinv ) << std::endl;
    std::cout << "Correlation F0 *  BL  : " << frame0i.Correlation ( blank ) << std::endl;
    std::cout << "Correlation BL * -BL  : " << blank.Correlation ( blankinv ) << std::endl;
    std::cout << "Correlation F0 * -F0  : " << frame0i.Correlation ( frame0inv ) << std::endl;

    Image frame0d(Config::OutputPath () + "CapturedFrames/depth_8.pgm");
    Image frame1i(Config::OutputPath () + "CapturedFrames/image_9.pgm");
    Image frame1d(Config::OutputPath () + "CapturedFrames/depth_9.pgm");

    PixelTracker pixTrack (0);

    pixTrack.SetReference ( 0, &frame0i, &frame0d );
    pixTrack.SetTarget ( 1, &frame1i, &frame1d );
    pixTrack.Track ();
    pixTrack.Export ( Config::OutputPath() + "CapturedFrames/disparity_8.ppm" );
}

int main_alt ( int argc, char** argv ) {
    Config::LoadConfigs(Config::RootPath() + "settings");

    static const std::string RES_PTSET_PATH(Config::ResourcesPath() + "PointSets/");
    static const std::string RES_IMG_PATH(Config::ResourcesPath() + "Images/");

    //PointSet ps0 (Config::OutputPath () + "CapturedFrames/pointset_8.ply");
    //PointSet ps1;
    //for ( unsigned int vId = 0; vId < ps0.GetNumVertices (); vId++ ) {
    //    Vertex vtx = ps0.GetVertex ( vId ); 
 
    //    unsigned int x = vId % 320u;
    //    unsigned int y = vId / 320u;

    //    vtx.SetUVCoord ( x, y );
    //    ps1.PushVertex ( vtx );
    //    if ( x < (319u) ) {
    //        if ( y < (239u) ) {
    //            Face f;

    //            f.v0 = vId;
    //            f.v1 = vId + 1u;
    //            f.v2 = vId + 320u;
    //            ps1.PushFace ( f );

    //            f.v0 = vId + 1u;
    //            f.v1 = vId + 320u;
    //            f.v2 = vId + 321u;
    //            ps1.PushFace ( f );
    //        }
    //    }
    //}
    //ps1.WriteToFile (Config::OutputPath () + "CapturedFrames/pointset_8wf.ply");


    //
    //Image frame0(Config::OutputPath () + "CapturedFrames/image_0.pgm");
    //Image* filtered = frame0.Filter ();
    //filtered->CreateAsciiPgm ( Config::OutputPath () + "filtered.pgm" );
    //delete filtered;

    //Image* subsampled = frame0.SubSample ( 2 );
    //subsampled->CreateAsciiPgm ( Config::OutputPath () + "subsampled2.pgm" );
    //delete subsampled;

    //subsampled = frame0.SubSample ( 4 );
    //subsampled->CreateAsciiPgm ( Config::OutputPath () + "subsampled4.pgm" );
    //delete subsampled;
    //subsampled = (Image*)0x0;

    //ImagePyramid pyr;
    //pyr.SetSamplingFactor ( 2 );
    //pyr.Assign ( &frame0 );
    //pyr.Export ( Config::OutputPath () + "Pyramids/image_0" ); 

    ParameterHandler* params = ParameterHandler::Instance ();
    const unsigned int& wSize = params->GetWindowSize ();
    const unsigned int& nSize = params->GetNeighbourhoodSize ();
    //dispX.SetMaxGreyLevel ( wSize );
    //dispY.SetMaxGreyLevel ( wSize );

    PixelTracker pixTracker (0);
    pixTracker.SetUp (
        wSize,
        wSize,
        nSize,
        nSize,
        0.95
    );

    Image frame1c(Config::OutputPath () + "CapturedFrames/image_1.pgm");
    pixTracker.Track (1, &frame1c);
    pixTracker.Export ( Config::OutputPath () + "/TCSNorefinement_"+toString(0.95)+"n.ppm" );

    pixTracker.disparityRefinement (&frame1c);
    pixTracker.Export ( Config::OutputPath () + "/TCSrefinement_"+toString(0.95)+"n.ppm" );

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

    SubImage figure;
    SubImage patch;
    frame0.GetSubImage (
        200,
        140,
        150,
        250,
        figure
    );
    figure.GetSubImage (
        50,
        50,
        50,
        50,
        patch
    );
    Image smallMask(Config::DataPath() + "smallMask.pgm");
    Image mask(Config::DataPath() + "mask.pgm");
    Image bigMask(Config::DataPath() + "bigMask.pgm");
    
    Image correlationMap( 1, 1, 255 );
    float correlationVal = 0.0f;
    CartesianCoordinate bestMatch;

    char c;
    std::cout << "Select Mode: Kinect ('k') Image Tracking('t') UI (u) Other:('o')" << std::endl;
    std::cin  >> c; 

    if ( c == 't' ) {
        figure.CreateAsciiPgm (
            Config::OutputPath() + "figure.pgm"
        );
        patch.CreateAsciiPgm (
            Config::OutputPath() + "patch.pgm"
        );

        std::vector<unsigned int> anchors;
        Image::CalculateAnchors (
            35,
            0,
            Config::OutputPath () + "CapturedFrames/",
            "image_",
            anchors
        );
        std::cout << "Anchor frames: ";
        for ( unsigned int i = 0; i < anchors.size(); i++ ) {
            std::cout << anchors[i] << " ";
        }
        std::cout << std::endl;

        std::cout   << "Correlation between frames F0 and F0: "
                    << frame0.Correlation ( frame0 )
                    << std::endl;
        std::cout   << "Correlation between frames F0 and F1: "
                    << frame0.Correlation ( frame1 )
                    << std::endl;
        std::cout   << "Error score between frames F0 and F0: "
                    << ImageBase::CalculateErrorScore ( frame0, frame0 )
                    << std::endl;
        std::cout   << "Error score between frames F0 and F1: "
                    << ImageBase::CalculateErrorScore ( frame0, frame1 )
                    << std::endl;
        //FindTemplateAndPrintMap(
        //    frame0,
        //    figure,
        //    correlationMap,
        //    bestMatch,
        //    correlationVal,
        //    "frame0figureCorrelation.pgm"
        //);
        //FindTemplateAndPrintMap(
        //    figure,
        //    patch,
        //    correlationMap,
        //    bestMatch,
        //    correlationVal,
        //    "figurePatchCorrelation.pgm"
        //);
        FindTemplateAndPrintMap(
            bigMask,
            smallMask,
            correlationMap,
            bestMatch,
            correlationVal,
            "smallMaskCorrelation.pgm"
        );

       // FindTemplateAndPrintMap(
       //     bigMask,
       //     mask,
       //     correlationMap,
       //     bestMatch,
       //     correlationVal,
       //     "bigMaskCorrelation.pgm"
       // );
       // 
       // FindTemplateAndPrintMap(
       //     frame1,
       //     mask,
       //     correlationMap,
       //     bestMatch,
       //     correlationVal,
       //     "frame1Correlation.pgm"
       // );

       // Rectangle window( 150, 150, 200, 200 );
       // FindTemplateAndPrintMap(
       //     frame1,
       //     mask,
       //     correlationMap,
       //     bestMatch,
       //     correlationVal,
       //     "frame1WindowedCorrelation.pgm",
       //     &window
       // );
        
        try {
            ParameterHandler* params = ParameterHandler::Instance ();
            const unsigned int& wSize = params->GetWindowSize ();
            const unsigned int& nSize = params->GetNeighbourhoodSize ();
            dispX.SetMaxGreyLevel ( wSize );
            dispY.SetMaxGreyLevel ( wSize );

            PixelTracker pixTracker (0);
            pixTracker.SetUp (
                wSize,
                wSize,
                nSize,
                nSize,
                0.95
            );
            
            pixTracker.Track ( 1 );
            pixTracker.Export ( Config::OutputPath () + "/Tracking01_"+toString(0.95)+"n.ppm" );
            std::cout << "Track 0 Done" << std::endl;

            pixTracker.SetRejectionTreshold(0.90); 
            pixTracker.Track ( 1 );
            pixTracker.Export ( Config::OutputPath () + "/Tracking01_"+toString(0.90)+"n.ppm" );
            std::cout << "Track 1 Done" << std::endl;

            pixTracker.SetRejectionTreshold(0.85); 
            pixTracker.Track ( 1 );
            pixTracker.Export ( Config::OutputPath () + "/Tracking01_"+toString(0.85)+"n.ppm" );
            std::cout << "Track 2 Done" << std::endl;


            ImageBase::TrackPixels (
                frame0,
                frame1,
                wSize,
                wSize,
                nSize,
                nSize,
                dispX,
                dispY
            );
            dispX.CreateAsciiPgm(Config::OutputPath() + "TrackingF0F1x.pgm");
            dispY.CreateAsciiPgm(Config::OutputPath() + "TrackingF0F1y.pgm");
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
    }
    
    if ( c == 'k' ) {
        KinectInit(argc, argv);
    }


}
