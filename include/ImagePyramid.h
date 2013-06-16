#ifndef _IMAGEPYRAMID_H_
#define _IMAGEPYRAMID_H_

#include <vector>
#include <string>

class Image;

class ImagePyramid
{
private:
    std::vector<Image*>     m_levels;
    unsigned int            m_samplingFactor;
    unsigned int            m_numLevels;

public:
    ImagePyramid ();
    ImagePyramid (
        const Image*        iSource,
        const unsigned int& iSamplingFactor=2
    );
    ~ImagePyramid ();

    void Export (
        const std::string&  iFilename
    ) const;
    void SetSamplingFactor (
        const unsigned int&  iSamplingFactor
    );
    const unsigned int& GetSamplingFactor () const;
    const unsigned int& GetNumLevels () const;
    void SetNumLevels (
        const unsigned int& iNumLevels
    );
    const Image* Top () const;
    Image* Top ();

    const Image* Bottom () const;
    Image* Bottom ();
    
    const Image* operator[] (
        const unsigned int& iIndex
    ) const;

    Image* operator[] (
        const unsigned int& iIndex
    );

    void Assign (
        const Image*        iBase
    );
    void Clear ();
    void SetLevel (
        const unsigned int& iIndex,
        Image*              iLevelData
    );
};

#endif
