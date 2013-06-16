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

public:
    ImagePyramid (
        const Image*        iSource,
        const unsigned int& iSamplingFactor=2
    );
    ~ImagePyramid ();

    void Export (
        const std::string&  iFilename
    ) const;
    const unsigned int& GetSamplingFactor () const;
    const unsigned int& GetNumLevels () const;
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
};

#endif
