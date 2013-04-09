#include <iostream>
#include "Image.h"
#include "PointSet.h"
#include "Camera.h"
#include "Config.h"

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

    CartesianCoordinate bestMatch;
    Image smallMask(Config::DataPath() + "smallMask.pgm");
    Image mask(Config::DataPath() + "mask.pgm");
    Image bigMask(Config::DataPath() + "bigMask.pgm");
    
    Image smallMaskCorrelation = bigMask.TemplateMatch( smallMask, bestMatch );
    std::cout << "Match found at (" << bestMatch.x << ", " 
                                    << bestMatch.y << ")" << std::endl;
    Image bigMaskCorrelation = bigMask.TemplateMatch( mask, bestMatch );
    std::cout << "Match found at (" << bestMatch.x << ", " 
                                    << bestMatch.y << ")" << std::endl;
    Image frame1Correlation = frame1.TemplateMatch( mask, bestMatch );
    std::cout << "Match found at (" << bestMatch.x << ", " 
                                    << bestMatch.y << ")" << std::endl;
    smallMaskCorrelation.CreateAsciiPgm(Config::OutputPath() + "smallMaskCorrelation.pgm");
    bigMaskCorrelation.CreateAsciiPgm(Config::OutputPath() + "bigMaskCorrelation.pgm");
    frame1Correlation.CreateAsciiPgm(Config::OutputPath() + "frame1Correlation.pgm");

//    Image ft = frame0.FourierTransform();
  //  ft.CreateAsciiPgm(Config::OutputPath() + "Frame0FT.pgm");
    
    //Image corr01 = frame0.Correlation(frame1);
    //Image corr10 = frame1.Correlation(frame0);
    //corr01.CreateAsciiPgm("corr01");
    //corr10.CreateAsciiPgm("corr10");

    Image diff01 = frame0.Difference(frame1);
    Image diff10 = frame1.Difference(frame0);
    diff01.CreateAsciiPgm(Config::OutputPath() + "diff01.pgm");
    diff10.CreateAsciiPgm(Config::OutputPath() + "diff10.pgm");

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
    
    return 0;
}
