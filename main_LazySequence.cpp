// main.cpp — fixed
#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <memory>
#include <functional>
#include <algorithm>

#include "LazySequence.h"
#include "ArraySequence.h"
#include "SmartPointer.h"

// Safety limits
static const size_t MAX_INDEX = 10000;
static const size_t MAX_PRINT = 50;

using std::cin;
using std::cout;
using std::endl;
using std::string;

// Example generator functions for int (expect Sequence<int>* and dynamic_cast to ArraySequence<int>*)
int FibRule(Sequence<int>* seq) {
    ArraySequence<int>* arr = dynamic_cast<ArraySequence<int>*>(seq);
    if (!arr) return 1;
    size_t n = arr->GetLength();
    if (n == 0) return 1;
    if (n == 1) return 1;
    int a = arr->Get(n - 1);
    int b = arr->Get(n - 2);
    return a + b;
}

int NatRule(Sequence<int>* seq) {
    ArraySequence<int>* arr = dynamic_cast<ArraySequence<int>*>(seq);
    if (!arr) return 0;
    size_t n = arr->GetLength();
    if (n == 0) return 0;
    return arr->Get(n - 1) + 1;
}

int main() {
    std::ios::sync_with_stdio(false);
    cin.tie(nullptr);

    std::vector< LazySequence<int> > seqs;
    seqs.reserve(16);

    auto read_int = [&]()->long long {
        long long x;
        while (!(cin >> x)) {
            cin.clear();
            cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            cout << "Please enter an integer: ";
        }
        return x;
    };

    cout << "=== LazySequence demo (Map / Reduce / Zip / Where / Concat) ===\n";

    while (true) {
        cout << "\nAvailable commands:\n";
        cout << "1  - Create empty LazySequence<int>\n";
        cout << "2  - Create LazySequence<int> from array\n";
        cout << "3  - Create lazy sequence with generator (Fibonacci / Natural)\n";
        cout << "4  - Show element (Get)\n";
        cout << "5  - Append / Prepend / InsertAt value\n";
        cout << "6  - Concat (seq A = seq A ++ seq B)\n";
        cout << "7  - Map (create mapping and show first N elements)\n";
        cout << "8  - Zip two sequences (show first N pairs)\n";
        cout << "9  - Where (filter, show first N elements)\n";
        cout << "10 - Reduce (sum for finite sequence)\n";
        cout << "11 - Show materialized elements of a sequence\n";
        cout << "12 - List sequences\n";
        cout << "0  - Exit\n";
        cout << "Choose command: ";

        int cmd;
        if (!(cin >> cmd)) {
            cout << "End of input. Exiting.\n";
            break;
        }

        try {
            if (cmd == 0) {
                cout << "Exit.\n";
                break;
            }

            if (cmd == 1) {
                seqs.emplace_back();
                cout << "Created empty LazySequence with id " << (seqs.size()-1) << "\n";
            }
            else if (cmd == 2) {
                cout << "How many elements in source array? ";
                int n = (int)read_int();
                if (n < 0) { cout << "Invalid count\n"; continue; }
                int* buf = nullptr;
                if (n > 0) {
                    buf = new int[n];
                    cout << "Enter " << n << " numbers:\n";
                    for (int i = 0; i < n; ++i) buf[i] = (int)read_int();
                }
                seqs.emplace_back(buf, n);
                if (buf) delete [] buf;
                cout << "Created LazySequence from array with id " << (seqs.size()-1) << "\n";
            }
            else if (cmd == 3) {
                cout << "Choose generator: 1 - Fibonacci, 2 - Natural (increment)\n> ";
                int g = (int)read_int();
                if (g != 1 && g != 2) { cout << "Unknown generator choice\n"; continue; }

                cout << "How many seed elements (0..1000)? ";
                int k = (int)read_int();
                if (k < 0 || k > 1000) { cout << "Invalid seed size\n"; continue; }

                int* seed = nullptr;
                if (k > 0) {
                    seed = new int[k];
                    cout << "Enter " << k << " seed elements:\n";
                    for (int i = 0; i < k; ++i) seed[i] = (int)read_int();
                }

                // Create exactly one LazySequence and then set generator on it.
                if (k > 0) {
                    seqs.emplace_back(seed, k); // constructor copies seed into core's materialised array
                } else {
                    seqs.emplace_back(); // empty sequence
                }

                // Now set generator on last created sequence
                if (g == 1) {
                    seqs.back().SetGenerator(FibRule);
                } else {
                    seqs.back().SetGenerator(NatRule);
                }

                if (seed) delete [] seed;
                cout << "Created lazy sequence with generator, id " << (seqs.size()-1) << "\n";
            }
            else if (cmd == 4) {
                cout << "Enter sequence id: ";
                int id = (int)read_int();
                if (id < 0 || (size_t)id >= seqs.size()) { cout << "Invalid id\n"; continue; }
                cout << "Enter element index (max " << MAX_INDEX << "): ";
                long long idx = read_int();
                if (idx < 0 || (size_t)idx > MAX_INDEX) { cout << "Index too large or negative\n"; continue; }
                try {
                    int v = seqs[id].Get((size_t)idx);
                    cout << "seq[" << id << "][" << idx << "] = " << v << "\n";
                } catch (const std::exception& e) {
                    cout << "Get failed: " << e.what() << "\n";
                }
            }
            else if (cmd == 5) {
                cout << "Enter sequence id: ";
                int id = (int)read_int();
                if (id < 0 || (size_t)id >= seqs.size()) { cout << "Invalid id\n"; continue; }
                cout << "Choose: 1-Append 2-Prepend 3-InsertAt\n> ";
                int op = (int)read_int();
                cout << "Enter value: ";
                int val = (int)read_int();
                if (op == 1) {
                    seqs[id].AppendValue(val);
                    cout << "Append queued/added\n";
                } else if (op == 2) {
                    seqs[id].PrependValue(val);
                    cout << "Prepend queued/added\n";
                } else if (op == 3) {
                    cout << "Enter position for InsertAt: ";
                    long long pos = read_int();
                    if (pos < 0) { cout << "Invalid position\n"; continue; }
                    seqs[id].InsertAtValue(val, (size_t)pos);
                    cout << "InsertAt performed\n";
                } else {
                    cout << "Unknown operation\n";
                }
            }
            else if (cmd == 6) {
                cout << "Concat: enter id of first sequence (A): ";
                int a = (int)read_int();
                cout << "Enter id of second sequence (B): ";
                int b = (int)read_int();

                if (a < 0 || b < 0 ||
                    (size_t)a >= seqs.size() ||
                    (size_t)b >= seqs.size()) {
                    cout << "Invalid ids\n";
                    continue;
                }

                // Берём корень второй последовательности
                SharedPtr< LazySequenceBase<int> > otherRoot = seqs[b].GetRoot();

                // Конкатим: seqs[a] = seqs[a] ++ seqs[b]
                seqs[a].ConcatWith(otherRoot);

                cout << "Concat done: seq[" << a << "] now contains seq[" << b
                     << "] as continuation (lazy)\n";
            }

            else if (cmd == 7) {
                cout << "Map: enter sequence id: ";
                int id = (int)read_int();
                if (id < 0 || (size_t)id >= seqs.size()) { cout << "Invalid id\n"; continue; }
                cout << "Mapping example: x -> x*2\n";
                auto root = seqs[id].GetRoot();
                // call template Map on dependent type via 'template'
                auto mapped = root->template Map<double>([](int x)->double { return x * 2.0; });
                cout << "How many elements to show (max " << MAX_PRINT << ")? ";
                int n = (int)read_int();
                n = std::min<int>(n, (int)MAX_PRINT);
                for (int i = 0; i < n; ++i) {
                    try {
                        double v = mapped->Get((size_t)i);
                        cout << "[" << i << "] -> " << v << "\n";
                    } catch (const std::exception& e) {
                        cout << "Stopped at i=" << i << " : " << e.what() << "\n";
                        break;
                    }
                }
            }
            else if (cmd == 8) {
                cout << "Zip: enter id of first sequence: ";
                int a = (int)read_int();
                cout << "Enter id of second sequence: ";
                int b = (int)read_int();
                if (a < 0 || b < 0 || (size_t)a >= seqs.size() || (size_t)b >= seqs.size()) { cout << "Invalid ids\n"; continue; }
                auto ra = seqs[a].GetRoot();
                auto rb = seqs[b].GetRoot();
                auto zr = ra->template Zip<int>(rb);
                cout << "How many pairs to show (max " << MAX_PRINT << ")? ";
                int n = (int)read_int();
                n = std::min<int>(n, (int)MAX_PRINT);
                for (int i = 0; i < n; ++i) {
                    try {
                        auto p = zr->Get((size_t)i);
                        cout << "[" << i << "] = (" << p.first << ", " << p.second << ")\n";
                    } catch (const std::exception& e) {
                        cout << "Stopped at i=" << i << " : " << e.what() << "\n";
                        break;
                    }
                }
            }
            else if (cmd == 9) {
                cout << "Where: enter sequence id: ";
                int id = (int)read_int();
                if (id < 0 || (size_t)id >= seqs.size()) { cout << "Invalid id\n"; continue; }
                cout << "Predicate: keep even numbers (x%2==0)\n";
                auto root = seqs[id].GetRoot();
                auto filtered = root->Where([](int x)->bool { return (x % 2) == 0; });
                cout << "How many to show (max " << MAX_PRINT << ")? ";
                int n = (int)read_int();
                n = std::min<int>(n, (int)MAX_PRINT);
                for (int i = 0; i < n; ++i) {
                    try {
                        int v = filtered->Get((size_t)i);
                        cout << "[" << i << "] -> " << v << "\n";
                    } catch (const std::exception& e) {
                        cout << "Stopped at i=" << i << " : " << e.what() << "\n";
                        break;
                    }
                }
            }
            else if (cmd == 10) {
                cout << "Reduce (sum): enter sequence id: ";
                int id = (int)read_int();
                if (id < 0 || (size_t)id >= seqs.size()) { cout << "Invalid id\n"; continue; }
                auto root = seqs[id].GetRoot();
                try {
                    int sum = Reduce<int,int>(root, [](int acc, int v)->int { return acc + v; }, 0);
                    cout << "Sum of elements = " << sum << "\n";
                } catch (const std::exception& e) {
                    cout << "Reduce not performed: " << e.what() << "\n";
                }
            }
            else if (cmd == 11) {
                cout << "Show materialized elements: enter id: ";
                int id = (int)read_int();
                if (id < 0 || (size_t)id >= seqs.size()) { cout << "Invalid id\n"; continue; }
                size_t m = seqs[id].GetMaterializedCount();
                cout << "Materialized elements: " << m << "\n";
                size_t show = (m > MAX_PRINT ? MAX_PRINT : m);
                for (size_t i = 0; i < show; ++i) {
                    try {
                        cout << "[" << i << "] = " << seqs[id].Get(i) << "\n";
                    } catch (...) { break; }
                }
                if (m > show) cout << "(... and " << (m-show) << " hidden)\n";
            }
            else if (cmd == 12) {
                cout << "List of sequences:\n";
                for (size_t i = 0; i < seqs.size(); ++i) {
                    Cardinal ln = seqs[i].GetLength();
                    cout << "id=" << i << " materialized=" << seqs[i].GetMaterializedCount()
                         << " length=" << (ln.IsOmega() ? string("Omega") : std::to_string(ln.GetValue())) << "\n";
                }
            }
            else {
                cout << "Unknown command\n";
            }
        } catch (const std::exception& e) {
            cout << "Exception: " << e.what() << "\n";
        } catch (...) {
            cout << "Unknown error\n";
        }
    }

    return 0;
}
