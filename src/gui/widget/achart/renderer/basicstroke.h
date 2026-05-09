/**
 * Copyright (C) 2009 - 2012 SC 4ViewSoft SRL
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef __BASIC_STROKE_H__
#define __BASIC_STROKE_H__
#include <vector>
#include <cairomm/context.h>
namespace cdroid{
/**
 * A descriptor for the stroke style.
 */
class BasicStroke{
public:
    /** The solid line style. */
    static const BasicStroke* const SOLID;// = new BasicStroke(Cairo::Context::LineCap::BUTT, Cairo::Context::LineJoin::MITER, 4,{}, 0);
    /** The dashed line style. */
    static const BasicStroke* const DASHED;//= new BasicStroke(Cairo::Context::LineCap::ROUND, Cairo::Context::LineJoin::BEVEL, 10, {10,10}, 1);
    /** The dot line style. */
    static const BasicStroke* const DOTTED;//= new BasicStroke(Cairo::Context::LineCap:ROUND, Cairo::Context::LineJoin::BEVEL, 5,{2,10}, 1);
private:
    /** The stroke cap. */
    Cairo::Context::LineCap mCap;
    /** The stroke join. */
    Cairo::Context::LineJoin mJoin;
    /** The stroke miter. */
    float mMiter;
    /** The path effect intervals. */
    std::vector<double> mIntervals;
    /** The path effect phase. */
    float mPhase;
public:
    /**
     * Build a new basic stroke style.
     *
     * @param cap the stroke cap
     * @param join the stroke join
     * @param miter the stroke miter
     * @param intervals the path effect intervals
     * @param phase the path effect phase
     */
    BasicStroke(Cairo::Context::LineCap cap, Cairo::Context::LineJoin join, float miter, const std::vector<double>& intervals, float phase) {
        mCap = cap;
        mJoin = join;
        mMiter = miter;
        mIntervals = intervals;
    }

    /**
     * Returns the stroke cap.
     *
     * @return the stroke cap
     */
    Cairo::Context::LineCap getCap() const{
        return mCap;
    }

    /**
     * Returns the stroke join.
     *
     * @return the stroke join
     */
    Cairo::Context::LineJoin getJoin() const{
        return mJoin;
    }

    /**
     * Returns the stroke miter.
     *
     * @return the stroke miter
     */
    float getMiter() const{
        return mMiter;
    }

    /**
     * Returns the path effect intervals.
     *
     * @return the path effect intervals
     */
    const std::vector<double>& getIntervals() const{
        return mIntervals;
    }

    /**
     * Returns the path effect phase.
     *
     * @return the path effect phase
     */
    float getPhase() const{
        return mPhase;
    }
};
}/*endof namespace*/
#endif/*__BASIC_STROKE_H__*/
