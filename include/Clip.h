#ifndef _CLIP_H_
#define _CLIP_H_

#include "Frame.h"
#include <math.h>
#include <vector>
#include <stdlib.h> // RAND_MAX
#include "Config.h"
#include "Tools.h"
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
#include "PPMImage.h"

/*
  This class will contain a set of frames, 
  where the first and last one are anchor frames
*/
class Clip {

private:
    std::vector <Frame*> m_frames;

public:

    Clip ();
    ~Clip ();

    Clip(
        int iFrameStart,
        int iFrameEnd
    );
    
    void createDisplacemenMaps(
        int frame
    ); 

};

#endif // _CLIP_H_
