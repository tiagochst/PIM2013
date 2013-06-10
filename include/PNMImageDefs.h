#ifndef _PNMIMAGEDEFS_H_
#define _PNMIMAGEDEFS_H_

enum FileMode {
    BINARY  =   ( 0x1u << 0 ),
    ASCII   =   ( 0x1u << 1 ),
    PIXMAP  =   ( 0x1u << 2 ),
    GREYMAP =   ( 0x1u << 3 ),
    BITMAP  =   ( 0x1u << 4 )
};
enum PNMChannel {
    RED     =   ( 0x1u << 0 ),
    GREEN   =   ( 0x1u << 1 ),
    BLUE    =   ( 0x1u << 2 ),
    GREY    =   ( 0x1u << 3 ),
    BIT     =   ( 0x1u << 4 )
};
struct RGBColor {
    unsigned int r, g, b;
};

#endif // _PNMIMAGEDEFS_H_
