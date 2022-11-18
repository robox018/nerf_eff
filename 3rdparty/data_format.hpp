#pragma once

#include "vcommon.hpp"
#include <string>


struct DataFormat {
    enum {
        RGBA,  // Simply stores rgba
        SH,
        SG,
        ASG,
        _COUNT,
    } format;

    // SH/SG/ASG dimension per channel
    int basis_dim = -1;

    // Parse a string like 'SH16', 'SG25'
    void parse(const std::string& str);

    // Convert to string
    std::string to_string() const;
};


