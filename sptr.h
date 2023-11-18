#include <iostream>

template <class T>
class UniquePtr {
public:
    UniquePtr() : ptr(nullptr) {}
    UniquePtr(const UniquePtr& o) = delete;
    UniquePtr& operator=(const UniquePtr& o) = delete;

    UniquePtr(UniquePtr&& o) {
        ptr = o.ptr;
        o.ptr = nullptr;
    }

    UniquePtr& operator=(UniquePtr&& o) {
        if (&o == this) {
            return *this;
        }
        delete ptr;
        ptr = o.ptr;
        o.ptr = nullptr;
        return *this;
    }

    UniquePtr(T* p) : ptr(p) {}

    T* operator->() { return ptr; }

    T& operator*() { return *ptr; }

    ~UniquePtr() { delete ptr; }

private:
    T* ptr;
};


// ================================================
struct RefCntBlock {
    size_t strong, weak;
};

template <class T>
class WeakPtr;

template <class T>
class SharedPtr {
    friend class WeakPtr<T>;
public:
    SharedPtr() : ptr(nullptr), counter(nullptr) {}
    SharedPtr(const SharedPtr& o) : ptr(o.ptr), counter(o.counter) { if (counter) ++counter->strong; }
    SharedPtr& operator=(const SharedPtr& o) {
        if (this != &o) {
            this->~SharedPtr();
            ptr = o.ptr;
            counter = o.counter;
            if (counter) ++counter->strong;
        }
        return *this;
    }

    SharedPtr(SharedPtr&& o) : ptr(o.ptr), counter(o.counter) {
        o.ptr = nullptr;
        o.counter = nullptr;
    }

    SharedPtr& operator=(SharedPtr&& o) {
        if (this != &o) {
            this->~SharedPtr();
            ptr = o.ptr;
            counter = o.counter;
            o.ptr = nullptr;
            o.counter = nullptr;
        }
        return *this;
    }

    SharedPtr(T* p) : ptr(p), counter(new RefCntBlock{1, 0}) {}

    SharedPtr(const WeakPtr<T>& o) : ptr(o.ptr), counter(o.counter) {
        if (counter) ++counter->strong;
    }

    void Reset() {
        this->~SharedPtr();
        ptr = nullptr;
        counter = nullptr;
    }

    T* operator->() { return ptr; }

    T& operator*() { return *ptr; }

    ~SharedPtr () {
        if (counter) {
            if (--counter->strong == 0) {
                delete ptr;
                if (counter->weak == 0) {
                    delete counter;
                }
            }
        }
    }

private:
    T* ptr;
    RefCntBlock* counter;
};

template <class T>
class WeakPtr {
    friend class SharedPtr<T>;
public:
    WeakPtr() : ptr(nullptr), counter(nullptr) {}
    WeakPtr(const WeakPtr& o) : ptr(o.ptr), counter(o.counter) { if (counter) ++counter->weak; }
    WeakPtr& operator=(const WeakPtr& o) {
        if (this != &o) {
            this->~WeakPtr();
            ptr = o.ptr;
            counter = o.counter;
            if (counter) ++counter->weak;
        }
        return *this;
    }

    WeakPtr(WeakPtr&& o) : ptr(o.ptr), counter(o.counter) {
        o.ptr = nullptr;
        o.counter = nullptr;
    }

    WeakPtr& operator=(WeakPtr&& o) {
        if (this != &o) {
            this->~WeakPtr();
            ptr = o.ptr;
            counter = o.counter;
            o.ptr = nullptr;
            o.counter = nullptr;
        }
        return *this;
    }

    WeakPtr(const SharedPtr<T>& o) : ptr(o.ptr), counter(o.counter) { if (counter) ++counter->weak; }

    WeakPtr& operator=(const SharedPtr<T>& o) {
        this->~WeakPtr();
        ptr = o.ptr;
        counter = o.counter;
        if (counter) ++counter->weak;
        return *this;
    }

    void Reset() {
        this->~WeakPtr();
        ptr = nullptr;
        counter = nullptr;
    }

    bool Expired() const { return !counter || counter->strong == 0; }

    SharedPtr<T> Lock() { return Expired() ? SharedPtr<T>() : SharedPtr<T>(*this); }

    ~WeakPtr () {
        if (counter && --counter->weak == 0 && counter->strong == 0) {
            delete counter;
        }
    }

private:
    T* ptr;
    RefCntBlock* counter;
};

