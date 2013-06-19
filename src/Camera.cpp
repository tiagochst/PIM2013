/*****************************************************************
*  This file based from the samples of OpenNi 1.x Alpha                *
*  from Copyright (C) 2011 PrimeSense Ltd.                             *
******************************************************************/

#include "Camera.h"
#include "ParameterHandler.h"
#include "FileWriterServices.h"
#include "MathUtils.h"

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
    :   m_pTexMap(NULL),
        m_help(0),
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

XnStatus Camera::Setup ( int argc, char **argv ) {
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
    if (
            m_imageMD.FullXRes() != m_depthMD.FullXRes()
        ||  m_imageMD.FullYRes() != m_depthMD.FullYRes()
    ) {
        printf ("The device depth and image resolution must be equal!\n");
        return 1;
    }

    /* RGB is the only image format supported. */
    if (
        m_imageMD.PixelFormat() != XN_PIXEL_FORMAT_RGB24
    ) {
        printf("The device image format must be RGB24\n");
        return 1;
    }

    m_depth.GetAlternativeViewPointCap().SetViewPoint(m_image);

    /* Texture map init */
    m_nTexMapX = MIN_CHUNKS_SIZE(m_depthMD.FullXRes(), TEXTURE_SIZE);
    m_nTexMapY = MIN_CHUNKS_SIZE(m_depthMD.FullYRes(), TEXTURE_SIZE);
    m_pTexMap = new XnRGB24Pixel[m_nTexMapX * m_nTexMapY];
    
    /* Initiate a frame with number 0 */
    m_nbFrames = 0;

    return rc;
}

XnStatus Camera::Init(int argc, char **argv)
{
    XnStatus rc = Setup(argc, argv);
    CHECK_RC(rc, "Error setting up kinect camera");
    
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

void Camera::WaitUpdateCamera () {
    XnStatus rc = m_rContext.WaitAnyUpdateAll ();
    //CHECK_RC ( rc, "Read failed" );

    m_depth.GetMetaData ( m_depthMD );
    m_image.GetMetaData ( m_imageMD );
    const XnDepthPixel* pDepth = m_depthMD.Data();

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

    ParameterHandler* params = ParameterHandler::Instance ();
    if ( params->GetCaptureMode() == true ) {
        captureSingleFrame ();
    }
}

void Camera::BuildTextureMaps () {
    // Initialize memory to 0.
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
}

int Camera::Display()
{
    // Copied from Camera
    // Clear the OpenGL buffers
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Setup the OpenGL viewpoint
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, GL_WIN_SIZE_X, GL_WIN_SIZE_Y, 0, -1.0, 1.0);

    WaitUpdateCamera ();

    BuildTextureMaps ();
    
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

    if(m_help){
        drawHelpScreen();
    }
    
    // Swap the OpenGL display buffers
    glutSwapBuffers();

    return 0;
    
}

/*!
 *  \brief  Treats an event of button pressed
 */
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
        m_depth.GetAlternativeViewPointCap().SetViewPoint(m_image);
        //m_depth.GetAlternativeViewPointCap().ResetViewPoint();
        break;
    case '3':
        m_eViewState = DISPLAY_MODE_IMAGE;
        m_depth.GetAlternativeViewPointCap().SetViewPoint(m_image);
        //m_depth.GetAlternativeViewPointCap().ResetViewPoint();
        break;
        /*Save depth and image to a pgm file*/    
    case 'p':
    case 'P':
        captureSingleFrame();
        break;
    case '?':
        if(m_help == 0){
            m_help = 1;
        }
        else{
            m_help = 0;
        }
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

