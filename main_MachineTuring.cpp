#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <fstream> 
#include <sstream>

#include "DynamicArray.h"
#include "LazySequence.h"
#include "SmartPointer.h"
#include "TuringMachine.h"
#include "WriteOnlyStream.h"

const int GLOBAL_SHIFT = 3;


char EncryptChar(char c) {
    if (std::isalpha(static_cast<unsigned char>(c))) {
        char base = (std::islower(static_cast<unsigned char>(c)) ? 'a' : 'A');
        int original_pos = c - base;
        int new_pos = (original_pos + GLOBAL_SHIFT) % 26;
        return static_cast<char>(base + new_pos);
    }
    return c;
}

char DecryptChar(char c) {
    if (std::isalpha(static_cast<unsigned char>(c))) {
        char base = (std::islower(static_cast<unsigned char>(c)) ? 'a' : 'A');
        int original_pos = c - base;
        
        int new_pos = (original_pos - GLOBAL_SHIFT) % 26;
        if (new_pos < 0) new_pos += 26;
        return static_cast<char>(base + new_pos);
    }
    return c;
}


void DemoLazyCaesar() {
    std::cout << "\n=== DEMO 1: Lazy Caesar Cipher (Sequence Map) ===\n";
    std::string sourceText = "Merry christmas";
    std::cout << "Original Text: " << sourceText << "\n";

    
    
    int len = (int)sourceText.length();
    
    char* buffer = new char[len];
    for(int i=0; i<len; ++i) buffer[i] = sourceText[i];

    SharedPtr<LazySequence<char>> inputSeq = MakeShared<LazySequence<char>>(buffer, len);
    delete[] buffer; 

    
    
    auto encryptedSeqBase = inputSeq->Map(EncryptChar);

    std::cout << "Encrypted Sequence (Lazy): ";
    
    size_t count = encryptedSeqBase->GetLength().GetValue();
    for (size_t i = 0; i < count; ++i) {
        std::cout << encryptedSeqBase->Get(i);
    }
    std::cout << "\n";

    
    
    auto decryptedSeqBase = encryptedSeqBase->Map(DecryptChar);

    std::cout << "Decrypted Sequence (Lazy): ";
    for (size_t i = 0; i < count; ++i) {
        std::cout << decryptedSeqBase->Get(i);
    }
    std::cout << "\n";
}

void DemoLazyTuringMachine() {
    std::cout << "\n=== DEMO 2: Lazy Turing Machine Execution ===\n";
    
    
    const int q0_start = 0;
    const int q1_seek_1 = 1;
    const int q2_seek_0 = 2;
    const int q3_return_left = 3;
    const int q_accept = 10;
    const int q_reject = -10;

    LazyTuringMachine tm('_');
    tm.SetStartState(q0_start);
    tm.SetAcceptState(q_accept);
    tm.SetRejectState(q_reject);


    tm.AddTransition(Transition(q0_start, '0', q1_seek_1, 'X', 1));
    tm.AddTransition(Transition(q0_start, '1', q2_seek_0, 'Y', 1));
    tm.AddTransition(Transition(q0_start, 'X', q0_start, 'X', 1));
    tm.AddTransition(Transition(q0_start, 'Y', q0_start, 'Y', 1));
    tm.AddTransition(Transition(q0_start, '_', q_accept, '_', 0));

    tm.AddTransition(Transition(q1_seek_1, '0', q1_seek_1, '0', 1));
    tm.AddTransition(Transition(q1_seek_1, 'X', q1_seek_1, 'X', 1));
    tm.AddTransition(Transition(q1_seek_1, 'Y', q1_seek_1, 'Y', 1));
    tm.AddTransition(Transition(q1_seek_1, '1', q3_return_left, 'Y', -1));
    tm.AddTransition(Transition(q1_seek_1, '_', q_reject, '_', 0));

    tm.AddTransition(Transition(q2_seek_0, '1', q2_seek_0, '1', 1));
    tm.AddTransition(Transition(q2_seek_0, 'X', q2_seek_0, 'X', 1));
    tm.AddTransition(Transition(q2_seek_0, 'Y', q2_seek_0, 'Y', 1));
    tm.AddTransition(Transition(q2_seek_0, '0', q3_return_left, 'X', -1));
    tm.AddTransition(Transition(q2_seek_0, '_', q_reject, '_', 0));

    tm.AddTransition(Transition(q3_return_left, 'X', q3_return_left, 'X', -1));
    tm.AddTransition(Transition(q3_return_left, 'Y', q3_return_left, 'Y', -1));
    tm.AddTransition(Transition(q3_return_left, '0', q3_return_left, '0', -1));
    tm.AddTransition(Transition(q3_return_left, '1', q3_return_left, '1', -1));
    tm.AddTransition(Transition(q3_return_left, '_', q0_start, '_', 1)); 

    
    std::string input = "0011";
    std::cout << "Running TM on input: " << input << "\n";

    auto stateSerializer = [](const TMState& state) -> std::string {
        std::stringstream ss;
        ss << state; 
        return ss.str();
    };
    std::ofstream outFile("tm_tape_trace.txt");
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open file for writing.\n";
        return;
    }

    WriteOnlyStream<TMState> fileStream(&outFile, stateSerializer);
    SharedPtr<LazySequence<TMState>> trace = tm.GetExecutionTrace(input);
    int maxSteps = 100;
    int step = 0;
    
    std::cout << "Trace:\n";
    while (step < maxSteps) {
        
        TMState currentState = trace->Get(step);
        
        std::cout << currentState << "\n"; 

        try {
            fileStream.Write(currentState); 
        } catch (const std::exception& e) {
             std::cerr << "Ошибка записи в файл: " << e.what() << "\n";
        }


        if (currentState.state == q_accept) {
            std::cout << ">>> RESULT: ACCEPTED <<<\n";
            break;
        }
        if (currentState.state == q_reject) {
            std::cout << ">>> RESULT: REJECTED <<<\n";
            break;
        }
        
        step++;
    }
    
    std::cout << "Trace saved to tm_tape_trace.txt\n";
}


int main() {
    DemoLazyCaesar();
    DemoLazyTuringMachine();
    return 0;
}