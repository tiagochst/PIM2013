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
    std::string     m_path;

public:
    FrameDescriptor (
        unsigned int&   iFrameId,
        Image*          iBrightnessInfo,
        Image*          iDepthInfo,
        PointSet*       iVertexInfo,
	std::string     iPath
    )   :   m_frameId ( iFrameId ),
            m_imageInfo ( iBrightnessInfo ),
            m_depthInfo ( iDepthInfo ),
            m_vertexInfo ( iVertexInfo ),
            m_path ( iPath + "f" + Int2Str ( iFrameId ) + "/" )
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
        system ( ( "mkdir -p " + m_path ).c_str() );

        std::string str_aux;
        if ( m_imageInfo ) {
            str_aux = m_path + "texture.pgm";
            m_imageInfo->CreateAsciiPgm(str_aux);
        }

        if ( m_depthInfo ) {
            str_aux = m_path + "depthMap.pgm";
            m_depthInfo->CreateAsciiPgm(str_aux);
        }

        if ( m_vertexInfo ) {
            str_aux = m_path + "mesh.ply";
            m_vertexInfo->WriteToFile(str_aux);
        }
    }

    void WriteMesh () const {
        if ( m_vertexInfo ) {
            if(m_path.find(".ply") == m_path.size()-4) 
                m_vertexInfo -> WriteToFile(m_path);
            else m_vertexInfo -> WriteToFile(m_path + ".ply");
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

void FileWriterServices::StartSavingMesh () {
    m_framesDone = false;

    m_consumer = new QThread ();
    this->moveToThread ( m_consumer );
    connect (
        m_consumer, SIGNAL (       started () ),
              this, SLOT   (   ProcessMesh () )
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

void FileWriterServices::ProcessMesh (
) {
    int fileCounter = 0;
    ParameterHandler* params = ParameterHandler::Instance ();
    while (
	   !m_frameQueue.empty ()
    ) {
        while ( !m_frameQueue.empty () ) {
            FrameDescriptor* data = m_frameQueue.dequeue ();

            data->WriteMesh();
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
    PointSet*       iVertexInfo,
    std::string     iPath
) {
     moveToThread ( m_consumer );
     m_frameQueue.enqueue (
        new FrameDescriptor (
            iFrameId,
            iBrighnessInfo,
            iDepthInfo,
            iVertexInfo,
	    iPath
        )
    );
}

