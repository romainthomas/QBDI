/*
 * This file is part of QBDI.
 *
 * Copyright 2017 Quarkslab
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Patch/PatchCondition.h"

namespace QBDI {
// PatchCondition
// ==============

RangeSet<rword> PatchCondition::affectedRange() {
  RangeSet<rword> r;
  r.add(Range<rword>(0, (rword) -1));
  return r;
}

PatchCondition::~PatchCondition() = default;

// MnemonicIs
// ==========

MnemonicIs::MnemonicIs(const char *mnemonic) :
  mnemonic(mnemonic)
{}

bool MnemonicIs::test(const llvm::MCInst* inst, rword address, rword instSize, llvm::MCInstrInfo* MCII) {
    return QBDI::String::startsWith(mnemonic.c_str(), MCII->getName(inst->getOpcode()).data());
}


// OpIs
// ====
OpIs::OpIs(unsigned int op) :
  op(op)
{}

bool OpIs::test(const llvm::MCInst* inst, rword address, rword instSize, llvm::MCInstrInfo* MCII) { // refactor all test() add MCII
    return inst->getOpcode() == op;
}

// RegIs
// =====
RegIs::RegIs(Operand opn, Reg reg) :
  opn(opn), reg(reg)
{}


bool RegIs::test(const llvm::MCInst* inst, rword address, rword instSize, llvm::MCInstrInfo* MCII) {
    return inst->getOperand(opn).isReg() && inst->getOperand(opn).getReg() == (unsigned int) reg;
}

// UseReg
// ======
UseReg::UseReg(Reg reg) :
  reg(reg)
{}

bool UseReg::test(const llvm::MCInst* inst, rword address, rword instSize, llvm::MCInstrInfo* MCII) {
    for(unsigned int i  = 0; i < inst->getNumOperands(); i++) {
        const llvm::MCOperand &op = inst->getOperand(i);
        if(op.isReg() && op.getReg() == (unsigned int) reg) {
            return true;
        }
    }
    return false;
}

// InstructionInRange
// ==================
InstructionInRange::InstructionInRange(Constant start, Constant end) :
  range(start, end)
{}

bool InstructionInRange::test(const llvm::MCInst* inst, rword address, rword instSize, llvm::MCInstrInfo* MCII) {
    if(range.contains(Range<rword>(address, address + instSize))) {
        return true;
    }
    else {
        return false;
    }
}

RangeSet<rword> InstructionInRange::affectedRange() {
    RangeSet<rword> r;
    r.add(range);
    return r;
}

// AddressIs
// =========

AddressIs::AddressIs(rword breakpoint) :
  breakpoint(breakpoint)
{}

bool AddressIs::test(const llvm::MCInst* inst, rword address, rword instSize, llvm::MCInstrInfo* MCII) {
    return address == breakpoint;
}

RangeSet<rword> AddressIs::affectedRange() {
    RangeSet<rword> r;
    r.add(Range<rword>(breakpoint, breakpoint + 1));
    return r;
}

// OperandIsReg
// ============

OperandIsReg::OperandIsReg(Operand opn) :
  opn(opn)
{}

bool OperandIsReg::test(const llvm::MCInst* inst, rword address, rword instSize, llvm::MCInstrInfo* MCII) {
    return inst->getOperand(opn).isReg();
}


// OperandIsImm
// ============

OperandIsImm::OperandIsImm(Operand opn) :
  opn(opn)
{}

bool OperandIsImm::test(const llvm::MCInst* inst, rword address, rword instSize, llvm::MCInstrInfo* MCII) {
    return inst->getOperand(opn).isImm();
}

// And
// ===
And::And(PatchCondition::SharedPtrVec conditions) :
  conditions(conditions)
{};

bool And::test(const llvm::MCInst* inst, rword address, rword instSize, llvm::MCInstrInfo* MCII) {
    for(unsigned int i = 0; i < conditions.size(); i++) {
        if(conditions[i]->test(inst, address, instSize, MCII) == false) {
            return false;
        }
    }
    return true;
}

RangeSet<rword> And::affectedRange() {
    RangeSet<rword> r;
    r.add(Range<rword>(0, (rword)-1));
    for(unsigned int i = 0; i < conditions.size(); i++) {
        r.intersect(conditions[i]->affectedRange());
    }
    return r;
}

// Or
// ==

Or::Or(PatchCondition::SharedPtrVec conditions) :
  conditions(conditions)
{}

bool Or::test(const llvm::MCInst* inst, rword address, rword instSize, llvm::MCInstrInfo* MCII) {
    for(unsigned int i = 0; i < conditions.size(); i++) {
        if(conditions[i]->test(inst, address, instSize, MCII) == true) {
            return true;
        }
    }
    return false;
}

RangeSet<rword> Or::affectedRange() {
    RangeSet<rword> r;
    for(unsigned int i = 0; i < conditions.size(); i++) {
        r.add(conditions[i]->affectedRange());
    }
    return r;
}

// Not
// ===
Not::Not(PatchCondition::SharedPtr condition) :
  condition(condition)
{}

bool Not::test(const llvm::MCInst* inst, rword address, rword instSize, llvm::MCInstrInfo* MCII) {
  return not condition->test(inst, address, instSize, MCII);
}

// True
// ====
True::True() = default;

bool True::test(const llvm::MCInst* inst, rword address, rword instSize, llvm::MCInstrInfo* MCII) {
    return true;
}

// DoesReadAccess
// ==============
DoesReadAccess::DoesReadAccess() = default;

bool DoesReadAccess::test(const llvm::MCInst* inst, rword address, rword instSize, llvm::MCInstrInfo* MCII) {
    return getReadSize(inst) > 0;
}

// DoesWriteAccess
// ===============
DoesWriteAccess::DoesWriteAccess() = default;

bool DoesWriteAccess::test(const llvm::MCInst* inst, rword address, rword instSize, llvm::MCInstrInfo* MCII) {
    return getWriteSize(inst) > 0;
}

// ReadAccessSizeIs
// ================
ReadAccessSizeIs::ReadAccessSizeIs(Constant size) :
  size(size)
{}

bool ReadAccessSizeIs::test(const llvm::MCInst* inst, rword address, rword instSize, llvm::MCInstrInfo* MCII) {
    return getReadSize(inst) == (rword) size;
}

// WriteAccessSizeIs
// =================
WriteAccessSizeIs::WriteAccessSizeIs(Constant size) :
  size(size)
{}

bool WriteAccessSizeIs::test(const llvm::MCInst* inst, rword address, rword instSize, llvm::MCInstrInfo* MCII) {
    return getWriteSize(inst) == (rword) size;
}

// IsStackRead
// ===========
IsStackRead::IsStackRead() = default;

bool IsStackRead::test(const llvm::MCInst* inst, rword address, rword instSize, llvm::MCInstrInfo* MCII) {
  return isStackRead(inst);
}

// IsStackWrite
// ============
IsStackWrite::IsStackWrite() = default;

bool IsStackWrite::test(const llvm::MCInst* inst, rword address, rword instSize, llvm::MCInstrInfo* MCII) {
    return isStackWrite(inst);
}

}
