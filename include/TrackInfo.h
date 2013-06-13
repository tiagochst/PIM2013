#ifndef _TRACKINFO_H_
#define _TRACKINFO_H_

#include <Eigen/Dense>

struct MatchDescriptor {
    float           m_score;
    unsigned int    m_xCoord;
    unsigned int    m_yCoord;
};

using namespace Eigen;
typedef Matrix<MatchDescriptor,Dynamic,Dynamic>  MatrixXmd;

class TrackInfo {
private:
    unsigned int                m_refFrameId;
    unsigned int                m_targetFrameId;
    MatrixXmd                   m_matches;

public:
    TrackInfo (
        const unsigned int&     iReferenceFrameId,
        const unsigned int&     iTargetFrameId
    );

    void SetDimensions (
        const unsigned int&     iWidth,
        const unsigned int&     iHeight
    );

    const MatchDescriptor& GetMatchData (
        const unsigned int&     iRefX,
        const unsigned int&     iRefY
    ) const;

    void SetMatch (
        const unsigned int&     iRefX,
        const unsigned int&     iRefY,
        const MatchDescriptor&  iMatchData
    );

    void CreateOutputImage (
        const std::string&      iFilename
    ) const;
};

#endif // _TRACKINFO_H_
