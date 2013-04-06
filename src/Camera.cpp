/*****************************************************************
*  This file based from the samples of OpenNi 1.x Alpha                *
*  from Copyright (C) 2011 PrimeSense Ltd.                             *
******************************************************************/

#include "Camera.h"

using namespace xn;

//----------------------------------------------------------------
// Statics
//-----------------------------------------------------------------
Camera* Camera::sm_pInstance = NULL;

//-----------------------------------------------------------------
// GLUT Hooks
//-----------------------------------------------------------------
void Camera::glutIdle (void)
{
    // Display the frame
    glutPostRedisplay();
}

void Camera::glutDisplay (void)
{
    Instance().Display();
}

void Camera::glutKeyboard (unsigned char key, int x, int y)
{
    Instance().OnKey(key, x, y);
}


//-----------------------------------------------------------------
// Method Definitions
//------------------------------------------------------------------
Camera::Camera(xn::Context& context)
    :m_pTexMap(NULL),
     m_nTexMapX(0),
     m_nTexMapY(0),
     m_eViewState(DEFAULT_DISPLAY_MODE),
     m_rContext(context)
{}

Camera::~Camera()
{
    delete[] m_pTexMap;
}

Camera& Camera::CreateInstance( xn::Context& context )
{
    assert(!sm_pInstance);
    return *(sm_pInstance = new Camera(context));
}

void Camera::DestroyInstance(Camera& instance)
{
    assert(sm_pInstance);
    assert(sm_pInstance == &instance);
    delete sm_pInstance;
    sm_pInstance = NULL;
}

Camera& Camera::Instance()
{
    assert(sm_pInstance);
    return *sm_pInstance;
}

XnStatus Camera::Init(int argc, char **argv)
{
    XnStatus rc;

    /* Verify Depth node in xml */
    rc = m_rContext.FindExistingNode(XN_NODE_TYPE_DEPTH, m_depth);
    CHECK_RC(rc,"No depth node exists! Check your XML.");

    /* Verify Image noede in xml */
    rc = m_rContext.FindExistingNode(XN_NODE_TYPE_IMAGE, m_image);
    CHECK_RC(rc,"No image node exists! Check your XML.");

    /* Gets the current depth-map meta data */
    m_depth.GetMetaData(m_depthMD);
    m_image.GetMetaData(m_imageMD);

    /* Hybrid mode isn't supported in this sample */
    if (m_imageMD.FullXRes() != m_depthMD.FullXRes() || m_imageMD.FullYRes() != m_depthMD.FullYRes())
    {
        printf ("The device depth and image resolution must be equal!\n");
        return 1;
    }

    /* RGB is the only image format supported. */
    if (m_imageMD.PixelFormat() != XN_PIXEL_FORMAT_RGB24)
    {
        printf("The device image format must be RGB24\n");
        return 1;
    }

    /* Texture map init */
    m_nTexMapX = MIN_CHUNKS_SIZE(m_depthMD.FullXRes(), TEXTURE_SIZE);
    m_nTexMapY = MIN_CHUNKS_SIZE(m_depthMD.FullYRes(), TEXTURE_SIZE);
    m_pTexMap = new XnRGB24Pixel[m_nTexMapX * m_nTexMapY];
    
    /* Initiate a frame with number 0 */
    m_nbFrames = 0;
    
    return InitOpenGL(argc, argv);
}

XnStatus Camera::Run()
{
    // Per frame code is in Display
    glutMainLoop();	// Does not return!

    return XN_STATUS_OK;
}

XnStatus Camera::InitOpenGL(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(GL_WIN_SIZE_X, GL_WIN_SIZE_Y);
    glutCreateWindow ("PIM380 - Kinect");
    //	glutFullScreen();
    glutSetCursor(GLUT_CURSOR_NONE);

    InitOpenGLHooks();

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    return XN_STATUS_OK;
}

void Camera::InitOpenGLHooks()
{
    glutKeyboardFunc(glutKeyboard);
    glutDisplayFunc(glutDisplay);
    glutIdleFunc(glutIdle);
}

