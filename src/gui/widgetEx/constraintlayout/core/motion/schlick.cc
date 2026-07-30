/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/

/*
 * Ported to C++ for CDROID from androidx.constraintlayout.core.motion.utils.Schlick.
 */
#include <widgetEx/constraintlayout/core/motion/schlick.h>

namespace cdroid {

Schlick::Schlick(const std::string& configString) {
    mStr = configString;
    size_t start = configString.find('(');
    size_t off1  = configString.find(',', start);
    mS = std::stod(configString.substr(start + 1, off1 - start - 1));
    size_t off2  = configString.find(',', off1 + 1);
    mT = std::stod(configString.substr(off1 + 1, off2 - off1 - 1));
}

double Schlick::func(double x) const {
    if (x < mT) {
        return mT * x / (x + mS * (mT - x));
    }
    return ((1 - mT) * (x - 1)) / (1 - x - mS * (mT - x));
}

double Schlick::dfunc(double x) const {
    if (x < mT) {
        return (mS * mT * mT) / ((mS * (mT - x) + x) * (mS * (mT - x) + x));
    }
    return (mS * (mT - 1) * (mT - 1))
           / ((-mS * (mT - x) - x + 1) * (-mS * (mT - x) - x + 1));
}

double Schlick::getDiff(double x) const {
    return dfunc(x);
}

double Schlick::get(double x) const {
    return func(x);
}

} // namespace cdroid
