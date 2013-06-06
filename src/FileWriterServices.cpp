#include "FileWriterServices.h"
#include "Image.h"
#include "PointSet.h"
#include "Config.h"
#include <QThread>
#include "Tools.h"
#include "ParameterHandler.h"
#include <iostream>

class FrameDescriptor {
private:
    unsigned int    m_frameId;
    Image*          m_imageInfo;
    Image*          m_depthInfo;
    PointSet*       m_vertexInfo;

public:
    FrameDescriptor (
        unsigned int&   iFrameId,
        Image*          iBrightnessInfo,
        Image*          iDepthInfo,
        PointSet*       iVertexInfo
    )   :   m_frameId ( iFrameId ),
            m_imageInfo ( iBrightnessInfo ),
            m_depthInfo ( iDepthInfo ),
            m_vertexInfo ( iVertexInfo )
    {}
    ~FrameDescriptor () {
        if ( m_imageInfo ) {
            delete m_imageInfo;
            m_imageInfo = (Image*)0x0;
        }
        if ( m_depthInfo ) {
            delete m_depthInfo;
            m_depthInfo = (Image*)0x0;
        }
        if ( m_vertexInfo ) {
            delete m_vertexInfo;
            m_vertexInfo = (PointSet*)0x0;
        }
    }

    void WriteFrame () const {
        std::string path = Config::OutputPath() + "CapturedFrames/";
        std::string suffix = "_" + Int2Str(m_frameId);

        std::string str_aux;

        if ( m_imageInfo ) {
            str_aux = path + "image" + suffix +".pgm";
            m_imageInfo->CreateAsciiPgm(str_aux);
        }

        if ( m_depthInfo ) {
            str_aux = path + "depth" + suffix +".pgm";
            m_depthInfo->CreateAsciiPgm(str_aux);
        }

        if ( m_vertexInfo ) {
            str_aux = path + "pointset" + suffix +".ply";
            m_vertexInfo->WriteToFile(str_aux);
        }
    }
};

FileWriterServices* FileWriterServices::sm_instance = (FileWriterServices*)0x0;

FileWriterServices::FileWriterServices()
    :   m_framesDone ( false ),
        m_frameQueue ()
{
}

void FileWriterServices::StartCapture () {
    m_framesDone = false;

    m_consumer = new QThread ();
    this->moveToThread ( m_consumer );
    connect (
        m_consumer, SIGNAL (       started () ),
              this, SLOT   ( ProcessFrames () )
    );
    connect (
              this, SIGNAL (      Finished () ),
        m_consumer, SLOT   (          quit () )
    );
    connect (
              this, SIGNAL (      Finished () ),
              this, SLOT   (   deleteLater () )
    );
    connect (
              this, SIGNAL (      Finished () ),
              this, SLOT   ( ResetInstance () )
    );
    connect (
        m_consumer, SIGNAL (      finished () ),
        m_consumer, SLOT   (   deleteLater () )
    );
    m_consumer->start ();
}

void FileWriterServices::ResetInstance () {
    sm_instance = (FileWriterServices*)0x0;
}

void FileWriterServices::DeleteInstance () {
    std::cout << "Deleting FWS instance" << std::endl;
    if ( sm_instance ) {
        delete sm_instance;
        sm_instance = (FileWriterServices*)0x0;
    }
}

void FileWriterServices::ProcessFrames () {
    int fileCounter = 0;
    ParameterHandler* params = ParameterHandler::Instance ();
    while (
            params->GetCaptureMode ()
        ||  !m_frameQueue.empty ()
    ) {
        while ( !m_frameQueue.empty () ) {
            FrameDescriptor* data = m_frameQueue.dequeue ();

            data->WriteFrame();
            delete data;
            data = (FrameDescriptor*)0x0;
            fileCounter++;
            float progress  = (float)(fileCounter)
                            / params->GetNumCaptureFrames ();
            emit ( Progress ( 100 * progress ) );
        }
    }
    std::cout << "Done writing files" << std::endl;
    emit (Finished());
}

FileWriterServices* FileWriterServices::Instance () {
    if ( !sm_instance ) {
        sm_instance = new FileWriterServices ();
    }
    return ( sm_instance );
}

void FileWriterServices::RegisterFrame (
    unsigned int    iFrameId,
    Image*          iBrighnessInfo,
    Image*          iDepthInfo,
    PointSet*       iVertexInfo
) {
     moveToThread ( m_consumer );
     m_frameQueue.enqueue (
        new FrameDescriptor (
            iFrameId,
            iBrighnessInfo,
            iDepthInfo,
            iVertexInfo
        )
    );
}