int Camera::Display()
{
    XnStatus rc = XN_STATUS_OK;

     /* Read a new frame */
    rc = m_rContext.WaitAnyUpdateAll();
    CHECK_RC(rc,"Read failed");

    m_depth.GetMetaData(m_depthMD);
    m_image.GetMetaData(m_imageMD);

    const XnDepthPixel* pDepth = m_depthMD.Data();
    const XnUInt8* pImage = m_imageMD.Data();

    unsigned int nImageScale = GL_WIN_SIZE_X / m_depthMD.FullXRes();

    // Copied from Camera
    // Clear the OpenGL buffers
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Setup the OpenGL viewpoint
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, GL_WIN_SIZE_X, GL_WIN_SIZE_Y, 0, -1.0, 1.0);

    // Calculate the accumulative histogram (the yellow display...)
    xnOSMemSet(m_pDepthHist, 0, MAX_DEPTH*sizeof(float));

    unsigned int nNumberOfPoints = 0;
    for (XnUInt y = 0; y < m_depthMD.YRes(); ++y)
    {
        for (XnUInt x = 0; x < m_depthMD.XRes(); ++x, ++pDepth)
        {
            if (*pDepth != 0)
            {
                m_pDepthHist[*pDepth]++;
                nNumberOfPoints++;
            }
        }
    }
    for (int nIndex=1; nIndex<MAX_DEPTH; nIndex++)
    {
        m_pDepthHist[nIndex] += m_pDepthHist[nIndex-1];
    }
    if (nNumberOfPoints)
    {
        for (int nIndex=1; nIndex<MAX_DEPTH; nIndex++)
        {
            m_pDepthHist[nIndex] = (unsigned int)(256 * (1.0f - (m_pDepthHist[nIndex] / nNumberOfPoints)));
        }
    }

    xnOSMemSet(m_pTexMap, 0, m_nTexMapX*m_nTexMapY*sizeof(XnRGB24Pixel));


    // check if we need to draw image frame to texture
    if (m_eViewState == DISPLAY_MODE_OVERLAY ||
        m_eViewState == DISPLAY_MODE_IMAGE)
    {
        const XnRGB24Pixel* pImageRow = m_imageMD.RGB24Data();
        XnRGB24Pixel* pTexRow = m_pTexMap + m_imageMD.YOffset() * m_nTexMapX;
        
        for (XnUInt y = 0; y < m_imageMD.YRes(); ++y)
        {
            const XnRGB24Pixel* pImage = pImageRow;
            XnRGB24Pixel* pTex = pTexRow + m_imageMD.XOffset();
            
            for (XnUInt x = 0; x < m_imageMD.XRes(); ++x, ++pImage, ++pTex)
            {
                *pTex = *pImage;
            }

            pImageRow += m_imageMD.XRes();
            pTexRow += m_nTexMapX;
        }
    }

    // check if we need to draw depth frame to texture
    if (m_eViewState == DISPLAY_MODE_OVERLAY ||
        m_eViewState == DISPLAY_MODE_DEPTH)
    {
        const XnDepthPixel* pDepthRow = m_depthMD.Data();
        XnRGB24Pixel* pTexRow = m_pTexMap + m_depthMD.YOffset() * m_nTexMapX;
        for (XnUInt y = 0; y < m_depthMD.YRes(); ++y)
        {
            const XnDepthPixel* pDepth = pDepthRow;
            XnRGB24Pixel* pTex = pTexRow + m_depthMD.XOffset();

            for (XnUInt x = 0; x < m_depthMD.XRes(); ++x, ++pDepth, ++pTex)
            {
                if (*pDepth != 0)
                {
                    int nHistValue = m_pDepthHist[*pDepth];
                    pTex->nRed = nHistValue;
                    pTex->nGreen = nHistValue;
                    pTex->nBlue = nHistValue;
                }
            }

            pDepthRow += m_depthMD.XRes();
            pTexRow += m_nTexMapX;
        }
    }
    
    // Create the OpenGL texture map
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_nTexMapX, m_nTexMapY, 0, GL_RGB, GL_UNSIGNED_BYTE, m_pTexMap);


    // Display the OpenGL texture map
    glColor4f(1,1,1,1);

    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);

    int nXRes = m_depthMD.FullXRes();
    int nYRes = m_depthMD.FullYRes();
	
    // upper left
    glTexCoord2f(0, 0);
    glVertex2f(0, 0);
    // upper right
    glTexCoord2f((float)nXRes/(float)m_nTexMapX, 0);
    glVertex2f(GL_WIN_SIZE_X, 0);
    // bottom right
    glTexCoord2f((float)nXRes/(float)m_nTexMapX, (float)nYRes/(float)m_nTexMapY);
    glVertex2f(GL_WIN_SIZE_X, GL_WIN_SIZE_Y);
    // bottom left
    glTexCoord2f(0, (float)nYRes/(float)m_nTexMapY);
    glVertex2f(0, GL_WIN_SIZE_Y);

    glEnd();
    glDisable(GL_TEXTURE_2D);

    // Subclass draw hook
    DisplayPostDraw();

    // Swap the OpenGL display buffers
    glutSwapBuffers();

    return 0;
    
}

