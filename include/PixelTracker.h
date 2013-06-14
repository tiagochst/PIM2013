#ifndef _PIXELTRACKER_H_
#define _PIXELTRACKER_H_

#include <string>
#include <Eigen/Dense>
#include <math.h>     

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
    TrackInfo*                  m_forwardTrack;
    TrackInfo*                  m_backwardsTrack;
    Eigen::MatrixXf             m_disparityMapX;
    Eigen::MatrixXf             m_disparityMapY;
    Image*                      m_depthMap;

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
    unsigned int Match (
        const unsigned int&     iUnmatchedPixels,
        Image*                  iTarget
    );
    void Export (
        const std::string&      iFilename
    ) const;

    /* Refinement functions*/ 
    float computeDpX(
        float e_1, float e0, float e1,
        CartesianCoordinate p,CartesianCoordinate q
    );
    float computeDpY(
        float e_1, float e0, float e1,
        CartesianCoordinate p,CartesianCoordinate q
    );

    float computeDsX(
        CartesianCoordinate p
    );
    float computeDsY(
        CartesianCoordinate p
    );
    float computeWp(
        float e_1, float e0, float e1
    );
    float getWs();

    /* 
       Mode 0 = X displacement
       Mode 1 = Y displacement
    */
    float computeUs(
        CartesianCoordinate p,
        Image* iTarget,
        int   iMode
    );

    float computeDprimeX(
        float e_1, float e0, float e1,
        CartesianCoordinate p, CartesianCoordinate q,
        Image* iTarget
    );
    float computeDprimeY(
        float e_1, float e0, float e1,
        CartesianCoordinate p, CartesianCoordinate q,
        Image* iTarget
    );
    void  disparityRefinement(
        Image* iTarget    
    );
  
};

#endif // _PIXELTRACKER_H_
