#include "PNMImage.h"
#include "PPMImage.h"

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