void Camera::OnKey(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 27: // ESC
        exit (1);
    case '1':
        m_eViewState = DISPLAY_MODE_OVERLAY;
        m_depth.GetAlternativeViewPointCap().SetViewPoint(m_image);
        break;
    case '2':
        m_eViewState = DISPLAY_MODE_DEPTH;
        m_depth.GetAlternativeViewPointCap().ResetViewPoint();
        break;
    case '3':
        m_eViewState = DISPLAY_MODE_IMAGE;
        m_depth.GetAlternativeViewPointCap().ResetViewPoint();
        break;
        /*Save depth and image to a pgm file*/    
    case 'p':
    case 'P':
        captureSingleFrame();
        break;
    }
}

void Camera::ScalePoint(XnPoint3D& point)
{
    point.X *= GL_WIN_SIZE_X;
    point.X /= m_depthMD.XRes();

    point.Y *= GL_WIN_SIZE_Y;
    point.Y /= m_depthMD.YRes();
}

std::string Camera::Int2Str(int nb){
    std::ostringstream ss;
    ss << nb;
    return ss.str();
}

void Camera::captureSingleFrame()
{
    Image camImg(m_imageMD.XRes(),m_imageMD.YRes(),65535);
    Image camDepth(m_depthMD.XRes(),m_depthMD.YRes(),65535); 

    const XnRGB24Pixel* pImageRow = m_imageMD.RGB24Data();
    const XnDepthPixel* pDepthRow = m_depthMD.Data();
     
    for (XnUInt y = 0; y < m_imageMD.YRes(); ++y)
    {
        const XnRGB24Pixel* pImage = pImageRow;
        const XnDepthPixel* pDepth = pDepthRow;
        
        for (XnUInt x = 0; x < m_imageMD.XRes(); ++x, ++pImage, ++pDepth)
        {
            /* HDTV rgb to grayscale*/
            camImg(x,y) = pImage -> nRed *  0.2126 +      \
                     pImage -> nBlue * 0.0722 + \
                     pImage-> nGreen * 0.7152;
            /* HDTV rgb to grayscale*/
            if (*pDepth != 0)
            {
                camDepth(x,y) =  m_pDepthHist[*pDepth];
            }
            
        }

        pImageRow += m_imageMD.XRes();
        pDepthRow += m_depthMD.XRes();

    }

    std::string str_aux = "CapturedFrames/image_"+ Int2Str(m_nbFrames)  +".pgm";    camImg.CreateAsciiPgm(str_aux);
    str_aux = "CapturedFrames/depth_"+ Int2Str(m_nbFrames)  +".pgm"; 
    camDepth.CreateAsciiPgm(str_aux);
    
    /* Frame saved: increase ID*/
    m_nbFrames++;  
    
}
