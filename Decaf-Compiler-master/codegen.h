#include "ast.h"
#include <stack>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/CallingConv.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Analysis/Verifier.h>
#include <llvm/Assembly/PrintModulePass.h>
#include <llvm/IR/IRBuilder.h>
//#include <llvm/ModuleProvider.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/JIT.h>
#include <llvm/Support/raw_ostream.h>

using namespace llvm;

class CodeGenBlock {
	public:
		BasicBlock *block;
		std::map<std::string, Value*> locals;
};

class CodeGenContext {

	//Function *mainFunction;

	std::list<CodeGenBlock *> blocks;
	public:

	Module *module;
	CodeGenContext() { module = new Module("main", getGlobalContext()); }
	void generateCode(Program* root);	
	std::map<std::string, Value*>& locals() { return blocks.front()->locals; }
	
	std::list<CodeGenBlock *>::iterator it;
	Value* getVariableAlloca(string id)
	{
		Value *x =module->getGlobalVariable(id);
	
		for ( it=blocks.begin(); it != blocks.end(); ++it)
		{
			if((*it)->locals.find(id) != (*it)->locals.end())
				return (*it)->locals[id];
		}
			return x;
	}
	BasicBlock *currentBlock() { return blocks.front()->block; }
	void pushBlock(BasicBlock *block) { blocks.push_front(new CodeGenBlock()); blocks.front()->block = block; }
	void popBlock() { CodeGenBlock *top = blocks.front(); blocks.pop_front(); delete top; }
};
