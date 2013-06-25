#include "Clip.h"

#include "Frame.h"
#include "Tools.h"
#include "Config.h"
#include "PixelTracker.h"
#include "PointSet.h"

Clip::Clip() {}

Clip::~Clip() {}

/* 
   Receives first and last frame (both anchor frames),
   Insert Frame in the vector
*/
Clip::Clip (
    const unsigned int& iStartFrame,
    const unsigned int& iEndFrame
)   :   m_startFrame ( iStartFrame ),
        m_endFrame ( iEndFrame ),
        m_currentFrame ( iStartFrame ),
        m_stopped ( true )
{
    std::string prefix = Config::FramesPath() + "f";
    
    Frame* f1 = new Frame ();
    f1->LoadFromFile ( prefix + Int2Str(m_startFrame) + "/" );

//    #pragma omp parallel for
    for ( unsigned int frame = m_startFrame; frame < m_endFrame; frame++ ) {
        std::cout << "Calculating frame " << frame << std::endl;
        
        Frame* f2 = new Frame ();
        f2->LoadFromFile ( prefix + Int2Str(frame + 1) + "/" );

        Clip::CreateDisplacementMaps ( frame, frame + 1, f1, f2 );
        Clip::CreateDisplacementMaps ( frame + 1, frame, f2, f1 );

        delete f1;
        f1 = f2;
        f2 = (Frame*)0x0;
    }
    delete f1;
    f1 = (Frame*)0x0;
}

const bool Clip::IsPlaying () const
{
    return !m_stopped;
}

void Clip::Play (
    const unsigned int& iRefId,
    PointSet&           iMesh
) {
    if ( m_stopped ) {
        m_stopped = false;
    }

    Frame f;
    f.LoadMotionField ( Config::FramesPath () + "f" + toString(m_currentFrame) + "/track/" + toString (m_currentFrame+1) + "/" );
    f.ApplyMotionField ( iMesh );

    m_currentFrame++;
    if ( m_currentFrame == m_endFrame ) {
        m_currentFrame = m_startFrame;
        m_stopped = true;
    }
}

void Clip::CreateDisplacementMaps (
    const unsigned int&     iRefFrameId,
    const unsigned int&     iTarFrameId,
    Frame*                  iRefFrame,
    Frame*                  iTarFrame
) {
    PixelTracker pt;

    pt.SetReference (
        iRefFrameId,
        iRefFrame->GetTexture (),
        iRefFrame->GetDepthMap ()
    );
    pt.SetTarget (
        iTarFrameId,
        iTarFrame->GetTexture (),
        iTarFrame->GetDepthMap ()
    );
    
    pt.Track ();
    pt.CalculateMotionField(
        iRefFrame->GetMesh (),
        iTarFrame->GetMesh ()
    );

    pt.Export ( Config::FramesPath() + "f" + Int2Str ( iRefFrameId ) 
                    + "/track/" + Int2Str ( iTarFrameId ) + "/" );
}


