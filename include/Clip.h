#ifndef _CLIP_H_
#define _CLIP_H_

class Frame;

/*
  This class will contain a set of frames, 
  where the first and last one are anchor frames
*/
class Clip {
private:
    unsigned int    m_startFrame;
    unsigned int    m_endFrame;

public:

    Clip ();
    ~Clip ();

    Clip (
        const unsigned int& iStartFrame,
        const unsigned int& iEndFrame
    );
    
    static void CreateDisplacementMaps (
        const unsigned int&     iRefFrameId,
        const unsigned int&     iTarFrameId,
        Frame*                  iRefFrame,
        Frame*                  iTarFrame
    );

};

#endif // _CLIP_H_
