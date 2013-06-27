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
#include <iomanip>
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
#include "Clip.h"

extern void PrintColorChart (
    const int& width,
    const int& height,
    PPMImage& colorChart
);

extern Vec3Df BilinearInterpolation (
    const Vec3Df&   f00,
    const Vec3Df&   f01,
    const Vec3Df&   f10,
    const Vec3Df&   f11,
    const float&    px0,
    const float&    py0,
    const float&    px1,
    const float&    py1,
    const float&    px,
    const float&    py
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

    unsigned int ref = 82;
    unsigned int start = 36;
    unsigned int end = 84;

    std::string prefix = Config::FramesPath () + "f";
    std::string refpath  = prefix + toString ( ref ) + "/";
    std::string startpath = prefix + toString ( start ) + "/";
    std::string endpath = prefix + toString ( end ) + "/";

    PixelTracker pt;
    PointSet* refMesh =  new PointSet ( refpath + "mesh.ply" );
    Image* refImg = new Image ( refpath + "texture.pgm"); 
    Image* refDep = new Image ( refpath + "depthMap.pgm");
    pt.SetReference ( ref, refImg, refDep );

    PointSet* tarMesh = new PointSet ( startpath + "mesh.ply" );
    Image* tarImg = new Image ( startpath + "texture.pgm");
    Image* tarDep = new Image ( startpath + "depthMap.pgm");
    pt.SetTarget ( start, tarImg, tarDep );
    pt.Track ();
    pt.CalculateMotionField ( refMesh, tarMesh );
    pt.Export ( refpath + "track/" + Int2Str(start) + "/" );

    tarMesh->LoadFromFile ( endpath + "mesh.ply" );
    tarImg->LoadFromFile ( endpath + "texture.pgm" );
    tarDep->LoadFromFile ( endpath + "depthMap.pgm" );

    pt.SetTarget ( end, tarImg, tarDep );
    pt.Track ();
    pt.CalculateMotionField ( refMesh, tarMesh );
    pt.Export ( refpath + "track/" + Int2Str(end) + "/" );

    Clip cl ( start, end );

    Frame f;
    PointSet mesh;
    mesh.LoadFromFile (Config::FramesPath () + "f"+toString(ref)+"/mesh.ply" );
    mesh.WriteToFile ( Config::OutputPath () + "Animation/4/frame000_"+toString(ref)+"r.ply");

    f.LoadMotionField ( Config::FramesPath () + "f" + toString(ref) + "/track/" + toString (start) + "/" );

    unsigned int i = 1;
    for ( unsigned int fId = start; fId < end; fId++, i++ ) {
        std::string suffix;
        if ( fId == start ) suffix = "a.ply";
        else suffix = ".ply";

        std::stringstream iStr;
        iStr << std::setfill('0') << std::setw(3) << i;

        if ( fId == ref ) {
            mesh.LoadFromFile (Config::FramesPath () + "f"+toString(ref)+"/mesh.ply");
            mesh.WriteToFile ( Config::OutputPath () + "Animation/4/frame"+iStr.str()+"_"+toString(ref)+"r.ply");
        }
        else f.ApplyMotionField ( mesh );
        
        mesh.WriteToFile ( Config::OutputPath () + "Animation/4/frame"+iStr.str()+"_"+toString(fId)+suffix);

        f.LoadMotionField ( Config::FramesPath () + "f" + toString(fId) + "/track/" + toString (fId+1) + "/" );
    }
    //for ( unsigned int fId = end; fId >= start; fId--, i++ ) {
    //    std::string suffix;
    //    if ( fId == start ) suffix = "a.ply";
    //    else suffix = ".ply";

    //    f.ApplyMotionField ( mesh );
    //    
    //    std::stringstream iStr;
    //    iStr << std::setfill('0') << std::setw(3) << i;
    //    mesh.WriteToFile ( Config::OutputPath () + "Animation/4/frame"+iStr.str()+"_"+toString(fId)+suffix);

    //    f.LoadMotionField ( Config::FramesPath () + "f" + toString(fId) + "/track/" + toString (fId-1) + "/" );
    //}

    return 0;
}

