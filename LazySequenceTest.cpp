#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <exception>
#include <stdexcept> // Для std::out_of_range
#include <algorithm>
#include <utility>
#include <functional>

#include "LazySequence.h"
#include "ArraySequence.h"
#include "MutableArraySequence.h"
#include "SmartPointer.h"
#include "Cardinal.h"


int FibRule(Sequence<int>* seq) {
    ArraySequence<int>* arr = dynamic_cast<ArraySequence<int>*>(seq);
    if (!arr) return 1;
    size_t n = arr->GetLength();
    if (n == 0) return 1;
    if (n == 1) return 1;
    
    
    int a = arr->Get(static_cast<int>(n - 1));
    int b = arr->Get(static_cast<int>(n - 2));
    return a + b;
}

int NatRule(Sequence<int>* seq) {
    ArraySequence<int>* arr = dynamic_cast<ArraySequence<int>*>(seq);
    if (!arr) return 0;
    size_t n = arr->GetLength();
    if (n == 0) return 0;
    
    
    return arr->Get(static_cast<int>(n - 1)) + 1;
}


template <class T, class R>
R Reduce(const SharedPtr< LazySequenceBase<T> >& seq, std::function<R(R, T)> reducer, R initial) {
    Cardinal len = seq->GetLength();
    
    if (len.IsOmega()) {
        throw std::runtime_error("Cannot reduce an infinite (Omega) sequence.");
    }
    
    R accumulator = initial;
    size_t n = len.GetValue();
    
    for (size_t i = 0; i < n; ++i) {
        accumulator = reducer(accumulator, seq->Get(static_cast<int>(i)));
    }
    
    return accumulator;
}

static int tests_passed = 0;
static int tests_failed = 0;

