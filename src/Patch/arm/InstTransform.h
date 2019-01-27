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
#ifndef INST_TRANSFORM_ARM_H
#define INST_TRANSFORM_ARM_H

#include <memory>
#include <vector>

#include "llvm/MC/MCInst.h"

#include "Patch/Types.h"
#include "Patch/PatchUtils.h"
#include "Patch/InstTransform.h"

namespace QBDI {

class ThumbLDRpciTransform : public InstTransform, public AutoAlloc<InstTransform, ThumbLDRpciTransform> {
  public:
    ThumbLDRpciTransform(Temp tmp);
    void transform(llvm::MCInst &inst, rword address, size_t instSize, TempManager *temp_manager);

  private:
    Temp temp;
};

}

#endif
