cmake_minimum_required(VERSION 3.28)
project(Laba2)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Laba2 main.cpp
        DynamicArray.h
        LinkedList.h
        Sequence.h
        ArraySequence.h
        MutableArraySequence.h
        ImmutableArraySequence.h
        LinkedSequence.h
        LinkedSequenceMutable.h
        LinkedSequenceImmutable.h
        Stack.h
        Queue.h
        Deck.h
        SmartPointer.h
)
