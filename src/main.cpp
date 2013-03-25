#include <iostream>
#include "Image.h"
#include "PointSet.h"

int main(int argc, char** argv) {
    Image myImage("frame_20121108T103323.258153_rgb-ascci.pgm");		
    std::cout << "Hello World" << std::endl;
 
    PointSet psAscii, psBinary;    
    psAscii.LoadFromFile("frame000-ascii.ply");
    psBinary.LoadFromFile("frame000-brut.ply");

	return 0;
}
