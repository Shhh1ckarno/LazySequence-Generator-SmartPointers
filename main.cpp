#include <iostream>
#include <string>
#include <stdexcept>
#include <clocale>
#include "MutableArraySequence.h"
#include "ImmutableArraySequence.h"
#include "LinkedSequenceMutable.h"
#include "LinkedSequenceImmutable.h"
#include "Stack.h"
#include "Queue.h"
#include "Deck.h"
#include <cassert>
void RunDequeTests() {
    Deque<double> dq;

    assert(dq.GetSize() == 0);


    dq.PushTop(10.0);
    dq.PushTop(1.2324);
    dq.PushTop(4.0);
    assert(dq.GetSize() == 3);
    assert(dq.Top() == 4.0);
    assert(dq.Front() == 10.0);

    dq.PushFront(1.2);
    assert(dq.GetSize() == 4);
    assert(dq.Front() == 1.2);

    double front = dq.PopFront();
    assert(front == 1.2);
    assert(dq.Front() == 10.0);
    assert(dq.GetSize() == 3);

    double back = dq.PopTop();
    assert(back == 4.0);
    assert(dq.Top() == 1.2324);
    assert(dq.GetSize() == 2);

    dq.Clear();
    assert(dq.GetSize() == 0);

    try {
        dq.PopFront();
        assert(false);
    } catch (const std::out_of_range&) {
    }

    try {
        dq.Front();
        assert(false);
    } catch (const std::out_of_range&) {
    }

    std::cout << "Deque tests PASS\n";
}
void RunQueueTests() {
    Queue<char> q;
    assert(q.GetSize() == 0);
    q.Push('a');
    q.Push('b');
    q.Push('c');
    assert(q.GetSize() == 3);
    assert(q.Front() == 'a');
    char front = q.Pop();
    assert(front == 'a');
    assert(q.Front() == 'b');
    front = q.Pop();
    assert(front == 'b');

    q.Clear();
    assert(q.GetSize() == 0);

    try {
        q.Pop();
        assert(false);
    } catch (const std::out_of_range&) {
    }

    std::cout << "Queue tests PASS\n";
}
void RunStackTests() {
    Stack<int> dq;
    assert(dq.GetSize() == 0);
    dq.Push(1);
    dq.Push(2);
    dq.Push(3);
    assert(dq.Top()==3);
    int a=dq.Pop();
    assert(a==3);
    assert(dq.Pop()==2);
    dq.Clear();
    try {
        dq.Pop();
        assert(false);
    } catch (const std::out_of_range&) {
    }

    std::cout << "Stack tests PASS\n";
}
int main() {
    RunDequeTests();
    RunQueueTests();
    RunStackTests();
    return 0;
}
