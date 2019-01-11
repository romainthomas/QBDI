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
#ifndef PATCHUTILS_H
#define PATCHUTILS_H

#include <memory>
#include <utility>
#include <vector>
#include <iostream>

#include "Platform.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "Patch/Types.h"
#include "Utility/LogSys.h"

#if defined(QBDI_ARCH_X86_64) || defined(QBDI_ARCH_X86)
// skip RAX as it is very often used implicitly and LLVM
// sometimes don't tell us...
#define _QBDI_FIRST_FREE_REGISTER 1
#else
#define _QBDI_FIRST_FREE_REGISTER 0
#endif

namespace QBDI {

class RelocatableInst;
class PatchGenerator;

// Helper template

template<typename T, typename U> class AutoAlloc {
public:

    operator std::shared_ptr<T>() {
        return std::shared_ptr<T>(new U(*static_cast<U*>(this)));
    }
};

void inline append(std::vector<std::shared_ptr<RelocatableInst>> &u, const std::vector<std::shared_ptr<RelocatableInst>> v) {
    u.insert(u.end(), v.begin(), v.end());
}

void inline prepend(std::vector<std::shared_ptr<RelocatableInst>> &u, const std::vector<std::shared_ptr<RelocatableInst>> v) {
    u.insert(u.begin(), v.begin(), v.end());
}

void inline insert(std::vector<std::shared_ptr<RelocatableInst>> &u, size_t pos, const std::vector<std::shared_ptr<RelocatableInst>> v) {
    u.insert(u.begin() + pos, v.begin(), v.end());
}

void inline append(std::vector<std::shared_ptr<PatchGenerator>> &u, const std::vector<std::shared_ptr<PatchGenerator>> v) {
    u.insert(u.end(), v.begin(), v.end());
}

void inline prepend(std::vector<std::shared_ptr<PatchGenerator>> &u, const std::vector<std::shared_ptr<PatchGenerator>> v) {
    u.insert(u.begin(), v.begin(), v.end());
}

void inline insert(std::vector<std::shared_ptr<PatchGenerator>> &u, size_t pos, const std::vector<std::shared_ptr<PatchGenerator>> v) {
    u.insert(u.begin() + pos, v.begin(), v.end());
}

// Helper classes

class TempManager {
    // Assign temp_id to reg_id
    std::map<uint32_t, uint32_t> temps;
    const llvm::MCInst* inst;
    llvm::MCInstrInfo* MCII;
    llvm::MCRegisterInfo* MRI;

public:

    TempManager(const llvm::MCInst *inst, llvm::MCInstrInfo* MCII, llvm::MCRegisterInfo *MRI) : inst(inst), MCII(MCII), MRI(MRI) {};

    Reg getRegForTemp(unsigned int temp_id) {
        unsigned int reg_id;

        // Check if the 'temp' is already associated with a register
        auto&& it_temp = this->temps.find(temp_id);
        if (it_temp != std::end(this->temps)) {
          return it_temp->second;
        }

        // Start from the last free register found (or default)
        if(temps.size() > 0) {
          reg_id = temps.rbegin()->second + 1;
        }
        else {
          reg_id = _QBDI_FIRST_FREE_REGISTER;
        }

        const llvm::MCInstrDesc &desc = MCII->get(inst->getOpcode());
        // Find a free register
        for(; reg_id < AVAILABLE_GPR; reg_id++) {
            bool free = true;
            // Check for explicit registers
            for(unsigned int j = 0; inst && j < inst->getNumOperands(); j++) {
                const llvm::MCOperand &op = inst->getOperand(j);
                if (op.isReg() && MRI->isSubRegisterEq(GPR_ID[reg_id], op.getReg())) {
                    free = false;
                    break;
                }
            }
            // Check for implicitly used registers
            // For instance, flags registers
            if (free) {
                const uint16_t* implicitRegs = desc.getImplicitUses();
                for (; implicitRegs && *implicitRegs; ++implicitRegs) {
                    if (MRI->isSubRegisterEq(GPR_ID[reg_id], *implicitRegs)) {
                        free = false;
                        break;
                    }
                }
            }
            // Check for implicitly modified registers
            if (free) {
                const uint16_t* implicitRegs = desc.getImplicitDefs();
                for (; implicitRegs && *implicitRegs; ++implicitRegs) {
                    if (MRI->isSubRegisterEq(GPR_ID[reg_id], *implicitRegs)) {
                        free = false;
                        break;
                    }
                }
            }
            if(free) {
                // store it and return it
                temps[temp_id] = reg_id;
                return reg_id;
            }
        }
        LogError("TempManager::getRegForTemp", "No free registers found");
        abort();
    }

    Reg::Vec getUsedRegisters() {
        Reg::Vec list;
        for (auto&& p : temps) {
          list.emplace_back(p.second);
        }
        return list;
    }

    size_t getUsedRegisterNumber() {
        return temps.size();
    }

    unsigned getRegSize(unsigned reg) {
        for(unsigned i = 0; i < MRI->getNumRegClasses(); i++) {
            if(MRI->getRegClass(i).contains(reg)) {
                return MRI->getRegClass(i).getPhysRegSize();
            }
        }
        LogError("TempManager::getRegSize", "Register class for register %u not found", reg);
        return 0;
    }

    unsigned getSizedSubReg(unsigned reg, unsigned size) {
        if(getRegSize(reg) == size) {
            return reg;
        }
        for(unsigned i = 1; i < MRI->getNumSubRegIndices(); i++) {
            unsigned subreg = MRI->getSubReg(reg, i);
            if(subreg != 0 && getRegSize(subreg) == size) {
                return subreg;
            }
        }
        LogError("TempManager::getSizedSubReg", "No sub register of size %u found for register %u (%s)", size, reg, MRI->getName(reg), MRI->getRegClass(reg).getSize());
        abort();
    }
};

}

#endif
