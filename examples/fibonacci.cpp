#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstdint>
#include <cstring>

#include "QBDI.h"

#define FAKE_RET_ADDR 42


int fibonacci(int n) {
  return 43;
    //if(n <= 2)
    //    return 1;
    //return fibonacci(n-1) + fibonacci(n-2);
}


QBDI::VMAction countInstruction(QBDI::VMInstanceRef vm, QBDI::GPRState *gprState,
                                QBDI::FPRState *fprState, void *data) {
    //uint32_t* counter = (uint32_t*) data;
    const QBDI::InstAnalysis* instAnalysis = vm->getInstAnalysis();
    std::cout << "R0: " << std::dec << gprState->r0 << std::endl;
    std::cout << "R1: " << std::dec << gprState->r1 << std::endl;
    std::cout << "R2: " << std::dec << gprState->r2 << std::endl;
    std::cout << "R3: " << std::dec << gprState->r3 << std::endl;
    std::cout << "R4: " << std::dec << gprState->r4 << std::endl;
    std::cout << "R5: " << std::dec << gprState->r5 << std::endl;
    std::cout << "R6: " << std::dec << gprState->r6 << std::endl;
    std::cout << "R7: " << std::dec << gprState->r7 << std::endl;
    std::cout << "R8: " << std::dec << gprState->r8 << std::endl;
    std::cout << "R9: " << std::dec << gprState->r9 << std::endl;
    std::cout << "R10 " << std::dec << gprState->r10 << std::endl;
    std::cout << "R12 " << std::dec << gprState->r12 << std::endl;
    std::cout << "FP " << std::hex << gprState->fp << std::endl;
    std::cout << "LR " << std::hex << gprState->lr << std::endl;
    std::cout << "PC " << std::hex << std::showbase << gprState->pc << std::endl;
    std::cout << "===========" << std::endl;
    std::cout << std::hex << instAnalysis->address << ": " << instAnalysis->disassembly << std::endl;
    std::cout << "===========" << std::endl;
    //(*counter)++;
    return QBDI::VMAction::CONTINUE;
}

QBDI::VMAction countRecursion(QBDI::VMInstanceRef vm, QBDI::GPRState *gprState,
                              QBDI::FPRState *fprState, void *data) {
    *((unsigned*) data) += 1;
    return QBDI::VMAction::CONTINUE;
}

static const size_t STACK_SIZE = 0x100000; // 1MB

int main(int argc, char** argv) {

    QBDI::addLogFilter("*", QBDI::LogPriority::DEBUG);
    //FILE* logFile = fopen(".qbdi_log", "w");
    //QBDI::setLogOutput(logFile);
    int n = 0;
    uint32_t counter = 0;
    unsigned recursions = 0;
    uint8_t *fakestack = nullptr;
    QBDI::GPRState *state;

    std::cout << "Initializing VM ..." << std::endl;
    // Constructing a new QBDI vm
    QBDI::VM *vm = new QBDI::VM();
    // Registering countInstruction() callback to be called after every instruction

    // POSTINST: OK
    // PREINST:  KO
    vm->addCodeCB(QBDI::PREINST, countInstruction, &counter);
    //vm->addCodeCB(QBDI::POSTINST, countInstruction, &counter);

    // Registering countRecursion() callback to be called before the first instruction of fibonacci
    //vm->addCodeAddrCB((QBDI::rword) &fibonacci, QBDI::PREINST, countRecursion, &recursions);

    // Get a pointer to the GPR state of the vm
    state = vm->getGPRState();
    // Setup initial GPR state, this fakestack will produce a ret NULL at the end of the execution
    QBDI::allocateVirtualStack(state, STACK_SIZE, &fakestack);
    // Argument to the fibonnaci call
    if(argc >= 2) {
        n = atoi(argv[1]);
    }
    if(n < 1) {
        n = 1;
    }
    QBDI::simulateCall(state, FAKE_RET_ADDR, {(QBDI::rword) n});

    std::cout << "Running fibonacci(" << n << ") ..." << std::endl;
    // Instrument everything
    vm->instrumentAllExecutableMaps();
    // Run the DBI execution
    vm->run((QBDI::rword) fibonacci, (QBDI::rword) FAKE_RET_ADDR);
    std::cout << "Returned value: " << QBDI_GPR_GET(state, QBDI::REG_RETURN) << std::endl;
    delete vm;
    QBDI::alignedFree(fakestack);

    return 0;
}
