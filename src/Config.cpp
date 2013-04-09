#include "Config.h"

const std::string USR_HOME = std::string(getenv("HOME"));
std::string Config::_rootPath(USR_HOME + "/.pim2013/");
std::string Config::_resourcesPath("");
std::string Config::_dataPath("");
std::string Config::_outputPath("");
std::string Config::_configPath("");

void Config::LoadConfigs(const std::string& iFilename)
{
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
const std::string& Config::RootPath()
{
    return Config::_rootPath;
}
const std::string& Config::ResourcesPath()
{
    return Config::_resourcesPath;
}
const std::string& Config::DataPath()
{
    return Config::_dataPath;
}
const std::string& Config::OutputPath()
{
    return Config::_outputPath;
}
const std::string& Config::ConfigPath()
{
    return Config::_configPath;
}
