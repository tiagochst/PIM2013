#ifndef _TRACKINFO_H_
#define _TRACKINFO_H_

#include <Eigen/Dense>

struct MatchDescriptor {
    bool            m_matched;
    float           m_score;
    int    m_xCoord;
    int    m_yCoord;

    MatchDescriptor ()
        :   m_matched (false), m_score (1.0f), m_xCoord(0u), m_yCoord(0u)
    {}
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

    MatchDescriptor& GetMatchData (
        const unsigned int&     iRefX,
        const unsigned int&     iRefY
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
