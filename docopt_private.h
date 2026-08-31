#pragma once

namespace docoptcpp03 {
namespace detail {

template <typename T>
class shared_ptr {
private:
    T* px;

public:
    shared_ptr() : px(0) {}

    explicit shared_ptr(T* p) : px(p) {
        if (px) px->add_ref();
    }

    ~shared_ptr() {
        if (px) px->release_ref();
    }

    shared_ptr(const shared_ptr& r) : px(r.px) {
        if (px) px->add_ref();
    }

    shared_ptr& operator=(const shared_ptr& r) {
        T* new_px = r.px;
        if (new_px) new_px->add_ref();
        if (px) px->release_ref();
        px = new_px;
        return *this;
    }

    T& operator*() const { return *px; }
    T* operator->() const { return px; }
    T* get() const { return px; }

    typedef T* shared_ptr::*unspecified_bool_type;
    operator unspecified_bool_type() const { return px ? &shared_ptr::px : 0; }
    bool operator!() const { return px == 0; }

    bool operator==(const shared_ptr& r) const { return px == r.px; }
    bool operator!=(const shared_ptr& r) const { return px != r.px; }
    bool operator<(const shared_ptr& r) const { return px < r.px; }
};

} // namespace detail
} // namespace docoptcpp03
