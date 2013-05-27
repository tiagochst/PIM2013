#ifndef _CAMERA_H__
#define _CAMERA_H__

#include <XnOpenNI.h>
#include <XnLog.h>
#include <XnCodecIDs.h>
#include <XnCppWrapper.h>
#include <XnPlatform.h>
#include <GL/gl.h>
#include <XnTypes.h>
#include <XnOS.h>
#include <GL/glut.h>
#include <math.h>
#include <cassert>
#include <sstream> 
#include <string> 
#include "Image.h"
#include "PointSet.h"

//----------------------------------------------------------
// Defines
//----------------------------------------------------------
#define MAX_DEPTH 10000
#define SAMPLE_XML_PATH "KinectConfig.xml"
#define GL_WIN_SIZE_X	1280
#define GL_WIN_SIZE_Y	1024
#define TEXTURE_SIZE	512
#define CAPTURED_FRAMES_DIR_NAME "CapturedFrames"
#define DEFAULT_DISPLAY_MODE	DISPLAY_MODE_DEPTH
#define MIN_NUM_CHUNKS(data_size, chunk_size)	((((data_size)-1) / (chunk_size) + 1))
#define MIN_CHUNKS_SIZE(data_size, chunk_size)	(MIN_NUM_CHUNKS(data_size, chunk_size) * (chunk_size))

//---------------------------------------------------------------
// Macros
//---------------------------------------------------------------
#define CHECK_RC(rc, what)					\
  if (rc != XN_STATUS_OK)					\
    {								\
      printf("%s failed: %s\n", what, xnGetStatusString(rc));	\
      return rc;						\
    }

enum DisplayModes_e
{
	DISPLAY_MODE_OVERLAY,
	DISPLAY_MODE_DEPTH,
	DISPLAY_MODE_IMAGE
};

class Camera
{
    int m_help; //! Show help screen

public:
	// Singleton
	static Camera& CreateInstance(xn::Context& context);
	static void DestroyInstance(Camera& instance);

	virtual XnStatus Init(int argc, char **argv);
	virtual XnStatus Run();	//Does not return
	void captureSingleFrame();
	std::string Int2Str(int nb);
protected:
	Camera(xn::Context& context);
	virtual ~Camera();

	virtual int Display();
	virtual void DisplayPostDraw(){};
	// Overload to draw over the screen image

	virtual void OnKey(unsigned char key, int x, int y);

	virtual XnStatus InitOpenGL(int argc, char **argv);
	void InitOpenGLHooks();

	static Camera& Instance();

	xn::Context&		m_rContext;
	xn::DepthGenerator	m_depth;
	xn::ImageGenerator	m_image;
	xn::IRGenerator	        m_IR;

	static Camera*	sm_pInstance;

	void ScalePoint(XnPoint3D& point);

	/*
	  UI: Help Screen
	  Draws a new window with commands 
	*/
	void glPrintString(void *font, const char *str);
	void printHelp(int nXLocation, int* pnYLocation);
	void drawHelpScreen();
	
private:
	// GLUT callbacks
	static void glutIdle();
	static void glutDisplay();
	static void glutKeyboard(unsigned char key, int x, int y);

	float			m_pDepthHist[MAX_DEPTH];
	XnRGB24Pixel*		m_pTexMap;
	unsigned int		m_nTexMapX;
	unsigned int		m_nTexMapY;
	DisplayModes_e		m_eViewState;
	unsigned int            m_nbFrames;
	xn::DepthMetaData	m_depthMD;  
	xn::ImageMetaData	m_imageMD;  
	xn::IRMetaData          m_irMD; 

};

#endif //NI_SIMPLE_VIEWER_H__
