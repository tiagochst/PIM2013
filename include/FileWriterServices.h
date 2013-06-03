#ifndef _FILEWRITERSERVICES_H_
#define _FILEWRITERSERVICES_H_

#include <QObject>
#include <QQueue>

class FrameDescriptor;
class QThread;
class PointSet;
class Image;

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
        unsigned int    iFrameId,
        Image*          iBrightnessInfo,
        Image*          iDepthInfo,
        PointSet*       iVertexInfo
    );

signals:
    void NoMoreFrames ();
    void Finished ();
    void Progress (int);

public slots:
    void StartCapture ();
    void ProcessFrames ();

protected slots:
    void ResetInstance ();

private:
    FileWriterServices ();
};

#endif // _FILEWRITERSERVICES_H_
