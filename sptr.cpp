#include <vector>
#include <iostream>
#include "catch.hpp"
#include "sptr.h"
#include <random>  
#include <ctime>  

class Awful {
public:
    Awful() = delete;
    Awful(const Awful&) = delete;
    Awful(Awful&&) = delete;
    Awful& operator=(const Awful&) = delete;
    Awful& operator=(Awful&&) = delete;
    
    friend Awful* make_awful(int64_t v);
    
    void Inc() {
        ++d;
    }
    
    void Dec() {
        --d;
    }
    
    int64_t Get() {
        return d;
    }
private:
    int64_t d;
    Awful(int64_t arg) : d(arg) {}
};

Awful* make_awful(int64_t v) {
    return new Awful(v);
}

TEST_CASE ( "Basic types", "[unique_ptr]" ) {
    UniquePtr<int64_t> p(new int64_t(25));
    
    REQUIRE(*p == 25);
    
    *p += 4;
    REQUIRE(*p == 29);
    
    UniquePtr<int64_t> q(new int64_t(21));
    
    REQUIRE(*p + *q == 50);
}

TEST_CASE ( "More complex types", "[unique_ptr]" ) {
    UniquePtr<std::vector<int64_t>> p(new std::vector<int64_t>(5));
    REQUIRE(p->size() == 5);
    
    p->resize(13);
    REQUIRE(p->size() == 13);
    
    p->clear();
    p->push_back(1);
    p->push_back(2);
    p->push_back(3);
    p->push_back(4);

    REQUIRE(p->size() == 4);
    REQUIRE((*p).size() == 4);
    REQUIRE(p->back() == 4);
    REQUIRE((*p).back() == 4);
}

TEST_CASE ( "Hostile types" , "[unique_ptr]" ) {
    UniquePtr<Awful> p(make_awful(4));
    REQUIRE(p->Get() == 4);
    p->Inc();
    p->Inc();
    p->Inc();
    REQUIRE(p->Get() == 7);
    
    UniquePtr q(std::move(p));
    REQUIRE(q->Get() == 7);
    q->Dec();
    q->Dec();
    REQUIRE(q->Get() == 5);
    
    p = std::move(q);
    REQUIRE(p->Get() == 5);
    
}

TEST_CASE ( "Basic types for shared ptr", "[shared_ptr]" ) {
    SharedPtr<int64_t> p(new int64_t(25));
    
    REQUIRE(*p == 25);
    
    *p += 4;
    REQUIRE(*p == 29);
    
    SharedPtr<int64_t> q(new int64_t(21));
    
    REQUIRE(*p + *q == 50);
    
    q = p;
    
    REQUIRE (*p + *q == 58);
    
    p = q;
    q = p;
    
    REQUIRE(*p + *q == 58);
}

TEST_CASE ( "Copying shared ptr", "[shared_ptr]" ) {
    SharedPtr<int64_t>* p = new SharedPtr<int64_t>(new int64_t(25));
    SharedPtr<int64_t>* q = new SharedPtr<int64_t>(*p);
    SharedPtr<int64_t>* r = new SharedPtr<int64_t>(*q);
    
    delete p;
    delete q;
    
    REQUIRE(*(*r) == 25);
    
    delete r;
}

TEST_CASE ( "Moving shared ptr", "[shared_ptr]" ) {
    SharedPtr<int64_t> p(new int64_t(25));
    SharedPtr<int64_t> q = std::move(p);
    SharedPtr<int64_t> r = std::move(q);
    
    p.Reset();
    q.Reset();
    
    REQUIRE((*r) == 25);
}

TEST_CASE ( "Hostile types with shared ptr" , "[shared_ptr]" ) {
    SharedPtr<Awful> p(make_awful(4));
    REQUIRE(p->Get() == 4);
    p->Inc();
    p->Inc();
    p->Inc();
    REQUIRE(p->Get() == 7);
    
    SharedPtr q(std::move(p));
    REQUIRE(q->Get() == 7);
    q->Dec();
    q->Dec();
    REQUIRE(q->Get() == 5);
    
    p = std::move(q);
    REQUIRE(p->Get() == 5);
    
    q = p;
    REQUIRE(p->Get() == 5);
    q->Inc();
    REQUIRE(p->Get() == 6);
    p->Dec();
    REQUIRE(q->Get() == 5);
    
}

TEST_CASE ( "Weak ptr basics", "[shared_ptr]" ) {
    SharedPtr<int64_t> p(new int64_t(1));
    WeakPtr w(p);
    
    REQUIRE(*p == 1);
    REQUIRE(*(w.Lock()) == 1);
    
    {
        SharedPtr<int64_t> q(new int64_t(2));
        p = q;
        w = q;
    }
    
    REQUIRE(*p == 2);
    REQUIRE(*(w.Lock()) == 2);
}

TEST_CASE ( "Copying weak ptr", "[shared_ptr]") {
    SharedPtr<int64_t> p(new int64_t(1));
    SharedPtr<int64_t> q(new int64_t(2));
    
    WeakPtr<int64_t> v(p), w(q);
    
    {
        WeakPtr<int64_t> t(v);
        v = w;
        w = t;
    }
    
    REQUIRE(*p == 1);
    REQUIRE(*q == 2);
    
    REQUIRE(*(v.Lock()) == 2);
    REQUIRE(*(w.Lock()) == 1);
}

TEST_CASE ( "Moving weak ptr", "[shared_ptr]") {
    SharedPtr<int64_t> p(new int64_t(1));
    SharedPtr<int64_t> q(new int64_t(2));
    
    WeakPtr<int64_t> v(p), w(q);
    
    {
        WeakPtr<int64_t> t(std::move(v));
        v = std::move(w);
        w = std::move(t);
    }
        
    REQUIRE(*p == 1);
    REQUIRE(*q == 2);
    
    REQUIRE(*(v.Lock()) == 2);
    REQUIRE(*(w.Lock()) == 1);
}

TEST_CASE ( "Weak ptr expires", "[shared_ptr]" ) {
    SharedPtr<int64_t> p(new int64_t(1));
    WeakPtr<int64_t> v(p);
    
    SharedPtr<int64_t> q(new int64_t(2));
    p = q;
    
    REQUIRE(*p == 2);
    REQUIRE(*q == 2);
    REQUIRE(v.Expired());
    
    v = q;
    REQUIRE(!v.Expired());
    REQUIRE(*(v.Lock()) == 2);
}
