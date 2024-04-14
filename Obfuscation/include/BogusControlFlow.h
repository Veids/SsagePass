#ifndef _BOGUSCONTROLFLOW_H_
#define _BOGUSCONTROLFLOW_H_
// LLVM libs
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
// System libs
#include <list>
#include <memory>
// User libs
#include "CryptoUtils.h"
#include "Utils.h"
using namespace std;
using namespace llvm;
namespace llvm{ // 基本块分割
    class BogusControlFlowPass : public PassInfoMixin<BogusControlFlowPass>{
        public:
            bool flag;
            BogusControlFlowPass(bool flag){
                this->flag = flag;
            } // 携带flag的构造函数
            PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM); // Pass实现函数
            void bogus(Function &F);
            void addBogusFlow(BasicBlock *basicBlock, Function &F);
            BasicBlock *createAlteredBasicBlock(BasicBlock *basicBlock, const Twine &Name, Function *F);
            bool doF(Function &F);
            static bool isRequired() { return true; } // 直接返回true即可
        private:
            bool containsCoroBeginInst(BasicBlock *b);
            bool containsMustTailCall(BasicBlock *b);
            bool containsSwiftError(BasicBlock *b);
            SmallVector<const ICmpInst *, 8> needtoedit;
    };
    BogusControlFlowPass *createBogusControlFlow(bool flag); // 创建基本块分割
}

#endif
