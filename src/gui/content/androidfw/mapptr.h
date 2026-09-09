// Port of AOSP incfs map_ptr (system/incremental_delivery/incfs/util/include/
// util/map_ptr.h) for the LoadedArsc/AssetManager2 port.
//
// CDROID has no Incremental FS, so this is the AOSP non-__ANDROID__ behavior:
// map_ptr is a thin read-only pointer wrapper; "verification" is the null
// check (AOSP: verify() returns true off incfs — `if (map_ == nullptr) return
// ptr_ != nullptr`). All data-integrity checking lives where AOSP puts it, in
// ChunkIterator::VerifyNextChunk's chunk size validation. Names and the API
// surface (convert/offset/iterator()/verified()/value()) match the original so
// the LoadedArsc port stays line-for-line.
//
// Copyright (C) 2020 The Android Open Source Project
// Licensed under the Apache License, Version 2.0.
#ifndef __CDROID_ANDROIDFW_MAP_PTR_H__
#define __CDROID_ANDROIDFW_MAP_PTR_H__

#include <cstddef>
#include <iterator>
#include <type_traits>

namespace cdroid {
namespace incfs {

template <typename T, bool Verified = false>
struct map_ptr;

// Variant of map_ptr that statically guarantees that the pointed to data is fully present and
// reading data will not result in IncFs raising a SIGBUS.
template <typename T>
using verified_map_ptr = map_ptr<T, true>;

template <typename T, bool Verified>
struct map_ptr final {
private:
    template <typename, bool>
    friend struct map_ptr;

    template <typename T1>
    using IsVoid = typename std::enable_if<std::is_void<T1>::value, int>::type;

    template <typename T1>
    using NotVoid = typename std::enable_if<!std::is_void<T1>::value, int>::type;

    template <bool V>
    using IsVerified = typename std::enable_if<V, int>::type;

    template <bool V>
    using IsUnverified = typename std::enable_if<!V, int>::type;

public:
    class const_iterator final {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = const map_ptr<T>;
        using difference_type = std::ptrdiff_t;
        using pointer = void;
        using reference = value_type;

        const_iterator() = default;
        const_iterator(const const_iterator& it) = default;

        bool operator==(const const_iterator& other) const { return safe_ptr_ == other.safe_ptr_; }
        bool operator!=(const const_iterator& other) const { return safe_ptr_ != other.safe_ptr_; }
        std::ptrdiff_t operator-(const const_iterator& other) const {
            return safe_ptr_ - other.safe_ptr_;
        }

        const_iterator operator+(int n) const {
            const_iterator other = *this;
            other += n;
            return other;
        }

        reference operator*() const { return safe_ptr_; }

        bool operator<(const const_iterator& other) const { return safe_ptr_ < other.safe_ptr_; }

        const const_iterator& operator--() {
            --safe_ptr_;
            return *this;
        }

        const const_iterator operator--(int) {
            const_iterator temp(*this);
            --safe_ptr_;
            return temp;
        }

        const const_iterator& operator++() {
            ++safe_ptr_;
            return *this;
        }

        const_iterator& operator+=(int n) {
            safe_ptr_ = safe_ptr_ + n;
            return *this;
        }

        const const_iterator operator++(int) {
            const_iterator temp(*this);
            ++safe_ptr_;
            return temp;
        }

    private:
        friend struct map_ptr<T, Verified>;
        explicit const_iterator(const map_ptr<T>& ptr) : safe_ptr_(ptr) {}
        map_ptr<T> safe_ptr_;
    };

    // Default constructor
    map_ptr() = default;

    // Implicit conversion from raw pointer
    map_ptr(const T* ptr) : ptr_(ptr) {}

    // Copy constructor
    map_ptr(const map_ptr& other) = default;

    // Move constructor
    map_ptr(map_ptr&& other) noexcept = default;

    // Implicit copy conversion from verified to unverified map_ptr<T>
    template <bool V2, bool V1 = Verified, IsUnverified<V1> = 0, IsVerified<V2> = 0>
    map_ptr(const map_ptr<T, V2>& other) : ptr_(other.ptr_) {}

    // Implicit move conversion from verified to unverified map_ptr<T>
    template <bool V2, bool V1 = Verified, IsUnverified<V1> = 0, IsVerified<V2> = 0>
    map_ptr(map_ptr<T, V2>&& other) : ptr_(other.ptr_) {}

    // Implicit conversion to unverified map_ptr<void>
    template <typename U, bool V2, typename T1 = T, bool V1 = Verified, IsVoid<T1> = 0,
              NotVoid<U> = 0, IsUnverified<V1> = 0>
    map_ptr(const map_ptr<U, V2>& other)
          : ptr_(reinterpret_cast<const void*>(other.ptr_)) {}

