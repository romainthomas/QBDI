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
#include "Patch/arm/InstrRules.h"
#include <iostream>
namespace QBDI {

/* Genreate a series of RelocatableInst which when appended to an instrumentation code trigger a
 * break to host. It receive in argument a temporary reg which will be used for computations then
 * finally restored.
*/
RelocatableInst::SharedPtrVec getBreakToHost(Reg regTmp, CPUMode cpuMode) {
    RelocatableInst::SharedPtrVec breakToHost;

    const size_t db_selector_offset = offsetof(Context, hostState.selector);
    const size_t patchSize = cpuMode == CPUMode::ARM ? (16 - 4): (16 + 1); // +1 For thumb mode
    breakToHost = {

      //BreakPoint(cpuMode),
      // Use the temporary register to compute PC + 16 which is the address which will follow this
      // patch and where the execution needs to be resumed:
      // temp = pc + 16
      Adr(cpuMode, regTmp, patchSize),

      // Set the selector to this address so the execution can be resumed when the exec block will be
      // reexecuted.
      // DataOffset[Offset(Selector)] := REG_TMP
      Str(cpuMode, regTmp, Offset(db_selector_offset)),

      // Restore the temporary register value
      // REG_TMP := DataOffset[Offset(REG_TMP)]
      Ldr(cpuMode, regTmp, Offset(regTmp)),

    };

    // Jump to the epilogue to break to the host
    append(breakToHost, JmpEpilogue().generate(cpuMode));


    return breakToHost;
}

}
