#include "ParameterHandler.h"

void ParameterHandler::SetFrame (
    const int&             iFrame
) {
    m_frame = iFrame;
}


const int& ParameterHandler::GetFrame () const
{
    return m_frame;
    
}
