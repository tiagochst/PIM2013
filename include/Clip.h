#ifndef _CLIP_H_
#define _CLIP_H_

class Frame;
class PointSet;

/*
  This class will contain a set of frames, 
  where the first and last one are anchor frames
*/
class Clip {
private:
    unsigned int    m_startFrame;
    unsigned int    m_endFrame;
    unsigned int    m_currentFrame;
    bool            m_stopped;

public:

    Clip ();
    ~Clip ();

    Clip (
        const unsigned int& iStartFrame,
        const unsigned int& iEndFrame
    );
    
    const bool IsPlaying () const;

    void Play (
        const unsigned int& iRefId,
        PointSet&           iMesh
    );

    static void CreateDisplacementMaps (
        const unsigned int&     iRefFrameId,
        const unsigned int&     iTarFrameId,
        Frame*                  iRefFrame,
        Frame*                  iTarFrame
    );

};

#endif // _CLIP_H_
