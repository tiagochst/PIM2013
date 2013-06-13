#ifndef PARAMETERHANDLER_H
#define PARAMETERHANDLER_H

#include <cmath>

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

    unsigned int    m_nearPlane;
    unsigned int    m_farPlane;
    bool            m_depthCrop;

    double          m_threshold;

private:
    ParameterHandler ()
        :   m_frame1 ( 0 ),
            m_frame2 ( 0 ),
            m_mesh ( true ),
            m_wSize ( 9 ),
            m_nSize ( 9 ),
            m_cameraFound (false),
            m_numCaptureFrames ( 35 ),
            m_nearPlane ( 0 ),
            m_farPlane ( 1500 ),
            m_depthCrop ( false ),
            m_threshold ( 0.0 )
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

    const unsigned int& GetNearPlane () const;
    void SetNearPlane (
        const unsigned int& iNearPlane
    );
    const unsigned int& GetFarPlane () const;
    void SetFarPlane (
        const unsigned int& iFarPlane
    );
    const bool& GetDepthCrop () const;
    void SetDepthCrop (
        const bool& iDepthCrop
    );

    void SetThreshold (
        const double&             iThreshold
    );

    const double& GetThreshold () const;

};


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
