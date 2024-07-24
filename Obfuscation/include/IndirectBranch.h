#ifndef LLVM_INDIRECTBRANCH_H
#define LLVM_INDIRECTBRANCH_H
// LLVM libs
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/NoFolder.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/LowerSwitch.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"
// User libs
#include "Utils.h"
#include "CryptoUtils.h"
using namespace llvm;
using namespace std;
namespace llvm{ // 间接跳转
    class IndirectBranchPass : public PassInfoMixin<IndirectBranchPass>{
        public:
            bool flag;
            bool initialized;
            std::unordered_map<BasicBlock *, unsigned long long> indexmap;
            std::unordered_map<Function *, ConstantInt *> encmap;
            std::set<Function *> to_obf_funcs;
            IndirectBranchPass(bool flag){
                this->flag = flag;
            } // 携带flag的构造函数
            PreservedAnalyses run(Function &F, FunctionAnalysisManager &FM); // Pass实现函数
            bool initialize(Module &M); // 开始初始化
            PreservedAnalyses HandleFunction(Function &Func); // 处理间接跳转
            bool doFinalization(Module &M); // 结束初始化
            void shuffleBasicBlocks(Function &F);
    };
    IndirectBranchPass *createIndirectBranch(bool flag); // 创建间接跳转
}
#endif // LLVM_INDIRECTBRANCH_H
