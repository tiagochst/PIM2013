#include "FileWriterServices.h"
#include "Image.h"
#include "PointSet.h"
#include "Config.h"
#include <QThread>
#include "Tools.h"
#include "ParameterHandler.h"
#include <iostream>
#include "PPMImage.h"
#include "PointCloudGenerator.h"
#include <vector>
#include <XnTypes.h>

class FrameDescriptor {
private:
    unsigned int    m_frameId;
    union {
        Image*      b;
        PPMImage*   t;
    }               m_imageInfo;
    bool            m_texture;
    Image*          m_depthInfo;
    union {
        PointSet*               ps;
        std::vector<XnPoint3D>* p;
    }               m_vertexInfo;
    std::string     m_path;

public:
    FrameDescriptor (
        unsigned int&   iFrameId,
        Image*          iBrightnessInfo,
        Image*          iDepthInfo,
        PointSet*       iVertexInfo,
        std::string     iPath
    )   :   m_frameId ( iFrameId ),
            m_texture ( false ),
            m_depthInfo ( iDepthInfo ),
            m_path ( iPath + "f" + Int2Str ( iFrameId ) + "/" )
    {
        m_imageInfo.b = iBrightnessInfo;
        m_vertexInfo.ps = iVertexInfo;
    }
    FrameDescriptor (
        unsigned int&           iFrameId,
        PPMImage*               iTextureInfo,
        Image*                  iDepthInfo,
        std::vector<XnPoint3D>* iVertexInfo,
        std::string             iPath
    )   :   m_frameId ( iFrameId ),
            m_texture ( true ),
            m_depthInfo ( iDepthInfo ),
            m_path ( iPath + "f" + Int2Str ( iFrameId ) + "/" )
    {
        m_imageInfo.t = iTextureInfo;
        m_vertexInfo.p = iVertexInfo;
    }
    ~FrameDescriptor () {
        if ( m_texture ) {
            if ( m_imageInfo.b ) {
                delete m_imageInfo.b;
                m_imageInfo.b = (Image*)0x0;
            }
        } else {
            if ( m_imageInfo.t ) {
                delete m_imageInfo.t;
                m_imageInfo.t = (PPMImage*)0x0;
            }
        }
        if ( m_depthInfo ) {
            delete m_depthInfo;
            m_depthInfo = (Image*)0x0;
        }
        if ( !m_texture ) {
            delete m_vertexInfo.ps;
            m_vertexInfo.ps = (PointSet*)0x0;
        } else {
            delete m_vertexInfo.p;
            m_vertexInfo.p = (std::vector<XnPoint3D>*)0x0;
        }
    }

    void WriteFrame () const {
        system ( ( "mkdir -p " + m_path ).c_str() );

        std::string str_aux;
        if ( !m_texture ) {
            if ( m_imageInfo.b ) {
                str_aux = m_path + "texture.pgm";
                m_imageInfo.b->CreateAsciiPgm(str_aux);
            }
        } else {
            if ( m_imageInfo.t ) {
                m_imageInfo.t->WriteToFile(m_path + "texture.pgm", GREYMAP | BINARY);
                m_imageInfo.t->WriteToFile(m_path + "colorData.ppm", PIXMAP | BINARY);
            }
        }

        if ( m_depthInfo ) {
            str_aux = m_path + "depthMap.prefilter.pgm";
            m_depthInfo->CreateAsciiPgm(str_aux);
        }

        if ( !m_texture ) {
            str_aux = m_path + "mesh.ply";
            m_vertexInfo.ps->WriteToFile(str_aux);
        } else {
            PointSet* ps = PointCloudGenerator::Generate(*(m_imageInfo.t), *m_depthInfo, *(m_vertexInfo.p), m_path );
            str_aux = m_path + "mesh.ply";
            ps->WriteToFile(str_aux);
            delete ps;
            ps = (PointSet*)0x0;
        }
    }

    void WriteMesh () const {
        if ( m_vertexInfo.ps ) {
            if(m_path.find(".ply") == m_path.size()-4) 
                m_vertexInfo.ps -> WriteToFile(m_path);
            else m_vertexInfo.ps -> WriteToFile(m_path + ".ply");
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
    unsigned int            iFrameId,
    PPMImage*               iTextureInfo,
    Image*                  iDepthInfo,
    std::vector<XnPoint3D>* iVertexInfo,
    std::string             iPath
) {
     moveToThread ( m_consumer );
     m_frameQueue.enqueue (
        new FrameDescriptor (
            iFrameId,
            iTextureInfo,
            iDepthInfo,
            iVertexInfo,
            iPath
        )
    );
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

