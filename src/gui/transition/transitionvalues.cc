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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA.
 *********************************************************************************/
#include <transition/transitionvalues.h>

#include <view/view.h>
#include <core/rect.h> // Rect / RectF / PointF

#include <algorithm>
#include <cstdint>
#include <functional>
#include <sstream>
#include <typeinfo>
#include <vector>

namespace cdroid {

// Type-dispatched equality for the nonstd::any values stored in
// TransitionValues#values. android stores Object and compares with Object#equals;
// CDROID dispatches on the std::type_info of the held value for the types the
// transitions actually capture (int/long/float/double/bool/string,
// Rect/RectF/PointF, View*, int[]/float[]). Types are checked with any_cast<T>
// only after a.type()==b.type()==typeid(T), so the casts cannot throw. Exposed
// (not file-local) so Transition::isValueChanged can reuse it.
bool anyValuesEqual(const nonstd::any& a, const nonstd::any& b) {
    if (a.has_value() != b.has_value()) return false;
    if (!a.has_value()) return true;
    if (a.type() != b.type()) return false;
    const std::type_info& t = a.type();
    if (t == typeid(int))         return nonstd::any_cast<int>(a) == nonstd::any_cast<int>(b);
    if (t == typeid(long))        return nonstd::any_cast<long>(a) == nonstd::any_cast<long>(b);
    if (t == typeid(int64_t))     return nonstd::any_cast<int64_t>(a) == nonstd::any_cast<int64_t>(b);
    if (t == typeid(float))       return nonstd::any_cast<float>(a) == nonstd::any_cast<float>(b);
    if (t == typeid(double))      return nonstd::any_cast<double>(a) == nonstd::any_cast<double>(b);
    if (t == typeid(bool))        return nonstd::any_cast<bool>(a) == nonstd::any_cast<bool>(b);
    if (t == typeid(std::string)) return nonstd::any_cast<std::string>(a) == nonstd::any_cast<std::string>(b);
    if (t == typeid(Rect))        return nonstd::any_cast<Rect>(a) == nonstd::any_cast<Rect>(b);
    if (t == typeid(RectF))       return nonstd::any_cast<RectF>(a) == nonstd::any_cast<RectF>(b);
    if (t == typeid(PointF))      return nonstd::any_cast<PointF>(a) == nonstd::any_cast<PointF>(b);
    if (t == typeid(View*))       return nonstd::any_cast<View*>(a) == nonstd::any_cast<View*>(b);
    if (t == typeid(std::vector<int>)) {
        // Pointer form: value-form any_cast cannot take a reference ValueType,
        // and the pointer form avoids copying the vector.
        const std::vector<int>* x = nonstd::any_cast<std::vector<int>>(&a);
        const std::vector<int>* y = nonstd::any_cast<std::vector<int>>(&b);
        return x->size() == y->size() && std::equal(x->begin(), x->end(), y->begin());
    }
    if (t == typeid(std::vector<float>)) {
        const std::vector<float>* x = nonstd::any_cast<std::vector<float>>(&a);
        const std::vector<float>* y = nonstd::any_cast<std::vector<float>>(&b);
        return x->size() == y->size() && std::equal(x->begin(), x->end(), y->begin());
    }
    // Unknown held type: conservative — treat as not equal (matches android's
    // Object#equals falling back to reference equality for uncatalogued types).
    return false;
}

namespace {

// Best-effort stringification of a captured value for toString() (debug only).
std::string anyToString(const nonstd::any& a) {
    if (!a.has_value()) return "null";
    const std::type_info& t = a.type();
    std::ostringstream oss;
    if (t == typeid(int))           oss << nonstd::any_cast<int>(a);
    else if (t == typeid(long))     oss << nonstd::any_cast<long>(a);
    else if (t == typeid(int64_t))  oss << nonstd::any_cast<int64_t>(a);
    else if (t == typeid(float))    oss << nonstd::any_cast<float>(a);
    else if (t == typeid(double))   oss << nonstd::any_cast<double>(a);
    else if (t == typeid(bool))     oss << (nonstd::any_cast<bool>(a) ? "true" : "false");
    else if (t == typeid(std::string)) oss << nonstd::any_cast<std::string>(a);
    else                            oss << t.name();
    return oss.str();
}

} // anonymous namespace

TransitionValues::TransitionValues(View* v) : view(v) {
}

bool TransitionValues::equals(const TransitionValues& other) const {
    // android: other instanceof TransitionValues && view == other.view && values.equals(other.values)
    if (view != other.view) return false;
    if (values.size() != other.values.size()) return false;
    for (const auto& kv : values) {
        auto it = other.values.find(kv.first);
        if (it == other.values.end()) return false;
        if (!anyValuesEqual(kv.second, it->second)) return false;
    }
    return true;
}

int TransitionValues::hashCode() const {
    // android: 31 * view.hashCode() + values.hashCode(). view#hashCode is Java's
    // identity hash; CDROID hashes the pointer value. values#hashCode combines
    // entry hashes — here key hash XOR held-type hash (consistent with equals).
    std::hash<std::string> hashString;
    std::intptr_t viewHash = reinterpret_cast<std::intptr_t>(view);
    int valuesHash = 0;
    for (const auto& kv : values) {
        const size_t typeHash = kv.second.has_value() ? kv.second.type().hash_code() : 0;
        valuesHash = 31 * valuesHash + (int)(hashString(kv.first) ^ typeHash);
    }
    return 31 * (int)viewHash + valuesHash;
}

std::string TransitionValues::toString() const {
    std::ostringstream oss;
    oss << "TransitionValues@" << std::hex << hashCode() << ":\n";
    oss << "    view = " << view << "\n";
    oss << "    values:";
    for (const auto& kv : values) {
        oss << "    " << kv.first << ": " << anyToString(kv.second) << "\n";
    }
    return oss.str();
}

} // namespace cdroid
