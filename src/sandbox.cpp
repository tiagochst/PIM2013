#include "PNMImage.h"
#include "PPMImage.h"
#include <QCoreApplication>
#include <QApplication>
#include "Window.h"
#include <QDir>
#include <QPixmap>
#include <QSplashScreen>
#include <QPlastiqueStyle>
#include <QCleanlooksStyle>
#include <string>
#include <iostream>
#include "MathUtils.h"
#include "Image.h"
#include "ImageBase.h"
#include "SubImage.h"
#include "PointSet.h"
#include "Camera.h"
#include "Config.h"
#include "Rectangle.h"
#include "ParameterHandler.h"
#include "PixelTracker.h"
#include "ImagePyramid.h"

extern void printColorChart (
    const int& width,
    const int& height,
    PPMImage& colorChart
);

int main ( void ) {
    Config::LoadConfigs(Config::RootPath() + "settings");

    static const std::string RES_PTSET_PATH(Config::ResourcesPath() + "PointSets/");
    static const std::string RES_IMG_PATH(Config::ResourcesPath() + "Images/");

    Image frame0i(Config::OutputPath () + "CapturedFrames/image_0.pgm");
    Image frame0d(Config::OutputPath () + "CapturedFrames/depth_0.pgm");
    Image frame1i(Config::OutputPath () + "CapturedFrames/image_1.pgm");
    Image frame1d(Config::OutputPath () + "CapturedFrames/depth_1.pgm");

    PixelTracker pixTrack (0);

    pixTrack.SetReference ( 0, &frame0i, &frame0d );
    pixTrack.SetTarget ( 1, &frame1i, &frame1d );
    pixTrack.Track ();
    pixTrack.Export ( Config::OutputPath() + "oi.pgm" );
    //
    //Image frame0(Config::OutputPath () + "CapturedFrames/image_0.pgm");
    //Image* filtered = frame0.Filter ();
    //filtered->CreateAsciiPgm ( Config::OutputPath () + "filtered.pgm" );
    //delete filtered;

    //Image* subsampled = frame0.SubSample ( 2 );
    //subsampled->CreateAsciiPgm ( Config::OutputPath () + "subsampled2.pgm" );
    //delete subsampled;

    //subsampled = frame0.SubSample ( 4 );
    //subsampled->CreateAsciiPgm ( Config::OutputPath () + "subsampled4.pgm" );
    //delete subsampled;
    //subsampled = (Image*)0x0;

    //ImagePyramid pyr;
    //pyr.SetSamplingFactor ( 2 );
    //pyr.Assign ( &frame0 );
    //pyr.Export ( Config::OutputPath () + "Pyramids/image_0" ); 

    ParameterHandler* params = ParameterHandler::Instance ();
    const unsigned int& wSize = params->GetWindowSize ();
    const unsigned int& nSize = params->GetNeighbourhoodSize ();
    //dispX.SetMaxGreyLevel ( wSize );
    //dispY.SetMaxGreyLevel ( wSize );

    PixelTracker pixTracker (0);
    pixTracker.SetUp (
        wSize,
        wSize,
        nSize,
        nSize,
        0.95
    );

    Image frame1(Config::OutputPath () + "CapturedFrames/image_1.pgm");
    pixTracker.Track (1, &frame1);
    pixTracker.Export ( Config::OutputPath () + "/TCSNorefinement_"+toString(0.95)+"n.ppm" );

    pixTracker.disparityRefinement (&frame1);
    pixTracker.Export ( Config::OutputPath () + "/TCSrefinement_"+toString(0.95)+"n.ppm" );

}
