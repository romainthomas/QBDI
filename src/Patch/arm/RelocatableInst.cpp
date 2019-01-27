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
#include "Patch/arm/RelocatableInst.h"
#include "Patch/arm/PatchRules.h"
#include "Patch/arm/Layer2.h"

namespace QBDI {

// MemoryConstant
// ==============
MemoryConstant::MemoryConstant(llvm::MCInst inst, unsigned int opn, rword value)
  : RelocatableInst(inst), opn(opn), value(value)
{}

llvm::MCInst MemoryConstant::reloc(ExecBlock *execBlock, CPUMode cpuMode) {
    uint16_t id = execBlock->newShadow();
    execBlock->setShadow(id, value);
    if (cpuMode == CPUMode::ARM) {
        inst.getOperand(opn).setImm(
            execBlock->getDataBlockOffset() + execBlock->getShadowOffset(id) - 8
        );
    }
    else if(cpuMode == CPUMode::Thumb) {
      bool usePC = false;
      for (llvm::MCOperand& op : this->inst) {
        if (op.isReg() and op.getReg() == Reg(REG_PC)) {
          op.setReg(execBlock->getScratchRegister());
          usePC = true;
        }
      }
      if (usePC) {
        inst.getOperand(opn).setImm(execBlock->getShadowOffset(id));
      } else {
          rword align = execBlock->getCurrentPC() % 4;
          inst.getOperand(opn).setImm(
            execBlock->getDataBlockOffset() + execBlock->getShadowOffset(id) + align - 4
          );
      }
    }
  return RelocatableInst::reloc(execBlock, cpuMode);
}

// DataBlockRel
// ============

DataBlockRel::DataBlockRel(llvm::MCInst inst, unsigned int opn, rword offset)
    : RelocatableInst(inst), opn(opn), offset(offset)
{}

llvm::MCInst DataBlockRel::reloc(ExecBlock *execBlock, CPUMode cpuMode) {
    if(cpuMode == CPUMode::ARM) {
        inst.getOperand(opn).setImm(offset + execBlock->getDataBlockOffset() - 8);
    }
    else if(cpuMode == CPUMode::Thumb) {
      bool usePC = false;
      for (llvm::MCOperand& op : this->inst) {
        if (op.isReg() and op.getReg() == Reg(REG_PC)) {
          op.setReg(execBlock->getScratchRegister());
          usePC = true;
        }
      }
      if (usePC) {
        inst.getOperand(opn).setImm(offset);
      } else {
        rword align = execBlock->getCurrentPC() % 4;
        inst.getOperand(opn).setImm(offset + execBlock->getDataBlockOffset() + align - 4);
      }
    }
  return RelocatableInst::reloc(execBlock, cpuMode);
}

// EpilogueRel
// ===========

EpilogueRel::EpilogueRel(llvm::MCInst inst, unsigned int opn, rword offset)
  : RelocatableInst(inst), opn(opn), offset(offset)
{}

llvm::MCInst EpilogueRel::reloc(ExecBlock *execBlock, CPUMode cpuMode) {
    if(cpuMode == CPUMode::ARM) {
        inst.getOperand(opn).setImm(offset + execBlock->getEpilogueOffset() - 8);
    }
    else if(cpuMode == CPUMode::Thumb) {
        //TODO Does EpilogueRel use PC Rel instructions ?
        //for (llvm::MCOperand& op : this->inst) {
        //  if (op.isReg() and op.getReg() == Reg(REG_PC)) {
        //    op.setReg(execBlock->getScratchRegister());
        //  }
        //}

        //intptr_t epilogueOffset = -llvm::sys::Process::getPageSize() + execBlock->getCodeBlockSize() - execBlock->epilogueSize ;
        // Thumb Align(PC,4)
        rword align = execBlock->getCurrentPC() % 4;
        inst.getOperand(opn).setImm(offset + execBlock->getEpilogueOffset() + align - 4);
    }
  return RelocatableInst::reloc(execBlock, cpuMode);
}

// HostPCRel
// =========
HostPCRel::HostPCRel(llvm::MCInst inst, unsigned int opn, rword offset)
  : RelocatableInst(inst), opn(opn), offset(offset)
{}

llvm::MCInst HostPCRel::reloc(ExecBlock *execBlock, CPUMode cpuMode) {
    uint16_t id = execBlock->newShadow();
    execBlock->setShadow(id, offset + execBlock->getCurrentPC());
    if(cpuMode == CPUMode::ARM) {
        inst.getOperand(opn).setImm(
            execBlock->getDataBlockOffset() + execBlock->getShadowOffset(id) - 8
        );
    }
    else if(cpuMode == CPUMode::Thumb) {
        bool usePC = false;
        for (llvm::MCOperand& op : this->inst) {
          if (op.isReg() and op.getReg() == Reg(REG_PC)) {
            usePC = true;
            op.setReg(execBlock->getScratchRegister());
          }
        }
        if (usePC) {
          inst.getOperand(opn).setImm(execBlock->getShadowOffset(id));
        } else {
          rword align = execBlock->getCurrentPC() % 4;
          inst.getOperand(opn).setImm(
            execBlock->getDataBlockOffset() + execBlock->getShadowOffset(id) + align - 4
          );
        }
     }
  return RelocatableInst::reloc(execBlock, cpuMode);
}

// InstId
// ======
InstId::InstId(llvm::MCInst inst, unsigned int opn)
  : RelocatableInst(inst), opn(opn)
{}

llvm::MCInst InstId::reloc(ExecBlock *execBlock, CPUMode cpuMode) {
    uint16_t id = execBlock->newShadow();
    execBlock->setShadow(id, execBlock->getNextInstID());
    if(cpuMode == CPUMode::ARM) {
        inst.getOperand(opn).setImm(
            execBlock->getDataBlockOffset() + execBlock->getShadowOffset(id) - 8
        );
    }
    else if(cpuMode == CPUMode::Thumb) {
        bool usePC = false;
        for (llvm::MCOperand& op : this->inst) {
          if (op.isReg() and op.getReg() == Reg(REG_PC)) {
            op.setReg(execBlock->getScratchRegister());
            usePC = true;
          }
        }
        if (usePC) {
          inst.getOperand(opn).setImm(execBlock->getShadowOffset(id));
        } else {
          rword align = execBlock->getCurrentPC() % 4;
          inst.getOperand(opn).setImm(
            execBlock->getDataBlockOffset() + execBlock->getShadowOffset(id) + align - 4
          );
        }
     }
  return RelocatableInst::reloc(execBlock, cpuMode);
}

// AdjustPCAlign
// =============
AdjustPCAlign::AdjustPCAlign(llvm::MCInst inst, unsigned int opn) :
  RelocatableInst(inst), opn(opn)
{}

llvm::MCInst AdjustPCAlign::reloc(ExecBlock *execBlock, CPUMode cpuMode) {
  if (cpuMode == CPUMode::Thumb) {
      // Thumb Align(PC,4)
      rword align = execBlock->getCurrentPC() % 4;
      inst.getOperand(opn).setImm(
          inst.getOperand(opn).getImm() + align
      );
   }
  return RelocatableInst::reloc(execBlock, cpuMode);
}


}
