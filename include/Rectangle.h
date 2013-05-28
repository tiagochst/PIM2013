#ifndef _RECTANGLE_H_
#define _RECTANGLE_H_

class Rectangle {
private:
    CartesianCoordinate     m_position;
    CartesianCoordinate     m_center;
    int                     m_right;
    int                     m_bottom;
    int                     m_width;
    int                     m_height;

public:
    inline Rectangle ( const int& iX, const int& iY, const int& iWidth, const int& iHeight )
        :   m_position( iX, iY ), m_center( iX + iWidth / 2, iY + iHeight / 2 ),
            m_width( iWidth ), m_height( iHeight ),
            m_right( iX + iWidth ), m_bottom( iY + iHeight )
    {}
    inline const int& Width() const { return m_width; }
    inline const int& Height() const { return m_height; } 
    inline const int& X() const { return m_position.x; }
    inline const int& Y() const { return m_position.y; }
    inline const int& Right() const { return m_right; }
    inline const int& Left() const { return m_position.x; }
    inline const int& Top() const { return m_position.y; }
    inline const int& Bottom() const { return m_bottom; }
    inline const CartesianCoordinate& Position() const { return m_position; }
    inline const CartesianCoordinate& Center() const { return m_center; }
};

#endif // _RECTANGLE_H_
