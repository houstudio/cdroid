// Minimal C++14 port of the android::base::expected used by the AOSP
// androidfw sources (android-base/result.h) plus androidfw/Errors.h's
// IOError/NullOrIOError. Only the surface the LoadedArsc / AssetManager2 /
// AttributeResolution port exercises: construction, has_value/value/error,
// unexpected, monostate.
//
// Copyright (C) 2019 The Android Open Source Project
// Licensed under the Apache License, Version 2.0.
#ifndef __CDROID_ANDROIDFW_EXPECTED_H__
#define __CDROID_ANDROIDFW_EXPECTED_H__

#include <cassert>
#include <new>
#include <type_traits>
#include <utility>

namespace cdroid {
namespace base {

// std::nullopt_t / std::nullopt (C++17) stand-ins for the port.
struct nullopt_t {
    explicit constexpr nullopt_t(int) {}
};
constexpr nullopt_t nullopt{0};

// Fixed-layout optional. The tree's optional-lite orders its state flag
// differently under -std=gnu++17 (flag at the tail) than under C++14 (flag
// at offset 0), and its triviality flips as well — so any optional crossing
// a mixed -std TU boundary (the test suite links gtest, which demands C++17,
// against this C++14 library) is re-parsed with the wrong layout and reads
// as empty. The AM2 boundary faces return base::optional/base::expected, so
// this shim pins BOTH the layout (storage first, flag last) and the return
// ABI (user-provided special members keep it non-trivially copyable in every
// mode, i.e. always sret). Never make these defaulted.
template <typename T>
class optional {
public:
    optional() : has_value_(false) {}
    optional(nullopt_t) : has_value_(false) {}
    optional(const T& v) : has_value_(true) { construct(v); }
    optional(T&& v) : has_value_(true) { construct(std::move(v)); }
    optional(const optional& other) : has_value_(other.has_value_) {
        if (other.has_value_) construct(*other);
    }
    optional(optional&& other) : has_value_(other.has_value_) {
        if (other.has_value_) construct(std::move(*other));
    }
    optional& operator=(const optional& other) {
        if (this != &other) {
            reset();
            has_value_ = other.has_value_;
            if (other.has_value_) construct(*other);
        }
        return *this;
    }
    optional& operator=(optional&& other) {
        if (this != &other) {
            reset();
            has_value_ = other.has_value_;
            if (other.has_value_) construct(std::move(*other));
        }
        return *this;
    }
    optional& operator=(nullopt_t) {
        reset();
        return *this;
    }
    optional& operator=(const T& v) {
        reset();
        has_value_ = true;
        construct(v);
        return *this;
    }
    optional& operator=(T&& v) {
        reset();
        has_value_ = true;
        construct(std::move(v));
        return *this;
    }
    ~optional() { reset(); }

    bool has_value() const { return has_value_; }
    explicit operator bool() const { return has_value_; }
    bool operator!() const { return !has_value_; }

    const T& operator*() const { assert(has_value_); return *ptr(); }
    T& operator*() { assert(has_value_); return *ptr(); }
    const T* operator->() const { assert(has_value_); return ptr(); }
    T* operator->() { assert(has_value_); return ptr(); }
    const T& value() const { assert(has_value_); return *ptr(); }
    T& value() { assert(has_value_); return *ptr(); }

private:
    typedef typename std::aligned_storage<sizeof(T), alignof(T)>::type storage_t;

    storage_t storage_;
    bool has_value_;

    T* ptr() { return reinterpret_cast<T*>(&storage_); }
    const T* ptr() const { return reinterpret_cast<const T*>(&storage_); }
    template <typename U>
    void construct(U&& u) { new (&storage_) T(std::forward<U>(u)); }
    void reset() {
        if (has_value_) {
            ptr()->~T();
            has_value_ = false;
        }
    }
};

// std::monostate (C++17) stand-in for the port.
struct monostate {};

template <typename E>
class unexpected {
public:
    explicit unexpected(const E& e) : error_(e) {}
    const E& error() const { return error_; }
private:
    E error_;
};

namespace internal {

// Storage for either a T or an E. A union-free, C++14-safe pair: for the
// port's value types (PODs, pointers, monostate) copy/destroy trivially.
template <typename T, typename E>
struct expected_storage {
    bool has_value_ = true;
    T value_{};
    E error_{};
};

}  // namespace internal

template <typename T, typename E>
class expected {
public:
    static_assert(!std::is_same<T, E>::value, "T and E must differ");

    // Default: a value-initialized T (AOSP default-constructs expected<T> as
    // holding a value — e.g. `return {};` for monostate returns).
    expected() : storage_{} {}
    expected(const T& v) : storage_{} { storage_.value_ = v; }
    expected(unexpected<E> u) : storage_{} {
        storage_.has_value_ = false;
        storage_.error_ = u.error();
    }

    // User-provided (not defaulted) copy/move. This keeps the type NON
    // trivially copyable in every -std mode — optional-lite's optional<E>
    // (NullOrIOError) is trivially copyable only under C++17, so the implicit
    // ops made this 12-byte type register-returned under -std=gnu++17 but
    // sret-returned under -std=gnu++14: a silent cross-TU ABI mismatch
    // (caller/callee disagree on the hidden sret pointer, arguments shift by
    // one register). AOSP's base::expected is non-trivially copyable too
    // (union-based storage). Never default these.
    expected(const expected& other) : storage_(other.storage_) {}
    expected& operator=(const expected& other) {
        storage_ = other.storage_;
        return *this;
    }
    expected(expected&& other) : storage_(std::move(other.storage_)) {}
    expected& operator=(expected&& other) {
        storage_ = std::move(other.storage_);
        return *this;
    }

    bool has_value() const { return storage_.has_value_; }
    explicit operator bool() const { return has_value(); }
    bool operator!() const { return !has_value(); }

    const T& value() const {
        assert(has_value());
        return storage_.value_;
    }
    T& value() { assert(has_value()); return storage_.value_; }

    const T& operator*() const { return value(); }
    T& operator*() { return value(); }
    const T* operator->() const { return &value(); }
    T* operator->() { return &value(); }

    T value_or(T&& default_value) const {
        return has_value() ? value() : std::move(default_value);
    }

    const E& error() const {
        assert(!has_value());
        return storage_.error_;
    }

private:
    internal::expected_storage<T, E> storage_;
};

}  // namespace base

namespace androidfw {

// androidfw/Errors.h (the port's subset).
enum class IOError {
    PAGES_MISSING,
    PAGES_MISSING_ANOTHER_PROCESS,
};

// nullopt = "no such entry" (a NOT-FOUND answer), a value = an I/O failure.
using NullOrIOError = base::optional<IOError>;

template <typename T>
using IOErrorOr = base::expected<T, IOError>;

}  // namespace androidfw
}  // namespace cdroid

#endif  // __CDROID_ANDROIDFW_EXPECTED_H__
