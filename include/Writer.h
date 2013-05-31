#ifndef _WRITER_H_
#define _WRITER_H_

#include <QObject>
#include <QString>
#include "Config.h"
#include "Image.h"
#include "PointSet.h"
#include "Tools.h"

class Writer
    :   public QObject {
    Q_OBJECT

private:
    int         fid;
    Image*       camImg;
    Image*       camDepth;
    PointSet*    pointCloud;

public:
    Writer (int id, Image* img, Image* dpth, PointSet* pst)
        :   camImg ( img ), camDepth ( dpth ), pointCloud ( pst ), fid ( id )
    {}
    ~Writer () {
        if ( camImg ) {
            delete camImg;
            camImg = 0x0;
        }
        if ( camDepth ) {
            delete camDepth;
            camDepth = 0x0;
        }
        if ( pointCloud ) {
            delete pointCloud;
            pointCloud = 0x0;
        }
    }

public slots:
    void write () {
        std::string path = Config::OutputPath() + "CapturedFrames/";
        std::string suffix = "_" + Int2Str(fid);

        std::string str_aux = path + "image" + suffix +".pgm";
        camImg->CreateAsciiPgm(str_aux);

        str_aux = path + "depth" + suffix +".pgm";
        camDepth->CreateAsciiPgm(str_aux);

        str_aux = path + "pointset" + suffix +".ply";
        pointCloud->WriteToFile(str_aux);

        emit finished();
    }

signals:
    void finished();
};

#endif // _WRITER_H_
