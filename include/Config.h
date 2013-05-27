#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>

class Config {
private:
    static std::string _rootPath;
    static std::string _resourcesPath;
    static std::string _dataPath;
    static std::string _outputPath;
    static std::string _configPath;
    static std::string _capturedFramesPath;

public:
    static void LoadConfigs(const std::string& iFilename);
    static const std::string& RootPath();
    static const std::string& ResourcesPath();
    static const std::string& DataPath();
    static const std::string& OutputPath();
    static const std::string& ConfigPath();
    static const std::string& FramesPath();
};

#endif
