#include "TrackInfo.h"
#include "Image.h"
#include "PNMImage.h"
#include "PNMImageDefs.h"
#include "PPMImage.h"
#include "Config.h"
#include "Tools.h"
#include "ParameterHandler.h"

TrackInfo::TrackInfo (
    const unsigned int&     iReferenceFrameId,
    const unsigned int&     iTargetFrameId
)   :   m_refFrameId ( iReferenceFrameId ),
        m_targetFrameId ( iTargetFrameId ),
        m_matches ()
{}

void TrackInfo::SetDimensions (
    const unsigned int&     iWidth,
    const unsigned int&     iHeight
) {
    m_matches.resize (
        iHeight,
        iWidth
    );
}

MatchDescriptor& TrackInfo::GetMatchData (
    const unsigned int&     iRefX,
    const unsigned int&     iRefY
) {
    return m_matches ( iRefY, iRefX );
}

const MatchDescriptor& TrackInfo::GetMatchData (
    const unsigned int&     iRefX,
    const unsigned int&     iRefY
) const {
    return m_matches ( iRefY, iRefX );
}

void TrackInfo::SetMatch (
    const unsigned int&     iRefX,
    const unsigned int&     iRefY,
    const MatchDescriptor&  iMatchData
) {
    m_matches ( iRefY, iRefX ) = iMatchData;
}

void TrackInfo::CreateOutputImage (
    const std::string&      iFilename
) const {
    const unsigned int S_OFF_X = 0u;
    const unsigned int S_OFF_Y = 0u;
    const unsigned int X_OFF_X = m_matches.cols ();
    const unsigned int X_OFF_Y = 0u;
    const unsigned int Y_OFF_X = 0u;
    const unsigned int Y_OFF_Y = m_matches.rows ();
    const unsigned int Z_OFF_X = m_matches.cols ();
    const unsigned int Z_OFF_Y = m_matches.rows ();

    Image refDepth (Config::OutputPath () + "CapturedFrames/depth_" + Int2Str (m_refFrameId) + ".pgm");
    Image targetDepth (Config::OutputPath () + "CapturedFrames/depth_" + Int2Str (m_targetFrameId) + ".pgm");
    
    PPMImage output;
    output.SetMaxValue ( 255u );

    ParameterHandler* params = ParameterHandler::Instance ();
    const unsigned int wSz  = params->GetWindowSize ();
    const unsigned int rows = 2 * m_matches.rows ();
    const unsigned int cols = 2 * m_matches.cols ();
    output.ResetDimensions (
        cols,
        rows
    );

    float invWsz = 1.0f / (float)wSz;
    for (
        unsigned int i = 0;
        i < m_matches.rows ();
        i++
    ) {
        for (
            unsigned int j = 0;
            j < m_matches.cols ();
            j++
        ) {
            const MatchDescriptor& md = m_matches ( i, j );
            if ( md.m_score >= 0.0f ) {
                int dispX = (int)md.m_xCoord - (int)j;
                int dispY = (int)md.m_yCoord - (int)i;
                int dispZ = targetDepth.GetGreyLvl (md.m_yCoord, md.m_xCoord) - refDepth.GetGreyLvl ( i, j );

                float bX = ( dispX + 0.5f * wSz ) * invWsz;
                float bY = ( dispY + 0.5f * wSz ) * invWsz;
                float bZ = ( dispZ + 0.5f * wSz ) * invWsz;
                float gX = 1.0f - bX;
                float gY = 1.0f - bY;
                float gZ = 1.0f - bZ;

                output.SetChannelValue ( X_OFF_Y + i, X_OFF_X + j,   RED, 0u );
                output.SetChannelValue ( Y_OFF_Y + i, Y_OFF_X + j,   RED, 0u );
                output.SetChannelValue ( Z_OFF_Y + i, Z_OFF_X + j,   RED, 0u );
                output.SetChannelValue ( X_OFF_Y + i, X_OFF_X + j, GREEN, gX * 255.f );
                output.SetChannelValue ( Y_OFF_Y + i, Y_OFF_X + j, GREEN, gY * 255.f );
                output.SetChannelValue ( Z_OFF_Y + i, Z_OFF_X + j, GREEN, abs ( dispZ ) );
                output.SetChannelValue ( X_OFF_Y + i, X_OFF_X + j,  BLUE, bX * 255.f );
                output.SetChannelValue ( Y_OFF_Y + i, Y_OFF_X + j,  BLUE, bX * 255.f );
                output.SetChannelValue ( Z_OFF_Y + i, Z_OFF_X + j,  BLUE, abs ( dispZ ) );
                output.SetChannelValue ( S_OFF_Y + i, S_OFF_X + j,  GREY, md.m_score * 255.f );
            } else {
                output.SetChannelValue ( X_OFF_Y + i, X_OFF_X + j, RED, 255u );
                output.SetChannelValue ( Y_OFF_Y + i, Y_OFF_X + j, RED, 255u );
                output.SetChannelValue ( Z_OFF_Y + i, Z_OFF_X + j, RED, 255u );
                output.SetChannelValue ( S_OFF_Y + i, S_OFF_X + j, RED, 255u );
            }
        }
    }
    output.WriteToFile (
        iFilename,
        PIXMAP|BINARY
    );
}