#define RUN_TEST(name) do { \
    try { name(); std::cout << "[PASS] " #name "\n"; ++tests_passed; } \
    catch (const std::exception& e) { std::cout << "[FAIL] " #name " : " << e.what() << "\n"; ++tests_failed; } \
    catch (...) { std::cout << "[FAIL] " #name " : unknown exception\n"; ++tests_failed; } \
} while(0)


void test_concat_two_finite() {
    int a_buf[] = {1,2,3};
    int b_buf[] = {4,5};
    LazySequence<int> a(a_buf, 3);
    LazySequence<int> b(b_buf, 2);

    a.ConcatWith(b.GetRoot());

    Cardinal len = a.GetLength();
    if (len.IsOmega()) throw std::runtime_error("expected finite length");
    assert(len.GetValue() == 5);
    
    
    for (size_t i = 0; i < 5; ++i) {
        int v = a.Get(static_cast<int>(i));
        int expect = (i < 3) ? a_buf[i] : b_buf[i - 3];
        if (v != expect) throw std::runtime_error("value mismatch in concat_two_finite");
    }
}


void test_concat_finite_plus_infinite() {
    int a_buf[] = {10,20};
    LazySequence<int> a(a_buf, 2);
    LazySequence<int> b(NatRule, nullptr);
    

    a.ConcatWith(b.GetRoot());

    
    assert(a.Get(0) == 10);
    assert(a.Get(1) == 20);

    
    
    assert(a.Get(2) == 21);
}


void test_concat_infinite_plus_finite() {
    LazySequence<int> a(NatRule, nullptr); 
    int b_buf[] = {7,8,9};
    LazySequence<int> b(b_buf, 3);

    int a0 = a.Get(0); 
    a.ConcatWith(b.GetRoot());

    
    int after0 = a.Get(0);
    if (after0 != a0) throw std::runtime_error("infinite+finite: first element changed");

    int g1 = a.Get(1); 
    int g2 = a.Get(2); 
    if (g1 != 1 || g2 != 2) {
         throw std::runtime_error("infinite+finite: generator failed to continue NatRule");
    }
}


void test_where_out_of_range() {
    int a_buf[] = {1,2,3};
    LazySequence<int> a(a_buf, 3);
    auto root = a.GetRoot();
    auto filtered = root->Where([](int x)->bool { return (x % 2) == 0; }); 


    
    int v0 = filtered->Get(static_cast<int>(0));
    if (v0 != 2) throw std::runtime_error("where: first match wrong");

    
    bool threw = false;
    try {
        filtered->Get(static_cast<int>(1));
    } catch (const std::out_of_range&) {
        threw = true;
    }
    if (!threw) throw std::runtime_error("where: expected out_of_range for second element");
}


void test_map_basic() {
    int a_buf[] = {2,3,4};
    LazySequence<int> a(a_buf, 3);
    auto root = a.GetRoot();
    auto mapped = root->Map<int>([](int x)->int { return x * 10; });

    
    int v0 = mapped->Get(static_cast<int>(0));
    int v1 = mapped->Get(static_cast<int>(1));
    int v2 = mapped->Get(static_cast<int>(2));
    
    
    assert(v0 == 20 && v1 == 30 && v2 == 40);
    
    Cardinal len = mapped->GetLength();
    if (len.IsOmega()) throw std::runtime_error("map: expected finite length");
    if (len.GetValue() != 3) throw std::runtime_error("map: length mismatch");
}


void test_zip_two_finite() {
    int a_buf[] = {1,2,3};
    int b_buf[] = {10,20,30,40}; 
    LazySequence<int> a(a_buf, 3);
    LazySequence<int> b(b_buf, 4);

    auto za = a.GetRoot()->Zip<int>(b.GetRoot());
    Cardinal len = za->GetLength();
    if (len.IsOmega()) throw std::runtime_error("zip: unexpected infinite");
    if (len.GetValue() != 3) throw std::runtime_error("zip: length wrong");

    
    for (size_t i = 0; i < 3; ++i) {
        auto p = za->Get(static_cast<int>(i));
        if (p.first != a_buf[i] || p.second != b_buf[i]) throw std::runtime_error("zip: pair mismatch");
    }
}


void test_reduce_sum() {
    int a_buf[] = {5,6,7};
    LazySequence<int> a(a_buf, 3);
    auto root = a.GetRoot();
    int s = Reduce<int,int>(root, [](int acc, int v)->int { return acc + v; }, 0);
    if (s != 18) throw std::runtime_error("reduce: wrong sum");
}


void test_generator_fib() {
    LazySequence<int> fib(FibRule, nullptr); 
    int expect[] = {1,1,2,3,5,8,13};
    
    
    for (size_t i = 0; i < sizeof(expect)/sizeof(int); ++i) {
        int v = fib.Get(static_cast<int>(i));
        if (v != expect[i]) {
            throw std::runtime_error("fib: unexpected value at index " + std::to_string(i));
        }
    }
}


void test_materialized_count() {
    LazySequence<int> n(NatRule, nullptr);
    size_t before = n.GetMaterializedCount();
    
    
    (void) n.Get(static_cast<int>(2)); 
    size_t after = n.GetMaterializedCount();
    if (after <= before) throw std::runtime_error("materialized count did not increase");
}


void test_concatwith_preserve_generator() {
    int a_buf[] = {100};
    LazySequence<int> a(a_buf, 1);
    LazySequence<int> b(NatRule, nullptr); 
    a.ConcatWith(b.GetRoot());


    assert(a.Get(0) == 100);
    
    int v1 = a.Get(1);
    
    if (v1 != 101) throw std::runtime_error("concatwith_preserve_generator: expected continuation (101)");
}

int main() {
    std::cout << "Running LazySequence unit tests...\n";

    RUN_TEST(test_concat_two_finite);
    RUN_TEST(test_concat_finite_plus_infinite);
    RUN_TEST(test_concat_infinite_plus_finite);
    RUN_TEST(test_where_out_of_range);
    RUN_TEST(test_map_basic);
    RUN_TEST(test_zip_two_finite);
    RUN_TEST(test_reduce_sum);
    RUN_TEST(test_generator_fib);
    RUN_TEST(test_materialized_count);
    RUN_TEST(test_concatwith_preserve_generator);

    std::cout << "----------------------------------------\n";
    std::cout << "Tests passed: " << tests_passed << "\n";
    std::cout << "Tests failed: " << tests_failed << "\n";
    return (tests_failed == 0) ? 0 : 2;
}