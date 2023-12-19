#include<iostream>

template <class T>
class UniquePtr {
public:
    UniquePtr(): ptr(nullptr) { }
    UniquePtr(const UniquePtr& o) = delete;
    UniquePtr& operator=(const UniquePtr& o) = delete;

    UniquePtr(UniquePtr&& o) {
        ptr = o.ptr;
        o.ptr = nullptr;
    }

    UniquePtr& operator=(UniquePtr&& o) {
        if(this==std::addressof(o)){
            return *this;
        }
        delete ptr;
        UniquePtr tmp = UniquePtr(std::move(o));
        std::swap(this->ptr, tmp.ptr);
        return *this;
    }

    UniquePtr(T* p): ptr(p) {}

    T* operator->() {  return ptr; }

    T& operator*() {  return *ptr; }

    ~UniquePtr() {  delete ptr;  }

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
    SharedPtr():ptr(nullptr), counter(nullptr) {  }
    SharedPtr(const SharedPtr& o) {
        if(o.counter==nullptr){
            ptr = o.ptr;
            counter = nullptr;
        }else{
            ptr = o.ptr;
            counter = o.counter;
            (counter->strong)++;
        }
    }
    SharedPtr& operator=(const SharedPtr& o) {
        if(this==std::addressof(o)){
            return *this;
        }else{
            SharedPtr<T> tmp(o);
            std::swap(this->ptr, tmp.ptr);
            std::swap(this->counter, tmp.counter);
        }
        return *this;
    }

    SharedPtr(SharedPtr&& o) {
        ptr = o.ptr;
        counter = o.counter;
        o.ptr = nullptr;
        o.counter = nullptr;
    }

    SharedPtr& operator=(SharedPtr&& o) {
        if(this==std::addressof(o)){
            return *this;
        }

        (*this).~SharedPtr();
        SharedPtr<T> tmp(std::move(o));
        std::swap(this->ptr, tmp.ptr);
        std::swap(this->counter, tmp.counter);

        return *this;
    }

    SharedPtr(T* p)  {
        ptr = p;
        if (p!=nullptr) {
            counter = new RefCntBlock{1, 0};
        } else {
            counter = nullptr;
        }
    }


    SharedPtr(const WeakPtr<T>& o);


    void Reset() {
        ptr = nullptr;
        if (counter) {
            if (--counter->strong == 0) {
                delete ptr;
                if (counter->weak == 0) {
                    delete counter;
                }
            }
        }
        counter = nullptr;
    }

    T* operator->() { return ptr; }

    T& operator*() { return *ptr; }

    ~SharedPtr () {
        if (counter!=nullptr) {
            if (counter->strong - 1 == 0) {
                delete ptr;
                --counter->strong;
                if (counter->weak == 0) delete counter;
            }else{
                --counter->strong;
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
    WeakPtr() : ptr(nullptr), counter(nullptr) { }

    WeakPtr(const WeakPtr& o) : ptr(o.ptr), counter(o.counter) {
        ptr = o.ptr;
        counter = o.counter;
        if (counter != nullptr) {
            ++counter->weak;
        }
    }

    WeakPtr& operator=(const WeakPtr& o) {
        if(this==std::addressof(o)){
            return *this;
        }else{
            WeakPtr<T> tmp(o);
            std::swap(this->ptr, tmp.ptr);
            std::swap(this->counter, tmp.counter);
        }
        return *this;
    }

    WeakPtr(WeakPtr&& o)  {
        ptr = o.ptr;
        counter = o.counter;
        o.ptr = nullptr;
        o.counter = nullptr;
    }

    WeakPtr& operator=(WeakPtr&& o) {
        if(this==std::addressof(o)){
            return *this;
        }

        (*this).~WeakPtr();
        WeakPtr<T> tmp(std::move(o));
        std::swap(this->ptr, tmp.ptr);
        std::swap(this->counter, tmp.counter);

        return *this;
    }

    WeakPtr(const SharedPtr<T>& o){
        ptr = o.ptr;
        counter = o.counter;
        if (counter!=nullptr){
            (counter->weak)++;
        }
    }

    WeakPtr& operator=(const SharedPtr<T>& o) {
        (*this).~WeakPtr();

        ptr = o.ptr;
        counter = o.counter;

        if (counter!=nullptr){
            (counter->weak)++;
        }
        return *this;
    }


    void Reset() {
        ptr = nullptr;
        if (counter) {
            if (--counter->strong == 0) {
                delete ptr;
                if (counter->weak == 0) {
                    delete counter;
                }
            }
        }
        counter = nullptr;
    }

    bool Expired() const { return !counter || counter->strong == 0; }

    SharedPtr<T> Lock() { return Expired() ? SharedPtr<T>() : SharedPtr<T>(*this); }

    ~WeakPtr () {
        if (counter !=nullptr){
            counter->weak--;
            if(counter->weak==0 && counter->strong==0){
                delete counter;
            }
        }
    }

private:
    T* ptr;
    RefCntBlock* counter;
};

template <class T>
SharedPtr<T>::SharedPtr(const WeakPtr<T>& o){

    ptr = o.ptr;
    counter = o.counter;

    if (counter!=nullptr) {
        ++counter->strong;
    }
}