#pragma once
#include "main.h"

void slp(double s){
    s*=1000000;
    usleep(s);
}

uint64_t now_ms() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(
            system_clock::now().time_since_epoch()
    ).count();
}

#define biHashtablevars(x) x(hashtable(T, U), forward) x(hashtable(T, U), backward)
template<typename T, typename U>
classDefinition(biHashtable, biHashtablevars, macroVoid)
    void bind(const T& k, const U& v) {
        if (auto it = forward.find(k); it != forward.end()) backward.erase(it->second);
        if (auto it = backward.find(v); it != backward.end()) forward.erase(it->second);
        forward[k] = v;
        backward[v] = k;
    }

    struct FProxy {
        biHashtable* self; T k;
        operator U() const {
            auto it = self->forward.find(k);
            return (it == self->forward.end()) ? U{} : it->second;
        }
        FProxy& operator=(const U& v) { self->bind(k, v); return *this; }
    };

    struct BProxy {
        biHashtable* self; U v;
        operator T() const {
            auto it = self->backward.find(v);
            return (it == self->backward.end()) ? T{} : it->second;
        }
        BProxy& operator=(const T& k) { self->bind(k, v); return *this; }
    };

    FProxy operator[](const T& k) { return FProxy{this, k}; }
    BProxy operator[](const U& v) { return BProxy{this, v}; }
endClassT(biHashtable)

//registerClassT(biHashtable, int, double)

//Wrap variable with this to automatically manage mutex during performing the variable's method
template<class T, class Mutex = std::mutex>
class AutoLock {
public:
    AutoLock() = default;

    template<class... Args>
    explicit AutoLock(std::in_place_t, Args&&... args)
            : value_(std::forward<Args>(args)...) {}

    AutoLock(const AutoLock&) = delete;
    AutoLock& operator=(const AutoLock&) = delete;

    struct Guard {
        std::unique_lock<Mutex> lk;
        T* p;
        T* operator->() noexcept { return p; }
        T& operator*()  noexcept { return *p; }
    };
    Guard operator->() { return Guard{std::unique_lock<Mutex>(mtx_), &value_}; }

    struct GuardConst {
        std::unique_lock<Mutex> lk;
        const T* p;
        const T* operator->() const noexcept { return p; }
        const T& operator*()  const noexcept { return *p; }
    };
    GuardConst operator->() const { return GuardConst{std::unique_lock<Mutex>(mtx_), &value_}; }

    template<class Elem>
    struct ElemProxy {
        std::unique_lock<Mutex> lk;
        Elem* p;

        operator Elem() const { return *p; }

        template<class U>
        ElemProxy& operator=(U&& u) {
            *p = std::forward<U>(u);
            return *this;
        }

        template<class U> ElemProxy& operator+=(U&& u) { *p += std::forward<U>(u); return *this; }
        template<class U> ElemProxy& operator-=(U&& u) { *p -= std::forward<U>(u); return *this; }
        template<class U> ElemProxy& operator*=(U&& u) { *p *= std::forward<U>(u); return *this; }
        template<class U> ElemProxy& operator/=(U&& u) { *p /= std::forward<U>(u); return *this; }

        ElemProxy& operator++() { ++(*p); return *this; }
        Elem operator++(int) { Elem old = *p; ++(*p); return old; }
        ElemProxy& operator--() { --(*p); return *this; }
        Elem operator--(int) { Elem old = *p; --(*p); return old; }
    };

    template<class Index>
    auto operator[](Index&& i) {
        using Ref = decltype(value_[std::forward<Index>(i)]);
        using Elem = std::remove_reference_t<Ref>;
        auto lk = std::unique_lock<Mutex>(mtx_);
        Ref r = value_[std::forward<Index>(i)];
        return ElemProxy<Elem>{std::move(lk), std::addressof(r)};
    }

    template<class Index>
    auto operator[](Index&& i) const {
        using Ref = decltype(value_[std::forward<Index>(i)]);
        using Elem = std::remove_reference_t<Ref>;
        auto lk = std::unique_lock<Mutex>(mtx_);
        Ref r = value_[std::forward<Index>(i)];
        return ElemProxy<const Elem>{std::move(lk), std::addressof(r)};
    }

private:
    mutable Mutex mtx_;
    T value_{};
};
