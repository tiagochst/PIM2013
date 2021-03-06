#include "ParameterHandler.h"

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
