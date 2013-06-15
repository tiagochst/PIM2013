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

extern void printColorChart (
    const int& width,
    const int& height,
    PPMImage& colorChart
);

int main ( void ) {
    PPMImage colorChart;
    printColorChart (
        30, 30, colorChart
    );
    Config::LoadConfigs(Config::RootPath() + "settings");

    static const std::string RES_PTSET_PATH(Config::ResourcesPath() + "PointSets/");
    static const std::string RES_IMG_PATH(Config::ResourcesPath() + "Images/");

    Image refImg, anchorImg; 

    Image frame0(Config::OutputPath () + "CapturedFrames/image_14.pgm");
    Image frame1(Config::OutputPath () + "CapturedFrames/image_15.pgm");
    Image dispX( frame0.GetWidth(), frame0.GetHeight(), 255 );
    Image dispY( frame0.GetWidth(), frame0.GetHeight(), 255 );

    ParameterHandler* params = ParameterHandler::Instance ();
    const unsigned int& wSize = params->GetWindowSize ();
    const unsigned int& nSize = params->GetNeighbourhoodSize ();
    dispX.SetMaxGreyLevel ( wSize );
    dispY.SetMaxGreyLevel ( wSize );

    PixelTracker pixTracker (14);
    pixTracker.SetUp (
                      wSize,
                      wSize,
                      nSize,
                      nSize,
                      0.95
    );

    pixTracker.Track ( 15);

    pixTracker.Export ( Config::OutputPath () + "/TCSNorefinement_"+toString(0.95)+"n.ppm" );

    pixTracker.disparityRefinement (&frame1);
    pixTracker.Export ( Config::OutputPath () + "/TCSrefinement_"+toString(0.95)+"n.ppm" );

    dispX.CreateAsciiPgm(Config::OutputPath() + "TrackingTestx.pgm");
    dispY.CreateAsciiPgm(Config::OutputPath() + "TrackingTesty.pgm");

    //PPMImage binImage;
    //PPMImage asciiImage;

    //PPMImage binImage;
    //PPMImage asciiImage;

    //binImage.LoadFromFile (
    //    "./in/ppm_bin.ppm"
    //);
    //binImage.WriteToFile (
    //    "./out/bin_ppm_to_bin_ppm.ppm",
    //    PIXMAP | BINARY
    //);
    //binImage.WriteToFile (
    //    "./out/bin_ppm_to_bin_pgm.pgm",
    //    GREYMAP | BINARY
    //);
    //binImage.WriteToFile (
    //    "./out/bin_ppm_to_ascii_ppm.ppm",
    //    PIXMAP | ASCII
    //);
    //binImage.WriteToFile (
    //    "./out/bin_ppm_to_ascii_pgm.pgm",
    //    GREYMAP | ASCII
    //);
    //binImage.WriteToFile (
    //    "./out/bin_ppm_to_ascii_pbm.pbm",
    //    BITMAP | ASCII
    //);

    //asciiImage.LoadFromFile (
    //    "./ppm_ascii.ppm"
    //);
    //asciiImage.WriteToFile (
    //    "./out/ascii_ppm_to_bin_ppm.ppm",
    //    PIXMAP | BINARY
    //);
    //asciiImage.WriteToFile (
    //    "./out/ascii_ppm_to_bin_pgm.pgm",
    //    GREYMAP | BINARY
    //);
    //asciiImage.WriteToFile (
    //    "./out/ascii_ppm_to_ascii_ppm.ppm",
    //    PIXMAP | ASCII
    //);
    //asciiImage.WriteToFile (
    //    "./out/ascii_ppm_to_ascii_pgm.pgm",
    //    GREYMAP | ASCII
    //);
    //asciiImage.WriteToFile (
    //    "./out/ascii_ppm_to_ascii_pbm.pbm",
    //    BITMAP | ASCII
    //);

}
