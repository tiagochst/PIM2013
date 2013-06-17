#ifndef _IMAGEBASE_H_
#define _IMAGEBASE_H_

#include <string>
#include <sstream>

struct CartesianCoordinate;
class Rectangle;
class Image;
#ifndef IMG_EQ_SIMG
class SubImage;
#else
typedef Image SubImage;
#endif

class BadIndex {
    std::string m_what;
public:
    inline BadIndex(int iCol, int iRow)
    {
        std::stringstream ss;
        ss << "Col: " << iCol << " Row: " << iRow;
        m_what = ss.str();
    }
    inline const std::string& what() { return m_what; }
};
class IncompatibleImages {};

class ImageBase {
public:
    ImageBase () {}
    virtual ~ImageBase () {}

    virtual void SetDimensions (
        const int& iWidth,
        const int& iHeight
    ) = 0;
    virtual void SetHeight (
        const int& iHeight
    ) = 0;
    virtual void SetWidth (
        const int& iWidth
    ) = 0;
    virtual void SetMaxGreyLevel (
        const int&  iGreyLevel,
        const bool& iRenormalise=false
    ) = 0; 
    virtual int const& GetHeight () const = 0;
    virtual int const& GetWidth () const = 0;
    virtual int const& GetMaxGreyLevel () const = 0;
    virtual const int& GetGreyLvl (
        const int& iRow,
        const int& iCol
    ) const = 0;
    virtual const int& GetGreyLvl (
        const CartesianCoordinate& iPos
    ) const = 0;
    virtual const float& GetNormed (
        const int& iRow,
        const int& iCol
    ) const = 0;
    virtual const float& GetNormed (
        const CartesianCoordinate& iPos
    ) const = 0;
    virtual void SetGreyLvl (
        const int& iRow,
        const int& iCol,
        const int& iValue
    ) = 0;
    virtual void SetNormed (
        const int& iRow,
        const int& iCol,
        const float& iValue
    ) = 0;
    virtual void SetGreyLvl (
        const CartesianCoordinate&  iPos,
        const int&                  iValue
    ) = 0;
    virtual void SetNormed (
        const CartesianCoordinate&  iPos,
        const float&                iValue
    ) = 0;

    // ImageBase methods
    /* Verify if image was read correctly*/
    void CreateAsciiPgm (
        const std::string&  iFilename
    );

    CartesianCoordinate Center () const;
    
    float Correlation (
        const ImageBase&    iOther
    ) const;
    void GetSubImage (
        const Rectangle&    iRegion,
        SubImage&           oSubImage
    );
    virtual void GetSubImage (
        const int&  iX,
        const int&  iY,
        const int&  iWidth,
        const int&  iHeight,
        SubImage&   oSubImage
    );
    float TemplateMatch (
        const ImageBase&        iMask,
        CartesianCoordinate&    oBestMatch,
        ImageBase*              oCorrelationMap=NULL
    ) const;
    float TemplateMatch (
        const ImageBase&        iMask,
        const int&              iSearchX,
        const int&              iSearchY,
        const int&              iSearchWidth,
        const int&              iSearchHeight,
        CartesianCoordinate&    oBestMatch,
        ImageBase*              oCorrelationMap=NULL
    ) const;
    float TemplateMatch (
        const ImageBase&        iMask,
        const Rectangle&        iSearchWindow,
        CartesianCoordinate&    oBestMatch,
        ImageBase*              oCorrelationMap=NULL
    ) const;

    static void TrackPixels(
        ImageBase&          iRefImage,
        ImageBase&          iTargetImage,
        const int&          iWindowWidth,
        const int&          iWindowHeight,
        const int&          iNeighbourhoodWidth,
        const int&          iNeighbourhoodHeight,
        ImageBase&          oDisplacementMapX,
        ImageBase&          oDisplacementMapY
    );

    static float CalculateErrorScore (
        ImageBase&          iImageA,
        ImageBase&          iImageB
    );
    
    float PixelCorrelation (
        const ImageBase& iOther,
        const CartesianCoordinate&    p,
        const CartesianCoordinate&    q,
        const int&          WinWidth,
        const int&          WinHeigh
    ) const;

};

#endif // _IMAGEBASE_H_
