#include <iostream>
#include "Image.h"
#include "PointSet.h"
#include "Camera.h"

#define RES_PTSET_PATH "../resources/Project/PointSets/"

int main(int argc, char** argv) {
      
    /* Class image test */

    Image myImage("frame_20121108T103323.258153_rgb-ascci.pgm");

    /* Class Ply test  */
    
    PointSet psAscii, psBinary;    
    psAscii.LoadFromFile(RES_PTSET_PATH + std::string("frame000-ascii.ply"));
    psBinary.LoadFromFile(RES_PTSET_PATH + std::string("frame000-brut.ply"));

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
