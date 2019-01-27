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

#include "Patch/InstTransform.h"

namespace QBDI {

InstTransform::~InstTransform() = default;

// SetOperand
// ==========
SetOperand::SetOperand(Operand opn, Temp temp) :
  opn(opn), type(TempOperandType), temp(temp), reg(0), imm(0)
{}

SetOperand::SetOperand(Operand opn, Reg reg) :
  opn(opn), type(RegOperandType), temp(0), reg(reg), imm(0)
{}

SetOperand::SetOperand(Operand opn, Constant imm) :
  opn(opn), type(ImmOperandType), temp(0), reg(0), imm(imm)
{}

void SetOperand::transform(llvm::MCInst &inst, rword address, size_t instSize, TempManager *temp_manager) {

 switch(type) {
  case TempOperandType:
    {
      inst.getOperand(opn).setReg(temp_manager->getRegForTemp(temp));
      break;
    }

  case RegOperandType:
    {
      inst.getOperand(opn).setReg(reg);
      break;
    }

  case ImmOperandType:
    {
      inst.getOperand(opn).setImm(imm);
      break;
    }
 }
}
// SubstituteWithTemp
// ==================

SubstituteWithTemp::SubstituteWithTemp(Reg reg, Temp temp) :
  reg(reg), temp(temp)
{}

void SubstituteWithTemp::transform(llvm::MCInst &inst, rword address, size_t instSize, TempManager *temp_manager) {
    for (size_t i = 0; i < inst.getNumOperands(); ++i) {
      llvm::MCOperand &op = inst.getOperand(i);
      if(op.isReg() && op.getReg() == reg) {
          op.setReg(temp_manager->getRegForTemp(temp));
      }
    }
}

// AddOperand
// ==========
AddOperand::AddOperand(Operand opn, Temp temp) :
  opn(opn), type(TempOperandType), temp(temp), reg(0), imm(0)
{}

AddOperand::AddOperand(Operand opn, Reg reg) :
  opn(opn), type(RegOperandType), temp(0), reg(reg), imm(0)
{}

AddOperand::AddOperand(Operand opn, Constant imm) :
  opn(opn), type(ImmOperandType), temp(0), reg(0), imm(imm)
{}

void AddOperand::transform(llvm::MCInst &inst, rword address, size_t instSize, TempManager *temp_manager) {
    switch(type) {
        case TempOperandType:
            inst.insert(inst.begin() + opn, llvm::MCOperand::createReg(temp_manager->getRegForTemp(temp)));
            break;
        case RegOperandType:
            inst.insert(inst.begin() + opn, llvm::MCOperand::createReg(reg));
            break;
        case ImmOperandType:
            inst.insert(inst.begin() + opn, llvm::MCOperand::createImm(imm));
            break;
    }
}

// RemoveOperand
// =============

RemoveOperand::RemoveOperand(Reg reg) :
  reg(reg) {}

void RemoveOperand::transform(llvm::MCInst &inst, rword address, size_t instSize, TempManager *temp_manager) {
    for(auto it = inst.begin(); it != inst.end(); ++it) {
        if(it->isReg() && it->getReg() == reg) {
            inst.erase(it);
            break;
        }
    }
}

// SetOpcode
// =========

SetOpcode::SetOpcode(unsigned int opcode) :
  opcode(opcode)
{}

void SetOpcode::transform(llvm::MCInst &inst, rword address, size_t instSize, TempManager *temp_manager) {
    inst.setOpcode(opcode);
}

}
