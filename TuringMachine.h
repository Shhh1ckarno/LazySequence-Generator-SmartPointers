#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <functional>
#include <algorithm>

#include "DynamicArray.h"   
#include "LazySequence.h"   
#include "SmartPointer.h"



struct Transition {
    int fromState;
    char readSym;
    int toState;
    char writeSym;
    int move; 

    Transition() : fromState(0), readSym(0), toState(0), writeSym(0), move(0) {}
    Transition(int f, char r, int t, char w, int m) 
        : fromState(f), readSym(r), toState(t), writeSym(w), move(m) {}
};



class TuringTape {
private:
    DynamicArray<char> leftTape; 
    DynamicArray<char> rightTape; 
    int headPos;
    char blank;

public:
    TuringTape(char blankSym = '_') : leftTape(), rightTape(), headPos(0), blank(blankSym) {}

    void LoadFromString(const std::string& input) {
        rightTape = DynamicArray<char>(input.length());
        for (size_t i = 0; i < input.length(); ++i) {
            rightTape.Set(i, input[i]);
        }
        headPos = 0;
    }

    char Read() const {
        if (headPos >= 0) {
            if (headPos < rightTape.GetSize()) return rightTape.Get(headPos);
        } else {
            int idx = -headPos - 1;
            if (idx < leftTape.GetSize()) return leftTape.Get(idx);
        }
        return blank;
    }

    void Write(char c) {
        if (headPos >= 0) {
            if (headPos >= rightTape.GetSize()) {
                rightTape.Resize(headPos + 1);
            }
            rightTape.Set(headPos, c);
        } else {
            int idx = -headPos - 1;
            if (idx >= leftTape.GetSize()) {
                leftTape.Resize(idx + 1);
            }
            leftTape.Set(idx, c);
        }
    }

    void MoveLeft() { headPos--; }
    void MoveRight() { headPos++; }
    
    std::string Snapshot(int radiusLeft, int radiusRight) const {
        std::stringstream ss;
        for (int i = headPos - radiusLeft; i <= headPos + radiusRight; ++i) {
            char c = blank;
            if (i >= 0) {
                if (i < rightTape.GetSize()) c = rightTape.Get(i);
            } else {
                int idx = -i - 1;
                if (idx < leftTape.GetSize()) c = leftTape.Get(idx);
            }
            
            if (i == headPos) ss << "[" << c << "]";
            else ss << c;
        }
        return ss.str();
    }

    std::string ToString() const {
        std::string res = "";
        for(int i = leftTape.GetSize() - 1; i >= 0; --i) {
            char c = leftTape.Get(i);
            if(c != 0) res += c; else res += blank;
        }
        for(int i = 0; i < rightTape.GetSize(); ++i) {
            char c = rightTape.Get(i);
            if(c != 0) res += c; else res += blank;
        }
        return res;
    }

    bool operator==(const TuringTape& other) const {
        return headPos == other.headPos && 
               leftTape == other.leftTape && 
               rightTape == other.rightTape &&
               blank == other.blank;
    }
};


struct TMState {
    TuringTape tape;
    int state;
    int stepCount;

    TMState() : tape('_'), state(0), stepCount(0) {}
    TMState(const TuringTape& t, int s, int count) : tape(t), state(s), stepCount(count) {}

    bool operator==(const TMState& other) const {
        return state == other.state && 
               stepCount == other.stepCount &&
               tape == other.tape;
    }
    bool operator!=(const TMState& other) const { return !(*this == other); }

    friend std::ostream& operator<<(std::ostream& os, const TMState& s) {
        os << "Step: " << s.stepCount << " | State: " << s.state;
        os << " | Tape: " << s.tape.Snapshot(5, 5);
        return os;
    }
};


class LazyTuringMachine {
public:
    LazyTuringMachine(char blank = '_') 
        : blankSym(blank), startState(0), acceptState(-1), rejectState(-2) {}

    void SetStartState(int s) { startState = s; }
    void SetAcceptState(int s) { acceptState = s; }
    void SetRejectState(int s) { rejectState = s; }
    
    void AddTransition(const Transition& tr) {
        transitions.Append(tr);
    }

    SharedPtr<LazySequence<TMState>> GetExecutionTrace(const std::string& inputTape) {
        
        TuringTape initialTape(blankSym);
        initialTape.LoadFromString(inputTape);
        
        TMState seedState(initialTape, startState, 0);
        
        SharedPtr<MutableArraySequence<TMState>> seedSeq = MakeShared<MutableArraySequence<TMState>>();
        seedSeq->Append(seedState);

        DynamicArray<Transition> rulesCapture = this->transitions; 
        int acc = this->acceptState;
        int rej = this->rejectState;

        auto generatorRule = [rulesCapture, acc, rej](Sequence<TMState>* history) mutable -> TMState {
            
            Cardinal len = history->GetLength();
            size_t lastIdx = len.GetValue() - 1; 
            TMState prev = history->Get(lastIdx);

            if (prev.state == acc || prev.state == rej) {
                return prev; 
            }

            char curChar = prev.tape.Read();
            bool found = false;
            
            TMState next = prev; 
            next.stepCount++;

            for (int i = 0; i < rulesCapture.GetSize(); ++i) {
                Transition tr = rulesCapture.Get(i);
                if (tr.fromState == prev.state && tr.readSym == curChar) {
                    next.tape.Write(tr.writeSym);
                    if (tr.move < 0) next.tape.MoveLeft();
                    else if (tr.move > 0) next.tape.MoveRight();
                    
                    next.state = tr.toState;
                    found = true;
                    break;
                }
            }

            if (!found) {
                next.state = rej; 
            }

            return next;
        };

        return MakeShared<LazySequence<TMState>>(generatorRule, seedSeq.get());
    }

private:
    DynamicArray<Transition> transitions; 
    int startState;
    int acceptState;
    int rejectState;
    char blankSym;
};