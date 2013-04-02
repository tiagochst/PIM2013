#include <iostream>
#include "Image.h"
#include "PointSet.h"
#include "Camera.h"

static const std::string RES_PTSET_PATH("../resources/Project/PointSets/");
static const std::string RES_IMG_PATH("../resources/Project/Images/");

int main(int argc, char** argv) {
    /* Class image test */
    // Image myImage(RES_IMG_PATH + "frame_20121108T103323.258153_rgb-ascci.pgm");

    // /* Class Ply test  */
    // PointSet psAscii, psBinary;    
    // psAscii.LoadFromFile(RES_PTSET_PATH + "frame000-ascii.ply");
    // psBinary.LoadFromFile(RES_PTSET_PATH + "frame000-brut.ply");
 
    Image frame0(RES_IMG_PATH + "frame_20121108T103323.258153_rgb-brut.pgm");
    Image frame1(RES_IMG_PATH + "frame_20121108T103323.390878_rgb-brut.pgm");

    //Coordinate bestMatch;
    Image mask("mask.pgm");
    //Image mask("bigMask.pgm");
    
    frame1.CrossCorrelation(mask);
    //std::cout << "Match found at (" << maxLoc.x << ", " 
    //                                << maxLoc.y << ")" << std::endl;
    //CVcorrelation.CreateAsciiPgm("CVcorrelation.pgm");

    //Image corr01 = frame0.Correlation(frame1);
    //Image corr10 = frame1.Correlation(frame0);
    //corr01.CreateAsciiPgm("corr01");
    //corr10.CreateAsciiPgm("corr10");

    Image diff01 = frame0.Difference(frame1);
    Image diff10 = frame1.Difference(frame0);
    diff01.CreateAsciiPgm("diff01.pgm");
    diff10.CreateAsciiPgm("diff10.pgm");


    /* Class Camera test: Kinect's  */
    xn::Context        g_context;
    xn::ScriptNode  g_scriptNode;
    xn::EnumerationErrors errors;
    XnStatus                  rc;
    
    /* Create a context with default settings */
    rc = g_context.InitFromXmlFile(SAMPLE_XML_PATH, g_scriptNode, &errors);
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
