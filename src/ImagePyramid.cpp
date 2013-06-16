#include "ImagePyramid.h"
#include "Image.h"

ImagePyramid::ImagePyramid (
    const Image*            iSource,
    const unsigned int&     iSamplingFactor
)   :   m_levels (),
        m_samplingFactor ( iSamplingFactor )
{
    // Copies the original image to the base of the pyramid.
    Image* img = new Image ();
    (*img) = (*iSource);
    m_levels.push_back ( img );

    Image* newImg = (Image*)0x0;
    while ( img->GetWidth () > 100 ) {
        img = img->Filter ();
        m_levels.push_back ( newImg = img->SubSample ( iSamplingFactor ) );
        delete img;
        img = newImg;
    }
}

ImagePyramid::~ImagePyramid ()
{
    for ( unsigned int i = 0; i < m_levels.size (); i++ ) {
        if ( m_levels[i] ) {
            delete m_levels[i];
            m_levels[i] = (Image*)0x0;
        }
    }
}

void ImagePyramid::Export (
    const std::string&  iFilename
) const {
    std::string filename;
    std::string pgm (".pgm");
    if ( iFilename.find (pgm) == iFilename.size () - pgm.size () ) {
        filename.substr ( 0, iFilename.find (pgm) );
    } else {
        filename = iFilename;
    }

    for ( unsigned int i = 0; i < m_levels.size (); i++ ) {
        std::stringstream fn;

        fn << filename << "_l" << i << pgm;
        m_levels[i]->CreateAsciiPgm ( fn.str () );
    }
}

const unsigned int& ImagePyramid::GetSamplingFactor ()
const {
    return m_samplingFactor;
}

const unsigned int& ImagePyramid::GetNumLevels ()
const {
    return m_levels.size ();
}
const Image* ImagePyramid::Top () const {
    return m_levels[m_levels.size() - 1];
}
Image* ImagePyramid::Top ()
{
    return m_levels[m_levels.size() - 1];
}

const Image* ImagePyramid::Bottom () const {
    return m_levels[0];
}
Image* ImagePyramid::Bottom ()
{
    return m_levels[0];
}

const Image* ImagePyramid::operator[] (
    const unsigned int& iIndex
) const {
    return m_levels[iIndex];
}

Image* ImagePyramid::operator[] (
    const unsigned int& iIndex
) {
    return m_levels[iIndex];
}
