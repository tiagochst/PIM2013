#include "ParameterHandler.h"

void ParameterHandler::SetCaptureMode (
    const bool&     iCaptureMode
) {
    m_captureMode = iCaptureMode;
}
const bool& ParameterHandler::GetCaptureMode ()
const {
    return m_captureMode;
}

void ParameterHandler::SetCamera (
    const bool&     iCamera
) {
    m_cameraFound = iCamera;
}
const bool& ParameterHandler::GetCamera ()
const {
    return m_cameraFound;
}

const unsigned int& ParameterHandler::GetNumCaptureFrames ()
const {
    return m_numCaptureFrames;
}

void ParameterHandler::SetFrame1 (
    const int&             iFrame
) {
    m_frame1 = iFrame;
}


const int& ParameterHandler::GetFrame1 () const
{
    return m_frame1;
    
}

void ParameterHandler::SetFrame2 (
    const int&             iFrame
) {
    m_frame2 = iFrame;
}


const int& ParameterHandler::GetFrame2 () const
{
    return m_frame2;
    
}

void ParameterHandler::SetMesh (
     const bool& iMesh
) {
      m_mesh = iMesh;
};

const bool& ParameterHandler::GetMesh () const
{
      return m_mesh;
};

void ParameterHandler::SetThreshold (
     const double& iThreshold
) {
    m_threshold = iThreshold;
};

const unsigned int& ParameterHandler::GetNearPlane () 
const {
    return m_nearPlane;
}
void ParameterHandler::SetNearPlane (
    const unsigned int& iNearPlane
) {
    m_nearPlane = iNearPlane;
}
const unsigned int& ParameterHandler::GetFarPlane () 
const {
    return m_farPlane;
}
void ParameterHandler::SetFarPlane (
    const unsigned int& iFarPlane
) {
    m_farPlane = iFarPlane;
}
const bool& ParameterHandler::GetDepthCrop () 
const {
    return m_depthCrop;
}
void ParameterHandler::SetDepthCrop (
    const bool& iDepthCrop
) {
    m_depthCrop = iDepthCrop;
}

const double& ParameterHandler::GetThreshold () const
{
    return m_threshold;
};
