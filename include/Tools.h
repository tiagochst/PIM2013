#ifndef _TOOLS_H_
#define _TOOLS_H_

#include <string>
#include <sstream>

inline std::string Int2Str(int nb){
    std::ostringstream ss;
    ss << nb;
    return ss.str();
}

#endif // _TOOLS_H_
