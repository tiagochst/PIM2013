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


private:
    ParameterHandler ()
        :   m_frame1(0),
            m_frame2(0),
            m_mesh(true),
            m_wSize ( 7 ),
            m_nSize ( 9 )
    {}
    ~ParameterHandler ()
    {}

public:
    static inline ParameterHandler* Instance ()
    {
        static ParameterHandler _instance;
        return &_instance;
    }

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

    const unsigned int& GetWindowSize () const;
    const void SetWindowSize (
        const unsigned int& iWindowSize
    );

    const unsigned int& GetNeighbourhoodSize () const;
    const void SetNeighbourhoodSize (
        const unsigned int& iNeighbourhoodSize
    );
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
