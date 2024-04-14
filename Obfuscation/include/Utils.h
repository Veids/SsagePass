#ifndef LLVM_UTILS_H
#define LLVM_UTILS_H
// LLVM libs
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/NoFolder.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/MDBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
// System libs
#include <map>
#include <set>
#include <vector>
#include <stdio.h>
#include <sstream>
// 常用宏定义
#define INIT_CONTEXT(F) CONTEXT=&F.getContext()
#define TYPE_I32 Type::getInt32Ty(*CONTEXT)
#define CONST_I32(V) ConstantInt::get(TYPE_I32, V, false)
#define CONST(T, V) ConstantInt::get(T, V)
extern llvm::LLVMContext *CONTEXT;
using namespace std;
namespace llvm{
    std::string readAnnotate(Function *f); // 读取llvm.global.annotations中的annotation值
    void fixStack(Function *F); // 修复PHI指令和逃逸变量
    bool toObfuscate(bool flag, llvm::Function *f, std::string const &attribute); // 判断是否开启混淆
    bool toObfuscateBoolOption(Function *f, std::string option, bool *val);
    bool toObfuscateUint32Option(Function *f, std::string option, uint32_t *val);
    bool hasApplePtrauth(Module *M);
    void FixFunctionConstantExpr(Function *Func);
    void FixBasicBlockConstantExpr(BasicBlock *BB);
    void turnOffOptimization(Function *f);
    bool readAnnotationMetadata(Function *f, std::string annotation);
    void writeAnnotationMetadata(Function *f, std::string annotation);
    bool AreUsersInOneFunction(GlobalVariable *GV);
    bool readAnnotationMetadataUint32OptVal(Function *f, std::string opt, uint32_t *val);
    bool readFlagUint32OptVal(Function *f, std::string opt, uint32_t *val);
    static const char obfkindid[] = "MD_obf";
}
#endif // LLVM_UTILS_H
