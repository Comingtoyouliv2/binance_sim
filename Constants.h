#pragma once

#include <limits>
#include "types.h"


struct Constants {
    static const Price InvalidPrice = std::numeric_limits<Price>::min();
};