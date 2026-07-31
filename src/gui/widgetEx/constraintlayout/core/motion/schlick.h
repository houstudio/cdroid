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
 *
 * Schlick's bias and gain easing functions ("Schlick(s,t)").
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_MOTION_SCHLICK_H
#define CDROID_CONSTRAINTLAYOUT_CORE_MOTION_SCHLICK_H

#include <string>

#include <widgetEx/constraintlayout/core/motion/easing.h>

namespace cdroid {

class Schlick : public Easing {
  public:
    explicit Schlick(const std::string& configString);
    double get(double x) const override;
    double getDiff(double x) const override;
  private:
    double func(double x) const;
    double dfunc(double x) const;
    double mS = 0, mT = 0;
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_CORE_MOTION_SCHLICK_H
