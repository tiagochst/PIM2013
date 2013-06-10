#ifndef _PPMIMAGE_H_
#define _PPMIMAGE_H_

#include "PNMImage.h"
#include "PNMImageDefs.h"

class PPMImage
    :   public PNMImage
{
private:
    RGBColor**                  m_pixmap;
    unsigned int                m_maxValue;

public:
    PPMImage ();
    ~PPMImage ();

    void ResetDimensions (
        const unsigned int&     iWidth,
        const unsigned int&     iHeight
    );
    const unsigned int& GetMaxValue () const;
    void SetMaxValue (
        const unsigned int&     iValue
    );
    const unsigned int& GetChannelValue (
        const unsigned int&     iRow,
        const unsigned int&     iCol,
        const unsigned int&     iChannel
    ) const;
    void SetChannelValue (
        const unsigned int&     iRow,
        const unsigned int&     iCol,
        const unsigned int&     iChannel,
        const unsigned int&     iValue
    );
    //void LoadFromFile (
    //    const std::string&      iFilename
    //);
    void WriteToFile (
        const std::string&      iFilename,
        const unsigned int&     iMode=(PIXMAP | BINARY)
    ) const;

private:
    void AllocatePixmap ();
    void ClearPixmap ();
};

#endif // _PPMIMAGE_H_
