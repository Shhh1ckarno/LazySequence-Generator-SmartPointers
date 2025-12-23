#include <iostream>
#include <fstream>
#include <chrono>
#include <utility> 
#include "SmartPointer.h"      
#include "MutableArraySequence.h" 

using namespace std;
using namespace std::chrono;


using ObjectCount = long long; 


void test_unique_ptr() {
    cout << "=== Testing UniquePtr ===\n";

    UniquePtr<MutableArraySequence<int>> seq(MakeUnique<MutableArraySequence<int>>());
    seq->Append(10);
    seq->Append(20);
    cout << "First: " << seq->GetFirst() << ", Last: " << seq->GetLast() << "\n";


    UniquePtr<MutableArraySequence<int>> seq2(std::move(seq));
    if (!seq) cout << "seq is empty after move\n";
    cout << "seq2 first: " << seq2->GetFirst() << ", last: " << seq2->GetLast() << "\n";
}

void test_shared_ptr() {
    cout << "=== Testing SharedPtr ===\n";

    SharedPtr<MutableArraySequence<int>> sp_seq(MakeShared<MutableArraySequence<int>>());
    sp_seq->Append(100);
    sp_seq->Append(200);

    SharedPtr<MutableArraySequence<int>> sp_seq2 = sp_seq;
    cout << "use_count after copy: " << sp_seq.use_count() << "\n";
    cout << "sp_seq2 first: " << sp_seq2->GetFirst() << ", last: " << sp_seq2->GetLast() << "\n";


    SharedPtr<MutableArraySequence<int>> sp_seq3 = std::move(sp_seq2);
    cout << "use_count after move: " << sp_seq.use_count() << "\n";

    sp_seq.reset();
    cout << "use_count after reset sp_seq: " << sp_seq3.use_count() << "\n";

    sp_seq3->Append(300);
    cout << "sp_seq3 last element: " << sp_seq3->GetLast() << "\n";
}


void benchmark_object_lifecycle(ObjectCount n, ofstream& out) {
    auto start = high_resolution_clock::now();
    for (ObjectCount i = 0; i < n; ++i) {

        int* p = new int((int)i);
        delete p;
    }
    auto end = high_resolution_clock::now();
    auto raw_time = duration_cast<milliseconds>(end - start).count();

    start = high_resolution_clock::now();
    for (ObjectCount i = 0; i < n; ++i) {

        UniquePtr<int> up_seq(MakeUnique<int>((int)i));
    } 
    end = high_resolution_clock::now();
    auto unique_time = duration_cast<milliseconds>(end - start).count();

  
    start = high_resolution_clock::now();
    for (ObjectCount i = 0; i < n; ++i) {

        SharedPtr<int> sp_seq = MakeShared<int>((int)i);
    } 
    end = high_resolution_clock::now();
    auto shared_time = duration_cast<milliseconds>(end - start).count();


    cout << "[Lifecycle Test] " << n << " objects: raw=" << raw_time << "ms, unique=" << unique_time << "ms, shared=" << shared_time << "ms\n";
    out << n << "," << raw_time << "," << unique_time << "," << shared_time << "\n";
}


int main() {

    test_unique_ptr();
    cout << "---------------------------\n";
    test_shared_ptr();
    cout << "---------------------------\n";


    ofstream out_lifecycle("results.csv"); 
    out_lifecycle << "size,raw,unique,shared\n";
    cout << "Running Benchmark: Object Lifecycle...\n";


    ObjectCount sizes_lifecycle[] = {
        10000,          
        100000,         
        1000000,        
        10000000,       
        100000000       
    }; 

    for (ObjectCount n : sizes_lifecycle) {
        benchmark_object_lifecycle(n, out_lifecycle);
    }
    out_lifecycle.close();
    cout << "---------------------------\n";
    cout << "All done. Results written to results.csv\n";

    cout << "Attempting to run visualization...\n";
    system("/opt/miniconda3/envs/visualizationlibraries/bin/python vizualisation.py");

    return 0;
}