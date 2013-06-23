#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
#include "PNMImageDefs.h"
#include "PNMImage.h"
#include "PPMImage.h"

PPMImage::PPMImage ()
    :   PNMImage (),
        m_pixmap ( 0x0 ),
        m_maxValue ( 1u )
{}
PPMImage::~PPMImage ()
{
    ClearPixmap ();
}
void PPMImage::ResetDimensions (
    const unsigned int&     iWidth,
    const unsigned int&     iHeight
) {
    ClearPixmap ();
    PNMImage::ResetDimensions (
        iWidth,
        iHeight
    );
    AllocatePixmap ();
}
const unsigned int& PPMImage::GetMaxValue ()
const {
    return m_maxValue;
}
void PPMImage::SetMaxValue (
    const unsigned int& iMaxValue
) {
    m_maxValue = iMaxValue;
}
#include <cmath>
const unsigned int PPMImage::GetChannelValue (
    const unsigned int&     iRow,
    const unsigned int&     iCol,
    const unsigned int&     iChannel
) const {
    const RGBColor& c = m_pixmap[iRow][iCol];
    if ( iChannel & RED ) {
        return c.r;
    } else if ( iChannel & BLUE ) {
        return c.b;
    } else if ( iChannel & GREEN ) {
        return c.g;
    } else {
        float grey  = 0.2126f * c.r
                    + 0.7152f * c.g
                    + 0.0722f * c.b;
        if ( iChannel & BIT ) {
            return ( grey >= ( 0.5f * m_maxValue ) ) ? 0u : 1u;
        } else {
            return nearbyint (grey);
        }
    }
}
void PPMImage::SetChannelValue (
    const unsigned int&     iRow,
    const unsigned int&     iCol,
    const unsigned int&     iChannel,
    const unsigned int&     iValue
) {
    RGBColor& c = m_pixmap[iRow][iCol];
    if (
        iChannel & BIT
    ) {
        c.r = c.g = c.b = ( 1.0f - iValue ) * m_maxValue;
    } else if (
        iChannel & GREY
    ) {
        c.r = c.g = c.b = iValue;
    } else {
        if ( iChannel & RED ) {
            c.r = iValue;
        }
        if ( iChannel & BLUE ) {
            c.b = iValue;
        }
        if ( iChannel & GREEN ) {
            c.g = iValue;
        }
    }
}
void PPMImage::WriteToFile (
    const std::string&      iFilename,
    const unsigned int&     iMode
) const {
    std::ofstream outFile (
        iFilename.c_str (),
        std::fstream::out | std::fstream::binary
    );

    std::vector<std::string> comments;
    comments.push_back ( "# Created by PIM2013" );
    if ( !( iMode & PIXMAP ) ) {
        comments.push_back ( "# Automatically converted from PPM" );
    }

    std::string version;
    if ( iMode & BINARY ) {
        if ( iMode & PIXMAP ) {
            version = "P6";
        } else if ( iMode & GREYMAP ) {
            version = "P5";
        } else if ( iMode & BITMAP ) {
            std::cerr << iFilename << ": PBM files are ascii only" << std::endl;

            outFile.close ();
            return;
        }
    } else {
        if ( iMode & PIXMAP ) {
            version = "P3";
        } else if ( iMode & GREYMAP ) {
            version = "P2";
        } else if ( iMode & BITMAP ) {
            version = "P1";
        }
    }
    outFile << version << std::endl;
    for ( unsigned int i = 0; i < comments.size (); i++ ) {
        outFile << comments[i] << std::endl;
    }
    outFile << Width () << " " << Height () << std::endl;
    outFile << m_maxValue << std::endl;

    for ( unsigned int i = 0; i < Height (); i++ ) {
        for ( unsigned int j = 0; j < Width (); j++ ) {
            if ( iMode & PIXMAP ) {
                unsigned int    red     = GetChannelValue ( i, j, RED );
                unsigned int    green   = GetChannelValue ( i, j, GREEN );
                unsigned int    blue    = GetChannelValue ( i, j, BLUE );
                if ( iMode & BINARY ) {
                    unsigned char   cred    = (red   * 255) / m_maxValue;
                    unsigned char   cgreen  = (green * 255) / m_maxValue;
                    unsigned char   cblue   = (blue  * 255) / m_maxValue;
                    outFile.write ( (char*)&cred  , 1 );
                    outFile.write ( (char*)&cgreen, 1 );
                    outFile.write ( (char*)&cblue , 1 );
                } else {
                    outFile << red << " " << green << " " << blue << std::endl;
                }
            } else if ( iMode & GREYMAP ) {
                unsigned int    grey    = GetChannelValue ( i, j, GREY );

                if ( iMode & BINARY ) {
                    unsigned char   cgrey   = (grey * 255) / m_maxValue; 
                    outFile.write ( (char*)&cgrey , 1 );
                } else {
                    outFile << grey << std::endl;
                }
            } else if ( iMode & BITMAP ) {
                unsigned int bit    = GetChannelValue ( i, j, BIT );

                outFile << bit << std::endl;
            }
        }
    }
    outFile.close ();
}
void PPMImage::AllocatePixmap ()
{
    m_pixmap = new RGBColor*[Height ()];
    for ( unsigned int i = 0; i < Height (); i++ ) {
        m_pixmap[i] = new RGBColor[Width ()];
        memset ( m_pixmap[i], 0x0, Width () * sizeof ( RGBColor ) );
    }
}
void PPMImage::ClearPixmap ()
{
    if ( m_pixmap ) {
        for ( unsigned int i = 0; i < Height (); i++ ) {
            if ( m_pixmap[i] ) {
                delete[] m_pixmap[i];
                m_pixmap[i] = (RGBColor*)0x0;
            }
        }
        delete[] m_pixmap;
        m_pixmap = (RGBColor**)0x0;
    }
}
