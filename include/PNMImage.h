#ifndef _PNMIMAGE_H_
#define _PNMIMAGE_H_

#include <string>

class PNMImage {
private:
    unsigned int    m_width;
    unsigned int    m_height;

public:
    PNMImage ();
    virtual ~PNMImage ();

    const unsigned int& Width () const;
    const unsigned int& Height () const;

    virtual void ResetDimensions (
        const unsigned int&     iWidth,
        const unsigned int&     iHeight
    );
    virtual const unsigned int& GetMaxValue () const = 0;
    virtual void SetMaxValue (
        const unsigned int&     iMaxValue
    ) = 0;
    virtual const unsigned int& GetChannelValue (
        const unsigned int&     iRow,
        const unsigned int&     iCol,
        const unsigned int&     iChannel
    ) const = 0;
    virtual void SetChannelValue (
        const unsigned int&     iRow,
        const unsigned int&     iCol,
        const unsigned int&     iChannel,
        const unsigned int&     iValue
    ) = 0;
    void LoadFromFile (
        const std::string&      iFilename
    );
    virtual void WriteToFile (
        const std::string&      iFilename,
        const unsigned int&     iMode
    ) const = 0;
};

#endif // _PNMIMAGE_H_
