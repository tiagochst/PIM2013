#ifndef _TOOLS_H_
#define _TOOLS_H_

#include <string>
#include <sstream>
#include <iomanip>
#include <Vec3D.hpp>
#include "PPMImage.h"
#include "Config.h"
#include "Color.h"

template<typename T>
inline std::string toString ( const T& val ) {
    std::stringstream ss;

    ss << val;

    return ss.str ();
}

inline std::string toString ( const float& val ) {
    std::stringstream ss;

    ss << std::setprecision(2) << val;

    return ss.str ();
}

inline std::string Int2Str(int nb){
    std::ostringstream ss;
    ss << nb;
    return ss.str();
}

inline void PrintColorChart (
    const int&          iChartWidth,
    const int&          iChartHeight,
    const std::string&  iFilename
) {
    const float maxDispX = iChartWidth  * 0.5f;   
    const float maxDispY = iChartHeight * 0.5f;
    const float maxDisp = sqrt ( maxDispX * maxDispX + maxDispY * maxDispY );

    PPMImage colorChart;
    colorChart.ResetDimensions ( iChartWidth, iChartHeight );
    colorChart.SetMaxValue ( 255u );
    Color c;
    Vec3Df redDir ( 1.0f, 0.0f, 0.0f );
    for ( unsigned int x = 0; x < iChartHeight; x++ ) {
        for ( unsigned int y = 0; y < iChartHeight; y++ ) {
            const float dX = x - maxDispX; 
            const float dY = y - maxDispY;

            Vec3Df disp ( dX, dY, 0.0f );
            const float value = disp.normalize () / maxDisp;

            const float hue = 180.0f * acos ( Vec3Df::dotProduct ( disp, redDir ) ) / M_PI; 
            if ( disp [1] < 0 ) {
                c.FromHSV ( 360.0f-hue, 1.0f, value ); 
            } else {
                c.FromHSV ( hue, 1.0f, value ); 
            }
            //std::cout << c.Red () << " " << c.Green () << " " << c.Blue () << std::endl;

            colorChart.SetChannelValue ( y, x,   RED, 255.f * c.Red () );
            colorChart.SetChannelValue ( y, x, GREEN, 255.f * c.Green () );
            colorChart.SetChannelValue ( y, x,  BLUE, 255.f * c.Blue () );
        }
    }
    colorChart.WriteToFile ( iFilename, PIXMAP | BINARY );
}

#endif // _TOOLS_H_
