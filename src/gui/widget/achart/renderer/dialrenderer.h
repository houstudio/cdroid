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
#ifndef __DIAL_RENDERER_H__
#define __DIAL_RENDERER_H__
namespace cdroid{
/**
 * Dial chart renderer.
 */
class DialRenderer :public DefaultRenderer {
public:
    enum Type {
        NEEDLE, ARROW
    };
private:
    /** The start angle in the dial range. */
    double mAngleMin = 330;
    /** The end angle in the dial range. */
    double mAngleMax = 30;
    /** The start value in dial range. */
    double mMinValue = MathHelper::NULL_VALUE;
    /** The end value in dial range. */
    double mMaxValue = -MathHelper::NULL_VALUE;
    /** The spacing for the minor ticks. */
    double mMinorTickSpacing = MathHelper::NULL_VALUE;
    /** The spacing for the major ticks. */
    double mMajorTickSpacing = MathHelper::NULL_VALUE;
    /** An array of the renderers types (default is NEEDLE). */
    std::vector<Type> mVisualTypes;
public:

    /**
     * Returns the start angle value of the dial.
     *
     * @return the angle start value
     */
    double getAngleMin() const{
        return mAngleMin;
    }

    /**
     * Sets the start angle value of the dial.
     *
     * @param min the dial angle start value
     */
    void setAngleMin(double min) {
        mAngleMin = min;
    }

    /**
     * Returns the end angle value of the dial.
     *
     * @return the angle end value
     */
    double getAngleMax() const{
        return mAngleMax;
    }

    /**
     * Sets the end angle value of the dial.
     *
     * @param max the dial angle end value
     */
    void setAngleMax(double max) {
        mAngleMax = max;
    }

    /**
     * Returns the start value to be rendered on the dial.
     *
     * @return the start value on dial
     */
    double getMinValue() const{
        return mMinValue;
    }

    /**
     * Sets the start value to be rendered on the dial.
     *
     * @param min the start value on the dial
     */
    void setMinValue(double min) {
        mMinValue = min;
    }

    /**
     * Returns if the minimum dial value was set.
     *
     * @return the minimum dial value was set or not
     */
    bool isMinValueSet() const{
        return mMinValue != MathHelper::NULL_VALUE;
    }

    /**
     * Returns the end value to be rendered on the dial.
     *
     * @return the end value on the dial
     */
    double getMaxValue() const{
        return mMaxValue;
    }

    /**
     * Sets the end value to be rendered on the dial.
     *
     * @param max the end value on the dial
     */
    void setMaxValue(double max) {
        mMaxValue = max;
    }

    /**
     * Returns if the maximum dial value was set.
     *
     * @return the maximum dial was set or not
     */
    bool isMaxValueSet() const{
        return mMaxValue != -MathHelper::NULL_VALUE;
    }

    /**
     * Returns the minor ticks spacing.
     *
     * @return the minor ticks spacing
     */
    double getMinorTicksSpacing() const{
        return mMinorTickSpacing;
    }

    /**
     * Sets the minor ticks spacing.
     *
     * @param spacing the minor ticks spacing
     */
    void setMinorTicksSpacing(double spacing) {
        mMinorTickSpacing = spacing;
    }

    /**
     * Returns the major ticks spacing.
     *
     * @return the major ticks spacing
     */
    double getMajorTicksSpacing() const{
        return mMajorTickSpacing;
    }

    /**
     * Sets the major ticks spacing.
     *
     * @param spacing the major ticks spacing
     */
    void setMajorTicksSpacing(double spacing) {
        mMajorTickSpacing = spacing;
    }

    /**
     * Returns the visual type at the specified index.
     *
     * @param index the index
     * @return the visual type
     */
    Type getVisualTypeForIndex(int index) const{
        if (index < mVisualTypes.size()) {
            return mVisualTypes.at(index);
        }
        return Type::NEEDLE;
    }

    /**
     * Sets the visual types.
     *
     * @param types the visual types
     */
    void setVisualTypes(const std::vector<Type>& types) {
        mVisualTypes=types;
    }
};
}/*endof namespace`*/
#endif/*__DIAL_RENDERER_H__*/