void Camera::ReadFrame (
    Image*      oBrightness,
    Image*      oDepth,
    PointSet*   oPoints
) {
    const XnUInt XOffset    = 160u;
    const XnUInt YOffset    = 120u;
    const XnUInt CropWidth  = 320u;
    const XnUInt CropHeight = 240u;

    const XnUInt& width   = m_imageMD.XRes();
    const XnUInt& height  = m_imageMD.YRes();

    if ( oBrightness ) {
        oBrightness->SetDimensions (
            CropWidth,
            CropHeight
        );
        oBrightness->SetMaxGreyLevel ( 255 );
    }
    if ( oDepth ) {
        oDepth->SetDimensions (
            CropWidth,
            CropHeight
        );
        oDepth->SetMaxGreyLevel ( 5000 );
    }

    const XnRGB24Pixel* pImageRow   = m_imageMD.RGB24Data()
                                    + YOffset * width
                                    + XOffset;
    const XnDepthPixel* pDepthRow   = m_depthMD.Data()
                                    + YOffset * width
                                    + XOffset;

    std::vector<XnPoint3D> projectivePoints;
    std::vector<Color> colors;
    std::vector<CartesianCoordinate> uvCoords;
    for (XnUInt y = 0; y < CropHeight; ++y)
    {
        const XnRGB24Pixel* pImage = pImageRow;
        const XnDepthPixel* pDepth = pDepthRow;
        
        for (XnUInt x = 0; x < CropWidth; ++x, ++pImage, ++pDepth)
        {
            Color c (
                (int)pImage->nRed,
                (int)pImage->nGreen,
                (int)pImage->nBlue
            );

            int depthVal = 0;
            //if (*pDepth != 0)
            //{
            //    depthVal = m_pDepthHist[*pDepth];
            //}
            depthVal = *pDepth;

            /* HDTV rgb to grayscale*/
            if ( oBrightness ) {
                oBrightness->SetGreyLvl (
                    y,
                    x,
                    c.Brightness ()
                );
            }
            
            if ( oDepth ) {
                oDepth->SetGreyLvl (
                    y,
                    x,
                    depthVal
                );
            }
            
            if ( oPoints ) {
                //if (depthVal > 0.0f && depthVal < 1500) {
                    XnPoint3D point;
                    point.X  = x;
                    point.Y  = y;
                    point.Z  = depthVal;
                    projectivePoints.push_back ( point );
                    colors.push_back(c);
                    CartesianCoordinate uvc;
                    uvc.x = x;
                    uvc.y = y;
                    uvCoords.push_back(uvc);
                //}
            }
        }

        pImageRow += m_imageMD.XRes();
        pDepthRow += m_depthMD.XRes();
    }
    if ( oPoints ) {
        std::vector<XnPoint3D> realWorld;
        realWorld.resize ( projectivePoints.size () );
        m_depth.ConvertProjectiveToRealWorld (
            projectivePoints.size (),
            &(projectivePoints[0]),
            &(realWorld[0])
        );
        for ( int vtx = 0; vtx < realWorld.size (); vtx++ ) {
            XnPoint3D pt = realWorld[vtx];
            Color& c = colors[vtx];
            CartesianCoordinate& uvc = uvCoords[vtx];

            unsigned int x = vtx % CropWidth;
            unsigned int y = vtx / CropWidth;

            oPoints->PushVertex (
                Vertex (
                    Vec3Df (
                        (float)( pt.X ),
                        (float)( pt.Y ),
                        (float)( -pt.Z )
                    ),
                    Vec3Df (
                        1,
                        0,
                        0
                    ),
                    c,
                    uvc.x,
                    uvc.y
                )
            );

            if ( x < (CropWidth - 1) ) {
                if ( y < (CropHeight - 1) ) {
                    Face f;
                    
                    f.v0 = vtx;
                    f.v1 = vtx + 1;
                    f.v2 = vtx + CropWidth;
                    oPoints->PushFace ( f );

                    f.v0 = vtx + 1;
                    f.v1 = vtx + CropWidth;
                    f.v2 = vtx + CropWidth + 1;
                    oPoints->PushFace ( f );
                }
            }
        }
        oPoints->MoveToBarycenter ();
    }
}

void Camera::captureSingleMesh(std::string iPath)
{
    ParameterHandler* params = ParameterHandler::Instance();

    Image*      camImg = new Image ();
    Image*      camDepth = new Image ();
    PointSet*   pointCloud = new PointSet ();

    ReadFrame (
        camImg,
        camDepth,
        pointCloud
    );

    FileWriterServices* fws = FileWriterServices::Instance ();
    fws->StartSavingMesh ();
    
    fws->RegisterFrame (
        999,
        NULL,
        NULL,
        pointCloud,
        iPath
    );
}

