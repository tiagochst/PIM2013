#ifndef _TOOLS_H_
#define _TOOLS_H_

#include <string>
#include <sstream>
#include <iomanip>

template<typename T>
inline std::string toString ( const T& val ) {
    std::stringstream ss;

    ss << val;

    return ss.str ();
}

inline std::string toString ( const float& val ) {
    std::stringstream ss;

    ss << std::setprecision(2) << val;

    return ss.str ();
}

inline std::string Int2Str(int nb){
    std::ostringstream ss;
    ss << nb;
    return ss.str();
}

#endif // _TOOLS_H_
