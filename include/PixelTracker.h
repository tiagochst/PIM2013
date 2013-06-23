#ifndef _PIXELTRACKER_H_
#define _PIXELTRACKER_H_

#include <string>
#include <Eigen/Dense>
#include <math.h>     
#include "ImagePyramid.h"

class Image;
class TrackInfo;
class PointSet;

class PixelTracker {
private:
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
    Eigen::MatrixXf             m_displacementX;
    Eigen::MatrixXf             m_displacementY;
    Eigen::MatrixXf             m_displacementZ;

public:
    PixelTracker ();
    ~PixelTracker ();
 
    void SetReference (
        const unsigned int&     iReferenceId
    );
    void SetRejectionTreshold (
        const float&            iRejectionTreshold
    );
    void Export (
        const std::string&      iFilename
    ) const;

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
    void PyramidRefineStep0 (
        const unsigned int&     iLevel
    );
    void PyramidRefine (
        const unsigned int&     iLevel
    );
    void ExportPyramidLevel (
        const unsigned int&     iLevel,
        const std::string&      iFilename
    ) const; 
    void CalculateMotionField (
        PointSet*     iRefMesh,
        PointSet*     iTarMesh
    );
};

#endif // _PIXELTRACKER_H_