int main_tits ( void ) {
    Config::LoadConfigs(Config::RootPath() + "settings");

    //for (unsigned int i = 0; i < 35; i++ ) {
    //    std::string outputPath = Config::FramesPath () + "filtered/f" + Int2Str(i) + "/";
    //    system ( ( "mkdir -p " + outputPath ).c_str () );

    //    std::string prefix = Config::FramesPath () + "f" + Int2Str(i) + "/";
    //    std::cout << prefix << std::endl;

    //    PointSet* ps = new PointSet ( prefix + "mesh.ply" );
    //    Image* tex = new Image ( prefix + "texture.pgm" );
    //    Image* img = new Image ( prefix + "depthMap.pgm" );
    //    Image* im = new Image ( img->GetWidth (), img->GetHeight (), img->GetMaxGreyLevel () );
    //    unsigned int zeroCount = 0;
    //    do {
    //        zeroCount = 0;
    //        for ( int x = 0; x < img->GetWidth (); x++ ) {
    //            for ( int y = 0; y < img->GetHeight (); y++ ) {
    //                int value = img->GetGreyLvl ( y, x );

    //                if ( value ) {
    //                    im->SetGreyLvl(y,x,value);
    //                    continue;
    //                };

    //                float median = 0;
    //                unsigned int zeroes = 0;
    //                int sv[9];
    //                unsigned int count = 0;
    //                for ( int dx = -1; dx <= 1; dx++ ) {
    //                    for ( int dy = -1; dy <= 1; dy++ ) {

    //                        int val = img->GetGreyLvl ( y + dy, x + dx );

    //                        if ( !val ) continue;

    //                        if ( count ) {
    //                            sv[count++] = val;
    //                        } else {
    //                            int idx = 0;
    //                            for ( idx = 0; idx < count && sv[idx]<val; idx++ );
    //                            int tmp[9];
    //                            memcpy ( tmp, sv + idx, (count - idx)*sizeof(int));
    //                            sv[idx] = val;
    //                            memcpy ( sv + idx + 1, tmp, (count++ - idx)*sizeof(int));
    //                        }
    //                    }
    //                }
    //                if ( count && count & 0x1 ) {
    //                    median = sv[count >> 1];
    //                } else if ( count ) {
    //                    median = (sv[count>>1]+sv[count>>2+1]) / 2.0f;
    //                }
    //                if ( median == 0 ) zeroCount++;
    //                im->SetGreyLvl ( y, x, nearbyint ( median ) );
    //            }
    //        }
    //        Image* tmp = img;
    //        img = im;
    //        im = tmp;
    //    } while ( zeroCount );
    //    for ( int x = 0; x < img->GetWidth (); x++ ) {
    //        for ( int y = 0; y < img->GetHeight (); y++ ) {
    //            Vertex* v = ps->GetVertex(x,y);

    //            Vec3Df pos = v->GetPosition();
    //            pos[2] = -im->GetGreyLvl ( y, x );
    //            v->SetPosition (pos);
    //        }
    //    }
    //    ps->WriteToFile ( outputPath + "mesh.ply" );
    //    tex->CreateAsciiPgm ( outputPath + "texture.ply" );
    //    im->CreateAsciiPgm ( outputPath + "depthMap.pgm" );
    //    
    //    delete ps;
    //    delete tex;
    //    delete img;
    //    delete im;
    //}

    //PPMImage img1; img1.LoadFromFile ( "img1.ppm" );
    //PPMImage img2; img2.LoadFromFile ( "img2.ppm" );


    //PPMImage colorChart;
    //colorChart.LoadFromFile ( Config::DataPath () + "ColorChart.ppm" );
    //colorChart.WriteToFile  ( "GreyLevels_bin.pgm", GREYMAP | BINARY );
    //colorChart.WriteToFile  ( "GreyLevels_ascii.pgm", GREYMAP | ASCII );
    //PrintColorChart (320,320,Config::DataPath () + "ColorChart.ppm" );

    //img1.WriteToFile ( "img1_ascii.ppm", PIXMAP | ASCII );
    //img1.WriteToFile ( "img1_binary.ppm", PIXMAP | BINARY );
    //img2.WriteToFile ( "img2_ascii.ppm", PIXMAP | ASCII );
    //img2.WriteToFile ( "img2_binary.ppm", PIXMAP | BINARY );
}

int main_math ( void ) {
    Vec3Df f00 ( 0.0f, 0.0f, 0.0f );
    Vec3Df f01 ( 0.0f, 2.0f, 0.0f );
    Vec3Df f10 ( 2.0f, 0.0f, 0.0f );
    Vec3Df f11 ( 2.0f, 2.0f, 0.0f );

    std::cout << BilinearInterpolation (f00,f01,f10,f11,0,0,1,1,0.4,0.6) << std::endl;
    std::cout << BilinearInterpolation (f00,f01,f10,f11,0,0,0,1,0.4,0.6) << std::endl;
    std::cout << BilinearInterpolation (f00,f01,f10,f11,0,0,1,0,0.4,0.6) << std::endl;
    std::cout << BilinearInterpolation (f00,f01,f10,f11,0,0,0,0,0.4,0.6) << std::endl;
    std::cout << BilinearInterpolation (f00,f01,f10,f11,1,1,1,1,0.4,0.6) << std::endl;

    Matrix33 m ( Vec3Df (1, 2, 3), Vec3Df ( 4, 5, 6), Vec3Df(7,8,9) );
    std::cout << m.Det () << std::endl;
    

    Parabola p ( 3, 4, 5, 0, 7, 4 );

    std::cout << p.Eval (  0 ) << std::endl;
    std::cout << p.Eval (  1 ) << std::endl;
    std::cout << p.Eval (  2 ) << std::endl;
    std::cout << p.Eval (  3 ) << std::endl;
    std::cout << p.Eval (  4 ) << std::endl;
    std::cout << p.Eval (  5 ) << std::endl;
    std::cout << p.Eval (  6 ) << std::endl;
    std::cout << p.Eval (  7 ) << std::endl;
    std::cout << p.Eval (  8 ) << std::endl;
    std::cout << p.Eval (  9 ) << std::endl;
    std::cout << p.Eval ( 10 ) << std::endl;

    return 0;
}

int main_total ( void ) {
    Config::LoadConfigs(Config::RootPath() + "settings");

    static const std::string RES_PTSET_PATH(Config::ResourcesPath() + "PointSets/");
    static const std::string RES_IMG_PATH(Config::ResourcesPath() + "Images/");

    Image frame0i(Config::FramesPath () + "f8/texture.pgm");
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

    Image frame0d(Config::FramesPath () + "f8/depth.pgm");
    Image frame1i(Config::FramesPath () + "f9/texture.pgm");
    Image frame1d(Config::FramesPath () + "f9/depth.pgm");

    PointSet mesh0 ( Config::FramesPath () + "f8/mesh.ply" );
    PointSet mesh1 ( Config::FramesPath () + "f9/mesh.ply" );

    PixelTracker pixTrack;
    pixTrack.SetReference ( 0, &frame0i, &frame0d );
    pixTrack.SetTarget ( 1, &frame1i, &frame1d );
    pixTrack.Track ();
    pixTrack.CalculateMotionField (
        &mesh0,
        &mesh1        
    );
    pixTrack.Export (Config::FramesPath () + "f8/");
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
