#ifndef PARAMETERHANDLER_H
#define PARAMETERHANDLER_H

#include <cmath>

class ParameterHandler
{    
private:
    int             m_frame1;
    int             m_frame2;
    bool            m_mesh;

private:
    ParameterHandler ()
      :   m_frame1(0),
          m_frame2(0),
          m_mesh(true)
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

};

#endif // PARAMETERHANDLER_H
