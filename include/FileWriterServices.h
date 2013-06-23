#ifndef _FILEWRITERSERVICES_H_
#define _FILEWRITERSERVICES_H_

#include <QObject>
#include <QQueue>
#include <vector>
#include <XnTypes.h>

class FrameDescriptor;
class QThread;
class PointSet;
class Image;
class PPMImage;

class FileWriterServices
    :   public QObject
{
    Q_OBJECT

private:
    QQueue<FrameDescriptor*>    m_frameQueue;
    bool                        m_framesDone;

    QThread*                    m_consumer;
    static FileWriterServices*  sm_instance;

public:
    static FileWriterServices* Instance ();
    static void DeleteInstance ();
    void RegisterFrame (
        unsigned int            iFrameId,
        PPMImage*               iTextureInfo,
        Image*                  iDepthInfo,
        std::vector<XnPoint3D>* iVertexInfo,
        std::string             iPath
    );
    void RegisterFrame (
        unsigned int    iFrameId,
        Image*          iBrightnessInfo,
        Image*          iDepthInfo,
        PointSet*       iVertexInfo,
        std::string     iPath
    );

signals:
    void NoMoreFrames ();
    void Finished ();
    void Progress (int);

public slots:
    void StartCapture ();
    void ProcessFrames ();
    void StartSavingMesh (); 
    void ProcessMesh ();


protected slots:
    void ResetInstance ();

private:
    FileWriterServices ();
};

#endif // _FILEWRITERSERVICES_H_
