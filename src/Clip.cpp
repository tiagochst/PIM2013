#include "Clip.h"

Clip::Clip()
{
}

Clip::~Clip()
{

    for(int i = 0; i < m_frames.size(); i++)
        {
            delete m_frames.at(i);
        }
    m_frames.clear();
}

/* 
   Receives first and last frame (both anchor frames),
   Insert Frame in the vector
*/
Clip::Clip(int iFrameStart,int iFrameEnd)
{
    std::string prefix = Config::OutputPath() + "CapturedFrames/";
    
    for(int frame = iFrameStart; frame <=iFrameEnd ; frame++){
        
        createDisplacemenMaps(frame);
        
        PointSet* ps = new PointSet ( prefix + "pointset_"+Int2Str(frame)+".ply");
        Image* tex = new Image ( prefix + "image_"+Int2Str(frame)+".pgm" );
        Image* dep = new Image ( prefix + "depth_"+Int2Str(frame)+".pgm" );
        PPMImage* disp = PPMImage::TryLoadFromFile ( prefix + "disparity_"+Int2Str(frame)+".ppm" ); 
        m_frames.push_back(new Frame (ps, tex, dep, disp));
    }
}

void Clip::createDisplacemenMaps(int frame){
    
    Image* refImg = new Image ( Config::FramesPath() + "image_" + Int2Str(frame)+".pgm"); 
    Image* tarImg = new Image ( Config::FramesPath() + "image_" + Int2Str(frame)+".pgm");
    Image* refDep = new Image ( Config::FramesPath() + "depth_" + Int2Str(frame+1)+".pgm");
    Image* tarDep = new Image ( Config::FramesPath() + "depth_" + Int2Str(frame+1)+".pgm");
    
    PixelTracker pt (0);
    pt.SetReference (
        frame,
        refImg,
        refDep
    );
    pt.SetTarget (
        frame+1,
        tarImg,
        tarDep
    );
    pt.Track ();
    
    pt.Export ( Config::FramesPath() + "disparity_" + Int2Str(frame) + ".ppm");
    
    delete refImg;
    delete tarImg;
    delete refDep;
    delete tarDep;
}


