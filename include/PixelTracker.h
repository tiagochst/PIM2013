#ifndef _PIXELTRACKER_H_
#define _PIXELTRACKER_H_

#include <string>

class Image;
class TrackInfo;

class PixelTracker {
private:
    unsigned int                m_referenceId;
    Image*                      m_refImage;
    unsigned int                m_winHeight;
    unsigned int                m_winWidth;
    unsigned int                m_nbhHeight;
    unsigned int                m_nbhWidth;
    float                       m_rejectionTreshold;
    TrackInfo*                  m_trackInfo;

public:
    PixelTracker (
        const unsigned int&     iReferenceId
    );
    ~PixelTracker ();
 
    void SetReference (
        const unsigned int&     iReferenceId
    );
    void SetUp (
        const unsigned int&     iWindowWidth,
        const unsigned int&     iWindowHeight,
        const unsigned int&     iNeighbourhoodWidth,
        const unsigned int&     iNeighbourhoodHeight,
        const float&            iRejectionTreshold
    );
    void Reset ();
    void SetRejectionTreshold (
        const float&            iRejectionTreshold
    );
    void TrackWithRegulation (
        const unsigned int&     iTargetFrameId
    );
    void TrackWithRegulation (
        const unsigned int&     iTargetFrameId,
        const Image*            iTarget
    );
    void Track (
        const unsigned int&     iTargetFrameId
    );
    void Track (
        const unsigned int&     iTargetFrameId,
        Image*                  iTarget
    );
    void Rematch (
        const unsigned int&     iTargetFrameId,
        const Image*            iTarget
    ) const;
    void Export (
        const std::string&      iFilename
    ) const;
};

#endif // _PIXELTRACKER_H_
