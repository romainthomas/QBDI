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

#include "Patch/InstrRule.h"

#if defined(QBDI_ARCH_X86_64) || defined(QBDI_ARCH_X86)
  #include "Patch/x86-64/InstrRules.h"
  #include "Patch/x86-64/PatchGenerator.h"
#elif defined(QBDI_ARCH_ARM)
  #include "Patch/arm/InstrRules.h"
  #include "Patch/arm/PatchGenerator.h"
#endif

namespace QBDI {
InstrRule::InstrRule(PatchCondition::SharedPtr condition, PatchGenerator::SharedPtrVec patchGen, InstPosition position, bool breakToHost) :
  condition(condition), patchGen(patchGen), position(position), breakToHost(breakToHost)
{}

InstPosition InstrRule::getPosition() {
  return position;
}

RangeSet<rword> InstrRule::affectedRange() const {
  return condition->affectedRange();
}

bool InstrRule::canBeApplied(const Patch &patch, LLVMCPU* llvmCPU) {
  return condition->test(&patch.metadata.inst, patch.metadata.address, patch.metadata.instSize, llvmCPU->getMII());
}


void InstrRule::instrument(Patch &patch, LLVMCPU* llvmCPU) {
    /* The instrument function needs to handle several different cases. An instrumentation can
     * be either prepended or appended to the patch and, in each case, can trigger a break to
     * host.
    */
    RelocatableInst::SharedPtrVec instru;
    TempManager tempManager(&patch.metadata.inst, llvmCPU->getMII(), llvmCPU->getMRI());

    // Generate the instrumentation code from the original instruction context
    for(PatchGenerator::SharedPtr& g : patchGen) {
        append(instru,
            g->generate(&patch.metadata.inst, patch.metadata.address, patch.metadata.instSize, patch.metadata.cpuMode, &tempManager, nullptr)
        );
    }

    // In case we break to the host, we need to ensure the value of PC in the context is
    // correct. This value needs to be set when instrumenting before the instruction or when
    // instrumenting after an instruction which does not set PC.
    if (breakToHost and (position == InstPosition::PREINST or patch.metadata.modifyPC == false)) {
      switch(position) {
          // In PREINST PC is set to the current address
          case InstPosition::PREINST:
            {
              // Tmp(0) := Instruction Address
              append(instru,
                     GetConstant(
                          Temp(0),
                          Constant(patch.metadata.address)
                     ).generate(
                          &patch.metadata.inst,
                          patch.metadata.address,
                          patch.metadata.instSize,
                          patch.metadata.cpuMode,
                          &tempManager,
                          nullptr
                     )
              );
              break;
            }
          // In POSTINST PC is set to the next instruction address
          case InstPosition::POSTINST:
            {
              append(instru,
                     GetConstant(
                          Temp(0),
                          Constant(patch.metadata.address + patch.metadata.instSize)
                     ).generate(
                          &patch.metadata.inst,
                          patch.metadata.address,
                          patch.metadata.instSize,
                          patch.metadata.cpuMode,
                          &tempManager,
                          nullptr
                     )
              );
              break;
            }
      }
      append(instru, SaveReg(tempManager.getRegForTemp(0), Offset(Reg(REG_PC))).generate(patch.metadata.cpuMode));
    }

    // The breakToHost code requires one temporary register. If none were allocated by the
    // instrumentation we thus need to add one.
    if(breakToHost && tempManager.getUsedRegisterNumber() == 0) {
      tempManager.getRegForTemp(Temp(0));
    }

    // Prepend the temporary register saving code to the instrumentation
    Reg::Vec usedRegisters = tempManager.getUsedRegisters();
    for (uint32_t i = 0; i < usedRegisters.size(); i++) {
      // STR REG_USED, DataBlock[Offset(REG_UNUSED)]
      prepend(instru, SaveReg(usedRegisters[i], Offset(usedRegisters[i])).generate(patch.metadata.cpuMode));
    }

    // In the break to host case the first used register is not restored and instead given to
    // the break to host code as a scratch. It will later be restored by the break to host code.
    if (breakToHost) {
      for(uint32_t i = 1; i < usedRegisters.size(); i++) {
        append(instru, LoadReg(usedRegisters[i], Offset(usedRegisters[i])).generate(patch.metadata.cpuMode));
      }
      append(instru, getBreakToHost(usedRegisters[0], patch.metadata.cpuMode));
    }
    // Normal case where we append the temporary register restoration code to the instrumentation
    else {
      for(uint32_t i = 0; i < usedRegisters.size(); i++) {
        append(instru, LoadReg(usedRegisters[i], Offset(usedRegisters[i])).generate(patch.metadata.cpuMode));
      }
    }


    // The resulting instrumentation is either appended or prepended as per the InstPosition
    if (position == PREINST) {
      patch.prepend(instru);
    } else if (position == POSTINST) {
      patch.append(instru);
    }
}

}
