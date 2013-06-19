#include <iostream>
#include "ImagePyramid.h"
#include "Image.h"

ImagePyramid::ImagePyramid ()
    :   m_levels (),
        m_samplingFactor ( 0 ),
        m_numLevels ( 0 )
{}
ImagePyramid::ImagePyramid (
    const Image*            iSource,
    const unsigned int&     iSamplingFactor
)   :   m_levels (),
        m_samplingFactor ( iSamplingFactor ),
        m_numLevels ( 0 )
{
    Assign ( iSource );
}

void ImagePyramid::Assign (
    const Image*            iBase
) {
    Clear ();

    // Copies the original image to the base of the pyramid.
    Image* img = new Image ();
    (*img) = (*iBase);
    m_levels.push_back ( img );
    m_numLevels++;
    std::cout << img->GetWidth () << std::endl;

    Image* newImg = (Image*)0x0;
    while ( img->GetWidth () > 100 ) {
        img = img->SubSample ( m_samplingFactor );
        m_levels.push_back ( newImg = img->Filter () );
        delete img;
        img = newImg;
        std::cout << img->GetWidth () << std::endl;
        m_numLevels++;
    }
}

ImagePyramid::~ImagePyramid ()
{
    Clear ();
}

void ImagePyramid::Clear ()
{
    for ( unsigned int i = 0; i < m_numLevels; i++ ) {
        if ( m_levels[i] ) {
            delete m_levels[i];
            m_levels[i] = (Image*)0x0;
        }
    }
    m_levels.clear ();
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

    for ( unsigned int i = 0; i < m_numLevels; i++ ) {
        std::stringstream fn;

        fn << filename << "_l" << i << pgm;
        m_levels[i]->CreateAsciiPgm ( fn.str () );
    }
}

void ImagePyramid::SetSamplingFactor (
    const unsigned int&     iSamplingFactor
) {
    m_samplingFactor = iSamplingFactor;
}
const unsigned int& ImagePyramid::GetSamplingFactor ()
const {
    return m_samplingFactor;
}

const unsigned int& ImagePyramid::GetNumLevels ()
const {
    return m_numLevels;
}
void ImagePyramid::SetNumLevels (
    const unsigned int& iNumLevels
) {
    Clear ();
    m_levels.resize ( iNumLevels );
    for ( unsigned int i = 0; i < iNumLevels; i++ ) {
        m_levels[i] = (Image*)0x0;
    }
    m_numLevels = iNumLevels;
}
const Image* ImagePyramid::Top () const {
    return m_levels[m_numLevels - 1];
}
Image* ImagePyramid::Top ()
{
    return m_levels[m_numLevels - 1];
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

void ImagePyramid::SetLevel (
    const unsigned int& iIndex,
    Image*              iLevelData
) {
    if ( m_levels[iIndex] != (Image*)0x0 ) {
        delete m_levels[iIndex];
    }
    m_levels[iIndex] = iLevelData;
}


