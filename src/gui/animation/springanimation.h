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
*/
#ifndef __SPRING_ANIMATION_H__
#define __SPRING_ANIMATION_H__
#include <animation/springforce.h>
#include <animation/dynamicanimation.h>
namespace cdroid{
class SpringAnimation :public DynamicAnimation{
private:
    static constexpr float UNSET = FLT_MAX;
    SpringForce* mSpring = nullptr;
    float mPendingPosition = UNSET;
    bool mEndRequested = false;
private:
    void sanityCheck();
public:
    /**
     * <p>This creates a SpringAnimation that animates a {@link FloatValueHolder} instance. During
     * the animation, the {@link FloatValueHolder} instance will be updated via
     * {@link FloatValueHolder#setValue(float)} each frame. The caller can obtain the up-to-date
     * animation value via {@link FloatValueHolder#getValue()}.
     *
     * <p><strong>Note:</strong> changing the value in the {@link FloatValueHolder} via
     * {@link FloatValueHolder#setValue(float)} outside of the animation during an
     * animation run will not have any effect on the on-going animation.
     *
     * @param floatValueHolder the property to be animated
     */
    SpringAnimation(FloatValueHolder* floatValueHolder);
    /**
     * This creates a SpringAnimation that animates the property of the given object.
     * Note, a spring will need to setup through {@link #setSpring(SpringForce)} before
     * the animation starts.
     *
     * @param object the Object whose property will be animated
     * @param property the property to be animated
     * @param <K> the class on which the Property is declared
     */
    SpringAnimation(void* object,const FloatProperty* property);

    /**
     * This creates a SpringAnimation that animates the property of the given object. A Spring will
     * be created with the given final position and default stiffness and damping ratio.
     * This spring can be accessed and reconfigured through {@link #setSpring(SpringForce)}.
     *
     * @param object the Object whose property will be animated
     * @param property the property to be animated
     * @param finalPosition the final position of the spring to be created.
     * @param <K> the class on which the Property is declared
     */
    SpringAnimation(void* object,const FloatProperty*property,float finalPosition);
    ~SpringAnimation()override;
    SpringForce* getSpring() const;
    SpringAnimation& setSpring(SpringForce* force);

    void start() override;

    /**
     * Updates the final position of the spring.
     * <p/>
     * When the animation is running, calling this method would assume the position change of the
     * spring as a continuous movement since last frame, which yields more accurate results than
     * changing the spring position directly through {@link SpringForce#setFinalPosition(float)}.
     * <p/>
     * If the animation hasn't started, calling this method will change the spring position, and
     * immediately start the animation.
     *
     * @param finalPosition rest position of the spring
     */
    void animateToFinalPosition(float finalPosition);

    /**
     * Skips to the end of the animation. If the spring is undamped, an
     * {@link IllegalStateException} will be thrown, as the animation would never reach to an end.
     * It is recommended to check {@link #canSkipToEnd()} before calling this method. This method
     * should only be called on main thread. If animation is not running, no-op.
     *
     * @throws IllegalStateException if the spring is undamped (i.e. damping ratio = 0)
     * @throws AndroidRuntimeException if this method is not called on the main thread
     */
    void skipToEnd();
    bool canSkipToEnd() const;

    /************************ Below are private APIs *************************/
    bool updateValueAndVelocity(long deltaT) override;

    float getAcceleration(float value, float velocity) const override;

    bool isAtEquilibrium(float value, float velocity) const override;

    void setValueThreshold(float threshold) override;
};
}/*endof namespace*/
#endif/*__SPRING_ANIMATION_H__*/
