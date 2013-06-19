#include <fstream>
#include <iostream>
#include <sstream>
#include "PNMImageDefs.h"
#include "PNMImage.h"
#include "PPMImage.h"

PNMImage::PNMImage ()
    :   m_width ( 0 ),
        m_height ( 0 )
{}
PNMImage::~PNMImage () {}

const unsigned int& PNMImage::Width ()
const {
    return m_width;
}
const unsigned int& PNMImage::Height ()
const {
    return m_height;
}

void PNMImage::ResetDimensions (
    const unsigned int&     iWidth,
    const unsigned int&     iHeight
) {
    m_width = iWidth;
    m_height = iHeight;
}
void PNMImage::LoadFromFile (
    const std::string&  iFilename
) {
    std::ifstream   input (
        iFilename.c_str (), 
        std::ifstream::in | std::ifstream::binary
    );

    std::stringstream ss;
    unsigned int mode = 0xFFFFu;
    unsigned int width, height, maxValue;
    
    if (
            input.is_open()
        &&  input.good()
    ) {
        /* First line : version of pnm file*/
        std::string version;
        getline (
            input,
            version
        );
        std::cout << version << std::endl;

        if (
            version.compare( "P1" ) == 0 
        ) {
            mode = ( BITMAP | ASCII );
            std::cerr << "TO BE DONE" << std::endl;
            input.close ();
            return;
        } else if (
            version.compare( "P2" ) == 0 
        ) {
            mode = ( GREYMAP | ASCII );
            std::cerr << "TO BE DONE" << std::endl;
            input.close ();
            return;
        } else if (
            version.compare( "P3" ) == 0 
        ) {
            mode = ( PIXMAP | ASCII );
        } else if (
            version.compare( "P5" ) == 0 
        ) {
            mode = ( GREYMAP | BINARY );
            std::cerr << "TO BE DONE" << std::endl;
            input.close ();
            return;
        } else if (
            version.compare( "P6" ) == 0
        ) {
            mode = ( PIXMAP | BINARY );
        } else {
            std::cerr   << "PNM version error " 
                        << iFilename.c_str()
                        << " Version: "
                        << version 
                        << std::endl;

        }


        /* Second line : comment */
        std::string comment;
        getline ( input, comment );
        std::cout << comment << std::endl;
        input >> width >> height >> maxValue;
        std::cout << width << " " << height << std::endl << maxValue << std::endl;

        //bool widthRead      = true;
        //bool heightRead     = true;
        //bool maxValueRead   = !( mode & BITMAP );
        //do {
        //    std::string inputLine;
        //    getline (
        //        input,
        //        inputLine
        //    );
        //    std::stringstream lineStream ( inputLine );
        //    std::cout << inputLine << std::endl;

        //    while (
        //            !lineStream.eof ()
        //        &&  ( widthRead || heightRead || maxValueRead )
        //    ) {
        //        char peeked = lineStream.peek ();
        //        while ( peeked == '#' || peeked == ' ' ) {
        //            lineStream >> peeked;
        //            peeked = lineStream.peek ();
        //        }
        //        if ( widthRead && !lineStream.eof () ) {
        //            lineStream >> width;
        //            std::cout << width << std::endl;
        //            widthRead = false;
        //        }
        //        if ( heightRead && !lineStream.eof () ) {
        //            lineStream >> height;
        //            std::cout << height << std::endl;
        //            heightRead = false;
        //        }
        //        if ( maxValueRead && !lineStream.eof () ) {
        //            lineStream >> maxValue;
        //            std::cout << maxValue << std::endl;
        //            maxValueRead = false;
        //        }
        //    }
        //} while ( widthRead || heightRead || maxValueRead );

        SetMaxValue ( maxValue );
        ResetDimensions ( width, height );
        if ( mode & BINARY ) {
            mode = mode & ~BINARY;

            for ( unsigned int i = 0; i < height; i++ ) {
                for ( unsigned int j = 0; j < width; j++ ) {
                    switch ( mode ) {
                    case PIXMAP:
                        unsigned char r, g, b;
                        input.read ( (char*)&r, 1 );
                        input.read ( (char*)&g, 1 );
                        input.read ( (char*)&b, 1 );

                        std::cout   << (unsigned int)r << " " 
                                    << (unsigned int)g << " " 
                                    << (unsigned int)b << std::endl;
                        SetChannelValue ( i, j,   RED, (unsigned int)r );
                        SetChannelValue ( i, j, GREEN, (unsigned int)g );
                        SetChannelValue ( i, j,  BLUE, (unsigned int)b );
                        break;

                    case BITMAP:
                        std::cerr << "BITMAPS CANNOT BE READ IN BINARY MODE" << std::endl;
                        input.close ();
                        return;

                    case GREYMAP:
                        unsigned int grey;
                        input.read ( (char*)&grey, sizeof ( grey ) );

                        std::cout << grey << std::endl;
                        SetChannelValue ( i, j, GREY, grey );
                        break;
                    }
                }
            }
        } else {
            ss << input.rdbuf();

            mode = mode & ~ASCII;
            for ( unsigned int i = 0; i < height; i++ ) {
                for ( unsigned int j = 0; j < width; j++ ) {
                    switch ( mode ) {
                        case PIXMAP:
                            RGBColor rgb;
                            ss >> rgb.r >> rgb.g >> rgb.b;

                            SetChannelValue ( i, j,   RED, rgb.r );
                            SetChannelValue ( i, j, GREEN, rgb.g );
                            SetChannelValue ( i, j,  BLUE, rgb.b );
                            break;

                        case BITMAP:
                            std::cerr << "TO BE DONE" << std::endl;
                            input.close ();
                            return;

                        case GREYMAP:
                            unsigned int grey;
                            ss >> grey;

                            SetChannelValue ( i, j, GREY, grey );
                            break;
                    }
                }
            }
        }
        input.close ();
    } else {
        std::cout << iFilename << std::endl;
        std::cerr << ": File could not be opened" << std::endl; 
    }
}