    // Implicit conversion from regular raw pointer
    map_ptr& operator=(const T* ptr) {
        ptr_ = ptr;
        return *this;
    }

    // Copy assignment operator
    map_ptr& operator=(const map_ptr& other) = default;

    map_ptr& operator=(map_ptr&& other) = default;

    template <bool V2>
    bool operator==(const map_ptr<T, V2>& other) const {
        return ptr_ == other.ptr_;
    }

    template <bool V2>
    bool operator!=(const map_ptr<T, V2>& other) const {
        return ptr_ != other.ptr_;
    }

    template <bool V2>
    bool operator<(const map_ptr<T, V2>& other) const {
        return ptr_ < other.ptr_;
    }

    template <bool V2>
    std::ptrdiff_t operator-(const map_ptr<T, V2>& other) const {
        return ptr_ - other.ptr_;
    }

    template <typename U>
    map_ptr<U> convert() const {
        return map_ptr<U>(reinterpret_cast<const U*>(ptr_));
    }

    // Retrieves a map_ptr<T> offset from an original map_ptr<U> by the specified number of `offset`
    // bytes.
    map_ptr<T> offset(std::ptrdiff_t offset) const {
        return map_ptr<T>(reinterpret_cast<const T*>(
                reinterpret_cast<const uint8_t*>(ptr_) + offset));
    }

    // Returns a raw pointer to the value of this pointer.
    const T* unsafe_ptr() const { return ptr_; }

    // Start T == void methods

    template <typename T1 = T, IsVoid<T1> = 0>
    operator bool() const {
        return ptr_ != nullptr;
    }

    // End T == void methods
    // Start T != void methods

    template <typename T1 = T, NotVoid<T1> = 0, bool V1 = Verified, IsUnverified<V1> = 0>
    operator bool() const {
        return verify();
    }

    template <typename T1 = T, NotVoid<T1> = 0, bool V1 = Verified, IsVerified<V1> = 0>
    operator bool() const {
        return ptr_ != nullptr;
    }

    template <typename T1 = T, NotVoid<T1> = 0>
    const_iterator iterator() const {
        return const_iterator(*this);
    }

    template <typename T1 = T, NotVoid<T1> = 0>
    const map_ptr<T1>& operator++() {
        ++ptr_;
        return *this;
    }

    template <typename T1 = T, NotVoid<T1> = 0>
    const map_ptr<T1> operator++(int) {
        map_ptr<T1> temp = *this;
        ++ptr_;
        return temp;
    }

    template <typename T1 = T, NotVoid<T1> = 0>
    const map_ptr<T1>& operator--() {
        --ptr_;
        return *this;
    }

    template <typename T1 = T, NotVoid<T1> = 0>
    const map_ptr<T1> operator--(int) {
        map_ptr<T1> temp = *this;
        --ptr_;
        return temp;
    }

    template <typename S, typename T1 = T, NotVoid<T1> = 0>
    map_ptr<T1> operator+(const S n) const {
        return map_ptr<T1>(ptr_ + n);
    }

    template <typename S, typename T1 = T, NotVoid<T1> = 0>
    map_ptr<T1> operator-(const S n) const {
        return map_ptr<T1>(ptr_ - n);
    }

    // Returns the value of the pointer.
    // The caller should verify the presence of the pointer data before calling this method.
    template <typename T1 = T, NotVoid<T1> = 0>
    const T1& value() const {
        return *ptr_;
    }

    // Returns a raw pointer to the value this pointer.
    // The caller should verify the presence of the pointer data before calling this method.
    template <typename T1 = T, NotVoid<T1> = 0>
    const T1* operator->() const {
        return ptr_;
    }

    // Verifies the presence of `n` elements of `T`.
    //
    // Returns true if the elements are completely present; otherwise, returns false.
    template <typename T1 = T, NotVoid<T1> = 0, bool V1 = Verified, IsUnverified<V1> = 0>
    bool verify(size_t /*n*/ = 1) const {
        // AOSP non-incfs path: with no IncFsFileMap behind the pointer, the
        // data is always present — only the null state fails.
        return ptr_ != nullptr;
    }

    // Returns a verified version of this pointer.
    // The caller should verify the presence of the pointer data before calling this method.
    template <typename T1 = T, NotVoid<T1> = 0>
    verified_map_ptr<T1> verified() const {
        return verified_map_ptr<T1>(ptr_);
    }

private:
    const T* ptr_ = nullptr;
};

}  // namespace incfs
}  // namespace cdroid

#endif  // __CDROID_ANDROIDFW_MAP_PTR_H__
