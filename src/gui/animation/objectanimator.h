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
#ifndef __OBJECT_ANIMATOR_H__
#define __OBJECT_ANIMATOR_H__

#include <animation/valueanimator.h>
#include <animation/animationhandler.h>
namespace cdroid{

class ObjectAnimator:public ValueAnimator{
private:
    void* mTarget;
    std::string mTargetClass;
    std::string mPropertyName;
    const Property* mProperty;
    bool mAutoCancel;
private:
    bool hasSameTargetAndProperties(const Animator*anim);
    ObjectAnimator(void* target,const std::string& propertyName);
protected:
    void initAnimation()override;
    bool isInitialized()override;
public:
    ObjectAnimator();
    ObjectAnimator(const ObjectAnimator&);
    ~ObjectAnimator()override;
    void setTarget(void*target)override;
    void*getTarget();
    ObjectAnimator&setDuration(int64_t dur)override;

    void setupStartValues()override;
    void setupEndValues()override;
    void animateValue(float fraction)override;

    void setPropertyName(const std::string&propertyName);
    void setProperty(const Property* property);
    const std::string getPropertyName()const;
    void setIntValues(const std::vector<int>&)override;
    void setFloatValues(const std::vector<float>&values)override;
    ObjectAnimator*clone()const override;
    void setAutoCancel(bool);
    bool shouldAutoCancel(const AnimationHandler::AnimationFrameCallback*anim);
    void start()override;

    static ObjectAnimator* ofInt(void* target,const std::string& propertyName, const std::vector<int>&);
    static ObjectAnimator* ofFloat(void* target,const std::string& propertyName,const std::vector<float>&);
    static ObjectAnimator* ofPropertyValuesHolder(void*target,const std::vector< PropertyValuesHolder*>&values);

    static ObjectAnimator* ofInt(void*target,const Property*prop,const std::vector<int>&);
    static ObjectAnimator* ofFloat(void*target,const Property*prop,const std::vector<float>&);
    std::string toString()const override;
};

}/*endof namespace*/
#endif/*__OBJECT_ANIMATOR_H__*/
