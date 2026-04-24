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
#ifndef __CDPLOT_UTILITY_H__
#define __CDPLOT_UTILITY_H__

#include <cmath>
#include <cstdint>
#include <functional>
#include <iterator>
#include <list>
#include <set>
#include <vector>

namespace plotcpp {

/** Internal real number type */
using Real = double;

static constexpr int32_t BORDER_COLOR=0x80808080;
namespace adaptor {
/**
 * @brief Convert any numeric type to the internal real representation
 *
 * @tparam T Custom numeric type
 * @param x Value
 * @return Real Conversion from T to Real
 */
template <typename T> Real Real(T x) {
    return static_cast<::plotcpp::Real>(x);
}

/**
 * @brief Convert a vector of any numeric type to the internal real
 * representation
 *
 * @tparam T Custom numeric type
 * @param v Vector
 * @return std::vector<Real> Conversion from T to Real
 */
template <typename T> std::vector<::plotcpp::Real> Real(const std::vector<T>& v) {
    const size_t size = v.size();

    std::vector<::plotcpp::Real> real_vector;
    real_vector.resize(size);

    for (size_t i = 0; i < size; ++i) {
        real_vector[i] = Real(v[i]);
    }

    return real_vector;
}

} // namespace adaptor

namespace ranges {

template <typename T> using Interval = std::pair<T, T>;

/**
 * @brief Returns a vector of Real numbers from a to b in equally spaced
 * intervals except, maybe, the last interval.
 *
 * @tparam T Custom numeric type
 */
template <typename T> std::vector<T> MakeRange(T start, T end, T step) {
    const bool wrong_direction =
        ((start < end) && (step <= 0)) || ((start > end) && (step >= 0));
    if (wrong_direction) {
        return {};
    }

    static const auto less = [](T a, T b) {
        return a < b;
    };
    static const auto greater = [](T a, T b) {
        return a > b;
    };
    std::function<bool(T, T)> cmp;
    if (start <= end) {
        cmp = less;
    } else {
        cmp = greater;
    }

    std::vector<T> range;
    range.reserve(
        1 + static_cast<size_t>(std::ceil(std::abs((end - start) / step))));

    while (cmp(start, end)) {
        range.push_back(start);
        start += step;
    }

    // Add end
    if (range.back() != end) {
        range.push_back(end);
    }

    return range;
}

/**
 * @brief Generates a vector of Real y as a function of a vector of Real x.
 *
 * @param x Input vector
 * @param function A function f such that y=f(x)
 * @return A vector y such that y=function(x)
 */
template <typename T>
std::vector<T> Generate(const std::vector<T> &x,
                        const std::function<T(T)> &function) {
    const size_t size = x.size();
    if (size == 0) {
        return {};
    }

    std::vector<T> y;
    y.resize(size);

    for (size_t i = 0; i < size; ++i) {
        y[i] = function(x[i]);
    }

    return y;
}

/**
 * @brief Partitions a range into a number of intermediate values
 * from the minumum value to the maximum value.
 */
template <typename T>
std::set<T> TrivialPartitionRange(const Interval<T> &range,
                                  unsigned int num_markers) {
    std::set<T> values;

    const T min = std::min(range.first, range.second);
    const T max = std::max(range.first, range.second);
    const T interval = (max - min) / (num_markers - 1);

    if (interval == 0) {
        return {min, max};
    }

    for (T marker = min; marker <= max; marker += interval) {
        values.insert(marker);
    }

    return values;
}

/**
 * @brief Partitions a range into a number of intermediate values
 * showing relevant values.
 */
template <typename T>
std::set<T> PartitionRange(const Interval<T> &range, unsigned int num_markers) {
    // TODO: Implement algorithm
    return TrivialPartitionRange(range, num_markers);
}

} // namespace ranges

template <typename T>
std::pair<size_t, bool>
BinarySearchInterval(T value, const std::vector<T> &intervals) {
    const size_t num_intervals = intervals.size();

    if (num_intervals == 0) {
        return {0, false};
    } else if ((num_intervals == 1) && (value == intervals.front())) {
        return {0, true};
    }

    if ((value < intervals.front()) || (value > intervals.back())) {
        return {0, false};
    }

    size_t low = 0;
    size_t high = num_intervals - 1;
    size_t index = 0;
    while (low != high) {
        index = (low + high) / 2;

        if ((value >= intervals[index]) && (value <= intervals[index + 1])) {
            break;
        }

        if (value > intervals[index + 1]) {
            low = index;
        } else {
            high = index;
        }
    }

    return {index, true};
}

namespace color_tables {

const std::vector<uint32_t> BRIGHT{
    0xFF4477AA, 0xFFEE6677, 0xFF228833, 0xFFCCBB44,
    0xFF66CCEE, 0xFFAA3377, 0xFFBBBBBB};

const std::vector<uint32_t> VIBRANT{
    0xFFEE7733, 0xFF0077BB, 0xFF33BBEE, 0xFFEE3377,
    0xCC3311, 0xFF009988, 0xFFBBBBBB};

const std::vector<uint32_t> MUTED{
    0xFFCC6677, 0xFF332288, 0xFFDDCC77, 0xFF117733,
    0xFF88CCEE, 0xFF882255, 0xFF44AA99, 0xFF999933};

const std::vector<uint32_t> LIGHT{
    0xFF77AADD, 0xFFEE8866, 0xFFEEDD88,
    0xFFFFAABB, 0xFF99DDFF, 0xFF44BB99,
    0xFFBBCC33, 0xFFAAAA00, 0xFFDDDDDD};

} // namespace color_tables

class ColorSelector final {
public:
    ColorSelector(const std::vector<uint32_t> &table) : m_table(table) {}
    uint32_t NextColor() {
        uint32_t color = m_table[index];
        index = (index + 1) % m_table.size();
        return color;
    }
private:
    const std::vector<uint32_t> m_table;

    size_t index = 0;
};

} // namespace plotcpp

#endif // __CDPLOT_UTILITY_H__
