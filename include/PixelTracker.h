#ifndef _PIXELTRACKER_H_
#define _PIXELTRACKER_H_

#include <string>
#include <Eigen/Dense>
#include <math.h>     
#include "ImagePyramid.h"

class Image;
class TrackInfo;

class PixelTracker {
private:
    unsigned int                m_referenceId;
    unsigned int                m_targetId;
    Image*                      m_refImage;
    int                         m_winHeight;
    int                         m_winWidth;
    int                         m_nbhHeight;
    int                         m_nbhWidth;
    float                       m_rejectionTreshold;
    
    unsigned int                m_refId;
    unsigned int                m_tarId;
    ImagePyramid                m_refImgPyr;
    ImagePyramid                m_refDepPyr;
    ImagePyramid                m_tarImgPyr;
    ImagePyramid                m_tarDepPyr;
    ImagePyramid                m_dispX;
    ImagePyramid                m_dispY;

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
    /* GDB Functions */
    void displayDisparityMapX();
    
    /* GDB Functions */
    void displayDisparityMapY();

    void SetReference (
        const unsigned int&     iReferenceId,
        const Image*            iReferenceImage,
        const Image*            iReferenceDepth
    );
    void SetTarget (
        const unsigned int&     iTargetId,
        const Image*            iTargetImage,
        const Image*            iTargetDepth
    );
    void Track ();
    void PyramidTrack (
        const unsigned int&     iLevel
    );
    void PyramidMatch (
        const unsigned int&     iLevel
    );
    void PyramidRematch (
        const unsigned int&     iLevel
    );
    void PyramidSmooth (
        const unsigned int&     iLevel
    );
    void PyramidOrdering (
        const unsigned int&     iLevel
    );
    void PyramidUniqueness (
        const unsigned int&     iLevel
    );
    void PyramidRefine (
        const unsigned int&     iLevel
    );
};

#endif // _PIXELTRACKER_H_
