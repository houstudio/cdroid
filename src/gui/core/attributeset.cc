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
#include <core/attributeset.h>
#include <porting/cdlog.h>

namespace cdroid{

AttributeSet::AttributeSet():AttributeSet(nullptr,""){
}

AttributeSet::AttributeSet(Context*ctx,const std::string&package)
    :mContext(ctx),mPackage(package){
}

Context*AttributeSet::getContext()const{
    return mContext;
}

// ----------------------------------------------------------------------------
// AOSP android.util.AttributeSet — interface defaults. The parsers implement
// the real answers (text XmlPullParser / binary XmlBlock::Parser); these
// defaults are what a synthetic empty set observes.
// ----------------------------------------------------------------------------
size_t AttributeSet::getAttributeCount()const{
    return 0;
}

std::string AttributeSet::getAttributeNamespace(int) const {
    return std::string();
}

std::string AttributeSet::getAttributeName(int) const {
    return std::string();
}

std::string AttributeSet::getAttributeValue(int) const {
    return std::string();
}

std::string AttributeSet::getAttributeValue(const std::string&, const std::string&) const {
    return std::string();
}

std::string AttributeSet::getPositionDescription() const {
    return std::string();
}

int AttributeSet::getAttributeNameResource(int) const {
    return 0;
}

int AttributeSet::getAttributeListValue(int, const std::vector<std::string>& options,
        int defaultValue) const {
    return defaultValue;
}

bool AttributeSet::getAttributeBooleanValue(int, bool defaultValue) const {
    return defaultValue;
}

int AttributeSet::getAttributeResourceValue(int, int defaultValue) const {
    return defaultValue;
}

int AttributeSet::getAttributeIntValue(int, int defaultValue) const {
    return defaultValue;
}

int AttributeSet::getAttributeUnsignedIntValue(int, int defaultValue) const {
    return defaultValue;
}

float AttributeSet::getAttributeFloatValue(int, float defaultValue) const {
    return defaultValue;
}

int AttributeSet::getAttributeListValue(const std::string&, const std::string&,
        const std::vector<std::string>& options, int defaultValue) const {
    return defaultValue;
}

bool AttributeSet::getAttributeBooleanValue(const std::string&, const std::string&,
        bool defaultValue) const {
    return defaultValue;
}

int AttributeSet::getAttributeResourceValue(const std::string&, const std::string&,
        int defaultValue) const {
    return defaultValue;
}

int AttributeSet::getAttributeIntValue(const std::string&, const std::string&,
        int defaultValue) const {
    return defaultValue;
}

int AttributeSet::getAttributeUnsignedIntValue(const std::string&, const std::string&,
        int defaultValue) const {
    return defaultValue;
}

float AttributeSet::getAttributeFloatValue(const std::string&, const std::string&,
        float defaultValue) const {
    return defaultValue;
}

std::string AttributeSet::getIdAttribute() const {
    return getAttributeValue(std::string(), "id");
}

std::string AttributeSet::getClassAttribute() const {
    return getAttributeValue(std::string(), "class");
}

int AttributeSet::getIdAttributeResourceValue(int defaultValue) const {
    return getAttributeResourceValue(std::string(), "id", defaultValue);
}

int AttributeSet::getStyleAttribute() const {
    return getAttributeResourceValue(std::string(), "style", 0);
}

void AttributeSet::dump()const{
    for (size_t i = 0; i < getAttributeCount(); i++) {
        LOGD("[%zu] %s = %s", i, getAttributeName((int)i).c_str(), getAttributeValue((int)i).c_str());
    }
}

}
