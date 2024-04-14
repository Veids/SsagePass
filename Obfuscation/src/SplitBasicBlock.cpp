/*
 *  LLVM StringEncryption Pass
    Copyright (C) 2017 Zhang(https://github.com/Naville/)
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.
    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
// User libs
#include "Utils.h"
#include "CryptoUtils.h"
#include "SplitBasicBlock.h"
// namespace
using namespace llvm;
using std::vector;

#define DEBUG_TYPE "split" // 调试标识
// Stats
STATISTIC(Split, "Basicblock splitted"); // 宏定义

// 可选的参数，指定一个基本块会被分裂成几个基本块，默认值为 3
static cl::opt<int> SplitNum("split_num", cl::init(3), cl::desc("Split <split_num> time(s) each BB")); 
// 貌似NEW PM暂时不支持这种传递

/**
 * @brief 新的实现方案
 * 
 * @param F 
 * @param AM 
 * @return PreservedAnalyses 
 */
PreservedAnalyses SplitBasicBlockPass::run(Function& F, FunctionAnalysisManager& AM) {
    // Check if the number of applications is correct
    if (!((SplitNum > 1) && (SplitNum <= 10))) {
      outs()
          << "Split application basic block percentage -split_num=x must be 1 "
             "< x <= 10";
      return PreservedAnalyses::all();
    } 

    Function *tmp = &F; // 传入的Function
    if (toObfuscate(flag, tmp, "split")){ // 判断什么函数需要开启混淆
        outs() << "\033[1;32m[SplitBasicBlock] Function : " << F.getName() << "\033[0m\n"; // 打印一下被混淆函数的symbol
        split(tmp); // 分割流程
        ++Split; // 计次
    }
    return PreservedAnalyses::none();
}

/**
 * @brief 对传入的基本块做分割
 * 
 * @param BB 
 */
void SplitBasicBlockPass::split(Function *F){
    SmallVector<BasicBlock *, 16> origBB;
    size_t split_ctr = 0;

    // Save all basic blocks
    for (BasicBlock &BB : *F)
      origBB.emplace_back(&BB);

    for (BasicBlock *currBB : origBB) {
      if (currBB->size() < 2 || containsPHI(currBB) ||
          containsSwiftError(currBB))
        continue;

      if ((size_t)SplitNum > currBB->size() - 1)
        split_ctr = currBB->size() - 1;
      else
        split_ctr = (size_t)SplitNum;

      // Generate splits point (count number of the LLVM instructions in the
      // current BB)
      SmallVector<size_t, 32> llvm_inst_ord;
      for (size_t i = 1; i < currBB->size(); ++i)
        llvm_inst_ord.emplace_back(i);

      // Shuffle
      split_point_shuffle(llvm_inst_ord);
      std::sort(llvm_inst_ord.begin(), llvm_inst_ord.begin() + split_ctr);

      // Split
      size_t llvm_inst_prev_offset = 0;
      BasicBlock::iterator curr_bb_it = currBB->begin();
      BasicBlock *curr_bb_offset = currBB;

      for (size_t i = 0; i < split_ctr; ++i) {
        for (size_t j = 0; j < llvm_inst_ord[i] - llvm_inst_prev_offset; ++j)
          ++curr_bb_it;

        llvm_inst_prev_offset = llvm_inst_ord[i];

        // https://github.com/eshard/obfuscator-llvm/commit/fff24c7a1555aa3ae7160056b06ba1e0b3d109db
        /* TODO: find a real fix or try with the probe-stack inline-asm when its
         * ready. See https://github.com/Rust-for-Linux/linux/issues/355.
         * Sometimes moving an alloca from the entry block to the second block
         * causes a segfault when using the "probe-stack" attribute (observed
         * with with Rust programs). To avoid this issue we just split the entry
         * block after the allocas in this case.
         */
        if (F->hasFnAttribute("probe-stack") && currBB->isEntryBlock() &&
            isa<AllocaInst>(curr_bb_it))
          continue;

        curr_bb_offset = curr_bb_offset->splitBasicBlock(
            curr_bb_it, curr_bb_offset->getName() + ".split");
      }
    }
}

/**
 * @brief 判断基本块是否包含PHI指令
 * 
 * @param BB 
 * @return true 
 * @return false 
 */
bool SplitBasicBlockPass::containsPHI(BasicBlock *BB){
    for (Instruction &I : *BB){
        if (isa<PHINode>(&I)){
            return true;
        }
    }
    return false;
}

bool SplitBasicBlockPass::containsSwiftError(BasicBlock *BB) {
    for (Instruction &I : *BB)
        if (AllocaInst *AI = dyn_cast<AllocaInst>(&I))
            if (AI->isSwiftError())
                return true;
    return false;
}

/**
 * @brief 辅助分割流程的函数
 * 
 * @param vec 
 */
void SplitBasicBlockPass::split_point_shuffle(SmallVector<size_t, 32> &vec) {
    int n = vec.size();
    for (int i = n - 1; i > 0; --i)
        std::swap(vec[i], vec[cryptoutils->get_range(i + 1)]);
}

/**
 * @brief 便于调用基本块分割
 *
 * @param flag
 * @return FunctionPass*
 */
SplitBasicBlockPass *llvm::createSplitBasicBlock(bool flag){
    return new SplitBasicBlockPass(flag);
}