void Camera::captureSingleFrame()
{
    ParameterHandler* params = ParameterHandler::Instance();

    Image*      camImg = new Image ();
    Image*      camDepth = new Image ();
    PointSet*   pointCloud = new PointSet ();

    ReadFrame (
        camImg,
        camDepth,
        pointCloud
    );

    //QThread* thread = new QThread();
    //Writer* worker = new Writer(m_nbFrames++, camImg, camDepth, pointCloud);
    //worker->moveToThread (thread);
    //connect(thread, SIGNAL(started()), worker, SLOT(write()));
    //connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
    //connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
    //connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    //thread->start();
    FileWriterServices* fws = FileWriterServices::Instance ();
    if ( m_nbFrames == 0 ) {
        fws->StartCapture ();
    }
    fws->RegisterFrame (
        m_nbFrames++,
        camImg,
        camDepth,
        pointCloud,
        Config::OutputPath() + "CapturedFrames/"
    );

    if ( m_nbFrames == params->GetNumCaptureFrames() ) {
        params->SetCaptureMode ( false );
        m_nbFrames = 0;
    }
}

/*!
 *  \brief  Creates the help screen during the capture of images. Use the Key '?'
 *          to call the menu
 */
void Camera::drawHelpScreen()
{
	int nXStartLocation = GL_WIN_SIZE_X/8;
	int nYStartLocation = GL_WIN_SIZE_Y/5;
	int nXEndLocation = GL_WIN_SIZE_X*7/8;
	int nYEndLocation = GL_WIN_SIZE_Y*4/5;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);		
	glBegin(GL_QUADS);
	glColor4f(0, 0, 0, 0.8);
	glVertex2i(nXStartLocation, nYStartLocation);
	glVertex2i(nXStartLocation, nYEndLocation);
	glVertex2i(nXEndLocation, nYEndLocation);
	glVertex2i(nXEndLocation, nYStartLocation);
	glEnd();

	glDisable(GL_BLEND);

	// set color to red
	glColor3f(1, 0, 0);

	// leave some margins
	nYStartLocation += 30;
	nXStartLocation += 30;

	// print left pane
	int nXLocation = nXStartLocation;
	int nYLocation = nYStartLocation;
	printHelp(nXLocation, &nYLocation);
}


/*!
 *  \brief  Create the content of the help screen
 */
void Camera::printHelp(int nXLocation, int* pnYLocation)
{
	int nYLocation = *pnYLocation;
	int nCount = 6;
	unsigned char aKeys[6];
	const char* aDescs[6];

        /* List of keys */
        aKeys[0] = 27;
        aDescs[0] = "Quit";

        aKeys[1] = 'p';
        aDescs[1] = "Save frame";

        aKeys[2] = '1';
        aDescs[2] = "Kinect: Overlay Mode";

        aKeys[3] = '2';
        aDescs[3] = "Kinect: Depth Mode";

        aKeys[4] = '3';
        aDescs[4] = "Kinect: Image Mode";

        aKeys[5] = '?';
        aDescs[5] = "Close help window";
        /* END List of keys */

	glColor3f(0, 1, 0);
	glRasterPos2i(nXLocation, nYLocation);
	nYLocation += 30;

	for (int i = 0; i < nCount; ++i, nYLocation += 35)
	{
		char buf[256];
		switch (aKeys[i])
		{
		case 27:
			sprintf(buf, "Esc");
			break;
		default:
			sprintf(buf, "%c", aKeys[i]);
			break;
		}

		glColor3f(1, 1, 1);
		glRasterPos2i(nXLocation, nYLocation);
		glPrintString(GLUT_BITMAP_TIMES_ROMAN_24, buf);

                sprintf(buf, "%s", aDescs[i]);

		glColor3f(0.5, 0.5, 1);
		glRasterPos2i(nXLocation + 40, nYLocation);
		glPrintString(GLUT_BITMAP_TIMES_ROMAN_24, buf);
	}

	*pnYLocation = nYLocation + 40;
}

/*!
 *  \brief  Print a string into the screen with a given font
 */
void Camera::glPrintString(void *font, const char *str)
{
	int i,l = strlen(str);

	for(i=0; i<l; i++)
	{
		glutBitmapCharacter(font,*str++);
	}
}
