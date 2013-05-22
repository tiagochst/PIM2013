#ifndef PARAMETERHANDLER_H
#define PARAMETERHANDLER_H

#include <cmath>

class ParameterHandler
{    
private:
    int             m_frame;

private:
    ParameterHandler ()
        :   m_frame(0)
    {}
    ~ParameterHandler ()
    {}

public:
    static inline ParameterHandler* Instance ()
    {
        static ParameterHandler _instance;
        return &_instance;
    }

    void SetFrame (
        const int&             iFrame
    );

    const int& GetFrame () const;
};

#endif // PARAMETERHANDLER_H
