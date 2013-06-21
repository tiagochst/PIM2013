#ifndef PARAMETERHANDLER_H
#define PARAMETERHANDLER_H

#include <cmath>

#include "Frame.h"

class ParameterHandler
{    
private:
    int             m_frame1;
    int             m_frame2;
    bool            m_mesh;

    unsigned int    m_wSize;
    unsigned int    m_nSize;


    bool            m_cameraFound;
    bool            m_captureMode;
    unsigned int    m_numCaptureFrames;

    float           m_nearPlane;
    float           m_farPlane;
    bool            m_depthCrop;

    double          m_threshold;

    Frame*          m_frame;

private:
    ParameterHandler ()
        :   m_frame1 ( 0 ),
            m_frame2 ( 0 ),
            m_mesh ( true ),
            m_wSize ( 5 ),
            m_nSize ( 9 ),
            m_cameraFound (false),
            m_numCaptureFrames ( 35 ),
            m_nearPlane ( 0 ),
            m_farPlane ( 1500 ),
            m_depthCrop ( false ),
            m_threshold ( 0.0 ),
            m_frame ( (Frame*)0x0 )
    {}
    ~ParameterHandler ()
    {}

public:
    static inline ParameterHandler* Instance ()
    {
        static ParameterHandler _instance;
        return &_instance;
    }

    void SetCaptureMode (
        const bool&     iCaptureMode
    );
    const bool& GetCaptureMode () const;

    void SetNumCaptureFrames (
        const int&             iFrame
    );

    const unsigned int& GetNumCaptureFrames () const;

    void SetFrame1 (
        const int&             iFrame
    );

    const int& GetFrame1 () const;

    void SetFrame2 (
        const int&             iFrame
    );

    const int& GetFrame2 () const;

    void SetMesh (const bool& iMesh);

    const bool& GetMesh () const;

    void SetCamera (const bool& iCamera);

    const bool& GetCamera () const;

    const unsigned int& GetWindowSize () const;
    const void SetWindowSize (
        const unsigned int& iWindowSize
    );

    const unsigned int& GetNeighbourhoodSize () const;
    const void SetNeighbourhoodSize (
        const unsigned int& iNeighbourhoodSize
    );

    const float& GetNearPlane () const;
    void SetNearPlane (
        const float& iNearPlane
    );
    const float& GetFarPlane () const;
    void SetFarPlane (
        const float& iFarPlane
    );
    const bool& GetDepthCrop () const;
    void SetDepthCrop (
        const bool& iDepthCrop
    );

    void SetThreshold (
        const double&             iThreshold
    );

    const double& GetThreshold () const;

    const Frame* GetCurrentFrame () const;
    Frame* GetCurrentFrame ();
    void SetCurrentFrame ( Frame* iFrame );

};

inline const Frame* ParameterHandler::GetCurrentFrame () const {
    return m_frame;
}
inline Frame* ParameterHandler::GetCurrentFrame () {
    return m_frame;
}
inline void ParameterHandler::SetCurrentFrame ( Frame* iFrame ) {
    if ( m_frame ) {
        delete m_frame;
    }
    m_frame = iFrame;
}


inline const unsigned int& ParameterHandler::GetWindowSize ()
const {
    return m_wSize;
}
inline const void ParameterHandler::SetWindowSize (
    const unsigned int& iWindowSize
) {
    m_wSize = iWindowSize;
}

inline const unsigned int& ParameterHandler::GetNeighbourhoodSize ()
const {
    return m_nSize;
}
inline const void ParameterHandler::SetNeighbourhoodSize (
    const unsigned int& iNeighbourhoodSize
) {
    m_nSize = iNeighbourhoodSize;
}


#endif // PARAMETERHANDLER_H
