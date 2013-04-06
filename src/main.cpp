#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include "Image.h"
#include "PointSet.h"
#include "Camera.h"

class Config {
private:
    static std::string _rootPath;
    static std::string _resourcesPath;
    static std::string _dataPath;
    static std::string _outputPath;
    static std::string _configPath;

public:
    static void LoadConfigs(const std::string& iFilename) {
        std::ifstream configFile( iFilename.c_str(), std::fstream::in);
        
        std::string line;
        
        if ( !configFile.good() ) {
            std::cout << "Could not open settings file." << std::endl;
            return;
        }
        
        while ( !configFile.eof() ) {
            std::getline( configFile, line );
            
            std::stringstream lineStream(line);
            
            std::string tag;

            lineStream >> tag;

            if ( tag.compare("RSC_DIR") == 0 ) {
                lineStream >> Config::_resourcesPath;

                std::cout << "RSC_DIR = " << Config::ResourcesPath() << std::endl;
            }
        }
        
        if (Config::_resourcesPath.empty() ) {
            std::cout << "Error loading settings from " << _rootPath + iFilename << std::endl;
            exit(-1);
        }
        if ( Config::_dataPath.empty() ) {
            Config::_dataPath = ResourcesPath() + "Data/";
        }
        if ( Config::_outputPath.empty() ) {
            Config::_outputPath = DataPath() + "Output/";
        }
        if ( Config::_configPath.empty() ) {
            Config::_configPath = DataPath() + "Config/";
        }
    }
    static const std::string& RootPath() {
        return Config::_rootPath;
    }
    static const std::string& ResourcesPath() {
        return Config::_resourcesPath;
    }
    static const std::string& DataPath() {
        return Config::_dataPath;
    }
    static const std::string& OutputPath() {
        return Config::_outputPath;
    }
    static const std::string& ConfigPath() {
        return Config::_configPath;
    }
};
static const std::string USR_HOME = std::string(getenv("HOME"));
std::string Config::_rootPath(USR_HOME + "/.pim2013/");
std::string Config::_resourcesPath("");
std::string Config::_dataPath("");
std::string Config::_outputPath("");
std::string Config::_configPath("");

int main(int argc, char** argv) {
    Config::LoadConfigs(Config::RootPath() + "settings");

    static const std::string RES_PTSET_PATH(Config::ResourcesPath() + "PointSets/");
    static const std::string RES_IMG_PATH(Config::ResourcesPath() + "Images/");

    /* Class image test */
    // Image myImage(RES_IMG_PATH + "frame_20121108T103323.258153_rgb-ascci.pgm");

    /* Class Ply test  */
    //PointSet psAscii, psBinary;    
    //psAscii.LoadFromFile(RES_PTSET_PATH + "frame000-ascii.ply");
    //psBinary.LoadFromFile(RES_PTSET_PATH + "frame000-brut.ply");
 
    Image frame0(RES_IMG_PATH + "frame_20121108T103323.258153_rgb-brut.pgm");
    Image frame1(RES_IMG_PATH + "frame_20121108T103323.390878_rgb-brut.pgm");

    Coordinate bestMatch;
    Image mask(Config::DataPath() + "mask.pgm");
    Image bigMask(Config::DataPath() + "bigMask.pgm");
    
    Image correlation = bigMask.PatternSearch(mask, bestMatch);
    //Image correlation = frame1.PatternSearch( mask, bestMatch );
    std::cout << "Match found at (" << bestMatch.x << ", " 
                                    << bestMatch.y << ")" << std::endl;
    correlation.CreateAsciiPgm(Config::OutputPath() + "correlation.pgm");

    //Image corr01 = frame0.Correlation(frame1);
    //Image corr10 = frame1.Correlation(frame0);
    //corr01.CreateAsciiPgm("corr01");
    //corr10.CreateAsciiPgm("corr10");

    Image diff01 = frame0.Difference(frame1);
    Image diff10 = frame1.Difference(frame0);
    diff01.CreateAsciiPgm(Config::OutputPath() + "diff01.pgm");
    diff10.CreateAsciiPgm(Config::OutputPath() + "diff10.pgm");


    /* Class Camera test: Kinect's  */
    xn::Context        g_context;
    xn::ScriptNode  g_scriptNode;
    xn::EnumerationErrors errors;
    XnStatus                  rc;
    
    /* Create a context with default settings */
    rc = g_context.InitFromXmlFile( ( Config::ConfigPath() + SAMPLE_XML_PATH ).c_str(), g_scriptNode, &errors);
    /* Verify if device is connected */
    if (rc == XN_STATUS_NO_NODE_PRESENT)
    {
        XnChar strError[1024];
        errors.ToString(strError, 1024);
        printf("%s\n", strError);
        return (rc);
    }
    else 
    {
        CHECK_RC(rc,"Open");
    }
        
    Camera& viewer = Camera::CreateInstance(g_context);
    
    rc = viewer.Init(argc, argv);
    CHECK_RC(rc,"Viewer Init");
    
    rc = viewer.Run();
    CHECK_RC(rc,"Viewer run");
    
    return 0;
}
