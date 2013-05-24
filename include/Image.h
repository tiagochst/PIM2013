#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <string>
#include "Eigen/Dense"
#include "MathUtils.h"

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

class Image {
private:
    int                 m_height;
    int                 m_width;
    int                 m_maxGreyLevel;
    Eigen::MatrixXi     m_figure;
    Eigen::MatrixXf     m_normalisedFigure;

public:
    Image( const int& iWidth, const int& iHeight, const int& iGreyLevel );
    Image( const std::string& iFilename );
    virtual ~Image();

    void SetHeight( const int& iHeight );
    void SetWidth( const int& iWidth );
    void SetMaxGreyLevel( const int& iGreyLevel ); 

    int const& GetHeight() const;
    int const& GetWidth() const;
    int const& GetMaxGreyLevel() const;

    /* Read a binary (P5) or Ascii(P2) .pgm file*/
    void LoadFromFile( const std::string& iFilename );

    /* Verify if image was read correctly*/
    void CreateAsciiPgm( const std::string& iFilename );

    void SetGreyLvl( const int& iRow, const int& iCol, const int& iValue );
    void SetNormed( const int& iRow, const int& iCol, const float& iValue );
    void SetGreyLvl( const CartesianCoordinate& iPos, const int& iValue );
    void SetNormed( const CartesianCoordinate& iPos, const float& iValue );

    const int&   GetGreyLvl( const int& iRow, const int& iCol ) const;
    const int&   GetGreyLvl( const CartesianCoordinate& iPos ) const;
    const float& GetNormed( const int& iRow, const int& iCol ) const;
    const float& GetNormed( const CartesianCoordinate& iPos ) const;

    CartesianCoordinate Center() const;
    
    float TemplateMatch(
        const Image&            iMask,
        CartesianCoordinate&    oBestMatch,
        Image*                  oCorrelationMap=NULL
    ) const;

    float TemplateMatch(
        const Image&            iMask,
        const Rectangle&        iSearchWindow,
        CartesianCoordinate&    oBestMatch,
        Image*                  oCorrelationMap=NULL
    ) const;

    void SubImage(
        const Rectangle&    iRegion,
        Image&              oSubImage
    ) const;
    void SubImage(
        const int&  iX,
        const int&  iY,
        const int&  iWidth,
        const int&  iHeight,
        Image&      oSubImage
    ) const;
    Image FourierTransform() const;
    Image Difference( const Image& iOther ) const;
    float Correlation( const Image& iOther ) const;

    static void TrackPixels(
        const Image&        iRefImage,
        const Image&        iTargetImage,
        const int&          iWindowWidth,
        const int&          iWindowHeight,
        const int&          iNeighbourhoodWidth,
        const int&          iNeighbourhoodHeight,
        Image&              oDisplacementMapX,
        Image&              oDisplacementMapY
    );

private:
    void ResetMatrix();
    void RecalculateGreyLvl();
    void RecalculateNormalised();
};

#endif /* IMAGE_H_ */
