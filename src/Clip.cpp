#include "Clip.h"

#include "Frame.h"
#include "Tools.h"
#include "Config.h"
#include "PixelTracker.h"

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
        m_endFrame ( iEndFrame )
{
    std::string prefix = Config::FramesPath() + "f";
    
    Frame* f1 = new Frame ();
    f1->LoadFromFile ( prefix + Int2Str(m_startFrame) + "/" );
    for ( unsigned int frame = m_startFrame; frame < m_endFrame; frame++ ) {
        
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
        iRefFrame->GetTexture (),
        iRefFrame->GetDepthMap ()
    );
    
    pt.Track ();
    pt.Calculate3DDisplacements (
        iRefFrame->GetMesh (),
        iTarFrame->GetMesh ()
    );

    pt.Export ( Config::FramesPath() + "f" + Int2Str ( iRefFrameId ) 
                    + "/track/" + Int2Str ( iTarFrameId ) + "/" );
}


