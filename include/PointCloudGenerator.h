#ifndef _POINTCLOUDGENERATOR_H_
#define _POINTCLOUDGENERATOR_H_

#include <string>
#include <XnTypes.h>
#include <vector>

class PointSet;
class Image;
class PPMImage;

class PointCloudGenerator {
public:
    static PointSet* Generate (
    const PPMImage&         iTexture,
    Image&                  iDepth,
    std::vector<XnPoint3D>& iRealWorld,
    const std::string&      iPath
    );
};

#endif // _POINTCLOUDGENERATOR_H_
