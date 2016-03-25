#include "codegen.h"

static IRBuilder<> Builder(getGlobalContext());

void CodeGenContext::generateCode(Program* root)
{
	std::cout << "Generating code...\n";
	*root->codeGen(*this); /* emit bytecode for the toplevel block */
}

static Type *typeOf(string type) 
{
	if (type.compare("int") == 0) {
		return Type::getInt32Ty(getGlobalContext());
	}
	else if (type.compare("boolean")==0)
		return Type::getInt1Ty(getGlobalContext());

	return Type::getVoidTy(getGlobalContext());
}

Value *Program::codeGen(CodeGenContext& context)
{
	int i;
	cout<<"Program Created"<<endl;
	
	Value *last = NULL;

	for(int i=0;i<fdl_.size();i++)
		last = fdl_[i].codeGen(context);
	for(int i=0;i<mtl_.size();i++)
		last = mtl_[i].codeGen(context);
	return last;
}

Value *FieldDecl::codeGen(CodeGenContext& context)
{
	int i;
	cout<<"Field Dec Created"<<endl;
	for(i=0;i<var.size();i++)
	{
		if(var[i].flag == 1)
		{
			Module* mod = context.module;
			GlobalVariable* gvar_int32_f = new GlobalVariable(/*Module=*/*mod, 
				/*Type=*/typeOf(type_),
				/*isConstant=*/false,
				/*Linkage=*/GlobalValue::ExternalLinkage,
				/*Initializer=*/0, // has initializer, specified below
				var[i].id_
				);  
						// Global Variable Definitions
				gvar_int32_f->setInitializer(ConstantInt::get(typeOf(type_), 0, true));     
		}
		else if(var[i].flag == 0)
		{
			char s[1000006];
			strcpy(s,var[i].lit_->val.c_str());

			int value = atoi(s);
                        ArrayType* ArrayTy_0 = ArrayType::get(typeOf(type_),value);
                        PointerType* PointerTy_1 = PointerType::get(ArrayTy_0, 0); 
                        GlobalVariable* gvar_ptr = new GlobalVariable(/*Module=*/*context.module, 
                                /*Type=*/ArrayTy_0,
                                /*isConstant=*/false,
                                /*Linkage=*/GlobalValue::ExternalLinkage,
                                /*Initializer=*/0, // has initializer, specified below
                                var[i].id_
                                );  
                        gvar_ptr->setInitializer(ConstantAggregateZero::get(ArrayTy_0));

		}
	}
}

Value *Variable::codeGen(CodeGenContext& context)
{
}

Value *AssignmentStmt::codeGen(CodeGenContext& context)
{
	cout<<"Assignment Statement of: "<<lval->id_<<endl;
	int f=0;
	
	if(lval->flag == 1){
		Value *retAlloca = context.getVariableAlloca(lval->id_);
		
		if(retAlloca == NULL)
		{
//			cout<< " NOT LOCAL " <<lval->id_ << endl;
			if(context.module->getGlobalVariable(lval->id_))
	                        retAlloca =context.module->getGlobalVariable(lval->id_);
			else{
				cerr<<"Undeclared Variable in assignment statement"<<lval->id_<<endl;
				return NULL;
		        }
		}

		Value *st = e_.codeGen(context);

		if(st!=NULL)
		{
			if(f == 0)
				st = new StoreInst(st,retAlloca, context.currentBlock());
			else
				st = Builder.CreateStore(st, retAlloca);
		}

	return st;
	}

	else if(lval->flag == 0)
	{
	
			Value* pos= lval->e_.codeGen(context);
			cout<<"count: "<<pos<<endl;
		
			if(!pos) return NULL;
	
			Value *st = e_.codeGen(context);
			if(!st) return NULL;

			std::vector<Value*> indices;
			indices.push_back(ConstantInt::get(typeOf("int"),0,true));
			indices.push_back(pos);
		return new StoreInst(st,ConstantExpr::getGetElementPtr(context.module->getGlobalVariable(lval->id_),indices),context.currentBlock());
	}
}

Value *ReturnStmt::codeGen(CodeGenContext& context)
{
	if(flag==1)
		return ReturnInst::Create(getGlobalContext(),context.currentBlock());	
	else
	{
		Value *val=e_->codeGen(context);
		return ReturnInst::Create(getGlobalContext(),val,context.currentBlock());	
	}
}


Value *Assign::codeGen(CodeGenContext& context)
{

	Value *st = e_.codeGen(context);

	if(st!=NULL)
		st = new StoreInst(st, context.locals()[id_], context.currentBlock());

	return st;
}

Value *ForStmt::codeGen(CodeGenContext& context)
{	
	Value *StartVal = ass.codeGen(context);


	Function *func = Builder.GetInsertBlock()->getParent();
	BasicBlock *forLoop = BasicBlock::Create(getGlobalContext(), "loop");
	BasicBlock *forCond = BasicBlock::Create(getGlobalContext(), "cond");
	BasicBlock *forEnd = BasicBlock::Create(getGlobalContext(), "end");

	cout<<"For started"<<endl;

	Builder.CreateBr(forCond);
	func->getBasicBlockList().push_back(forCond);
	Builder.SetInsertPoint(forCond);
	context.pushBlock(forCond);


	cout << "Condition\n";
	Value *Cond = e_.codeGen(context);

	Value *condVal = Builder.CreateICmpNE(Cond,ConstantInt::get(Cond->getType(),0),"loopcond");
	Builder.CreateCondBr(condVal,forLoop, forEnd);
	context.popBlock();

	forCond = Builder.GetInsertBlock();

	func->getBasicBlockList().push_back(forLoop);
	Builder.SetInsertPoint(forLoop);
	context.pushBlock(forLoop);


	Value *loop = bl.codeGen(context);

	cout << "END\n";
	
	Builder.CreateBr(forCond);
	context.popBlock();
	forLoop = Builder.GetInsertBlock();

	func->getBasicBlockList().push_back(forEnd);
	Builder.SetInsertPoint(forEnd);
	context.pushBlock(forEnd);

	return NULL;
}

Value *IFStmt::codeGen(CodeGenContext& context)
{
	Value *cond = e_.codeGen(context);
        if(cond==NULL) return NULL;

        Function *func = Builder.GetInsertBlock()->getParent();
        BasicBlock *Then = BasicBlock::Create(getGlobalContext(), "if_then",func);
        BasicBlock *Else = BasicBlock::Create(getGlobalContext(), "if_else");
        BasicBlock *End = BasicBlock::Create(getGlobalContext(), "if_end");
        
        Value *condVal = Builder.CreateICmpNE(cond,ConstantInt::get(cond->getType(),0),"ifcond");
        Builder.CreateCondBr(condVal, Then, Else);
        Builder.SetInsertPoint(Then);
        context.pushBlock(Then);
        Value *thenVal = ifb.codeGen(context);
        Builder.CreateBr(End);
        context.popBlock();
        Then = Builder.GetInsertBlock();
        
        func->getBasicBlockList().push_back(Else);
        Builder.SetInsertPoint(Else);
        context.pushBlock(Else);

        Value *elseVal = elb.codeGen(context);
        Builder.CreateBr(End);
        context.popBlock();
	Else = Builder.GetInsertBlock();
        
	func->getBasicBlockList().push_back(End);
        Builder.SetInsertPoint(End);
        context.pushBlock(End);

        return NULL;
}

Value *Statement::codeGen(CodeGenContext& context)
{
	
	if(flag == 0)
		return 	ass->codeGen(context);

	if(flag==1)
		return rt->codeGen(context);

	if(flag == 3)
	{
		BasicBlock *bblock = BasicBlock::Create(getGlobalContext(), "block");
		context.pushBlock(bblock);
		blk->codeGen(context);
		context.popBlock();
	}

	if(flag==4)
		return fr->codeGen(context);
	
	if(flag==5)
		return ifel->codeGen(context);

	if(flag==7)
		return mc->codeGen(context);
	return NULL;
}
Value *NormalMethodCall::codeGen(CodeGenContext& context)
{
	Function *function = context.module->getFunction(id_);
	if(function==NULL)
	{
		cerr<<id_<<"function not declared"<<endl;
		return NULL;
	}
	int i;
	vector<Value*> args;

	for(i=0;i<el_.size();i++)
	{
		args.push_back(el_[i].codeGen(context));
	}	
	llvm::ArrayRef<llvm::Value*>  argsRef(args);

	CallInst *call = NULL;
	call = CallInst::Create(function, argsRef, "", context.currentBlock());
	return call;
}

Value *CalloutMethodCall::codeGen(CodeGenContext& context)
{
	std::vector<llvm::Type *> argsType;
	std::vector<llvm::Value *> Args;
	Value* xx; 
	
	for(int i = 0; i < argmt.size(); i++) {

		int x=argmt[i].flag;
		if(x == 1)
			 xx= Builder.CreateGlobalStringPtr(argmt[i].strlit.c_str()); 
		else 
			xx=argmt[i].e_.codeGen(context);

		argsType.push_back(xx->getType());
		Args.push_back(xx);
	}

	llvm::ArrayRef<llvm::Type*>  argsRef(argsType);
	llvm::ArrayRef<llvm::Value*>  argsRef1(Args);

	llvm::FunctionType *Ftype = llvm::FunctionType::get(Builder.getInt32Ty(), argsRef, false);
	llvm::Constant *Func = context.module->getOrInsertFunction(strlit, Ftype);

	if(!Func) return NULL;
	return Builder.CreateCall(Func, argsRef1);      

}

Value *MethodCall::codeGen(CodeGenContext& context)
{
	if(flag==0)
	{
		Value * call = nm->codeGen(context);
		return call;
	}
	else if(flag==1)
	{
		Value * call = cm->codeGen(context);
		return call;
	}
}

Value *Binexpr::codeGen(CodeGenContext& context)
{
	Instruction::BinaryOps instr;
	if(op_.compare("+")==0)
	{
		instr=Instruction::Add;
	}

	else if(op_.compare("-")==0)
	{
		instr=Instruction::Sub;
	}

	else if(op_.compare("*")==0)
	{
		instr=Instruction::Mul;
	}

	else if(op_.compare("/")==0)
	{
		instr=Instruction::SDiv;
	}
	
	else if(op_.compare("%")==0)
	{
		instr=Instruction::SRem;
	}
	
	else if(op_.compare("&&")==0)
	{
		instr=Instruction::And;
	}
	
	else if(op_.compare("||")==0)
	{
		instr=Instruction::Or;
	}

	else if(op_.compare("||")==0)
	{
		instr=Instruction::Or;
	}

	else if(op_.compare("==")==0)
	{
		return CmpInst::Create(Instruction::ICmp, ICmpInst::ICMP_EQ, lhs.codeGen(context), rhs.codeGen(context),"",context.currentBlock());
	}
	
	else if(op_.compare("<=")==0)
	{
		return CmpInst::Create(Instruction::ICmp, ICmpInst::ICMP_ULE, lhs.codeGen(context), rhs.codeGen(context),"",context.currentBlock());
	}

	else if(op_.compare(">=")==0)
	{
		return CmpInst::Create(Instruction::ICmp, ICmpInst::ICMP_UGE, lhs.codeGen(context), rhs.codeGen(context),"",context.currentBlock());
	}
	
	else if(op_.compare("<")==0)
	{
		return CmpInst::Create(Instruction::ICmp, ICmpInst::ICMP_ULT, lhs.codeGen(context), rhs.codeGen(context),"",context.currentBlock());
	}

	else if(op_.compare(">")==0)
	{
		return CmpInst::Create(Instruction::ICmp, ICmpInst::ICMP_UGT, lhs.codeGen(context), rhs.codeGen(context),"",context.currentBlock());
	}

	else if(op_.compare("!=")==0)
	{
		return CmpInst::Create(Instruction::ICmp, ICmpInst::ICMP_NE, lhs.codeGen(context), rhs.codeGen(context),"",context.currentBlock());
	}

	Value *bin=NULL;

	Value *l = lhs.codeGen(context);
	Value *r = rhs.codeGen(context);

	if(l!=NULL && r!=NULL){
		cout<<"BINDAJFNSJNFJFNNJS\n";
		return  BinaryOperator::Create(instr, l,r, "", context.currentBlock());
	}

	return bin;

}

Value *Unexpr::codeGen(CodeGenContext& context)
{
}

Value *Literal::codeGen(CodeGenContext& context)
{
	char s[1000006];
	strcpy(s,val.c_str());
	int value = atoi(s);
		return ConstantInt::get(Type::getInt32Ty(getGlobalContext()),value,true);
}


Value *Expr::codeGen(CodeGenContext& context)
{
	if(flag==0)
		return m->codeGen(context);

	if(flag == 1)
		return be->codeGen(context);		

	if(flag==2)
	{
		
		if(lc->flag == 1)
		{
			Value *retAlloca = context.getVariableAlloca(lc->id_);
		
			if(retAlloca == NULL)
			{
				if(context.module->getGlobalVariable(lc->id_)){
					retAlloca =context.module->getGlobalVariable(lc->id_);
					return Builder.CreateLoad(retAlloca, lc->id_.c_str());
				}
				else
				{
					cerr<<"Undeclared Variable in assignment statement"<<lc->id_<<endl;
					return NULL;
				}
			}
			return new LoadInst(retAlloca, "", false, context.currentBlock());
		}
		else
		{
			Value *pos=lc->e_.codeGen(context);

			if(!pos) return NULL;
	
			std::vector<Value*> indices;                                                                                 
			indices.push_back(ConstantInt::get(typeOf("int"), 0, true));
			indices.push_back(pos);

			if(context.module->getGlobalVariable(lc->id_))	
	     		  return new LoadInst( ConstantExpr::getGetElementPtr(context.module->getGlobalVariable(lc->id_), indices),"",false,context.currentBlock());
		}
	}
	if(flag == 4)
		return ue->codeGen(context);
	if(flag == 3)
	{
		return li->codeGen(context);
	}
	return NULL;
}

Value *Block::codeGen(CodeGenContext& context)
{
	int i;
	for(i=0;i<v_sub.size();i++)
		v_sub[i].codeGen(context);
	for(i=0;i<stmts.size();i++)
		stmts[i].codeGen(context);
	
}


Value *MetDecl::codeGen(CodeGenContext& context)
{
	cout<<"New Function"<<endl;
	int i;
	vector<Type*> argTypes;
	for(i=0;i<var_decls.size();i++)
	{
		argTypes.push_back(typeOf(var_decls[i].type_id));
	}
	ArrayRef<Type *> argsRef(argTypes);

	FunctionType *ftype = FunctionType::get(typeOf(ret_type),argsRef,false);
	Function *function = Function::Create(ftype,GlobalValue::InternalLinkage,id_,context.module);
	BasicBlock *bblock = BasicBlock::Create(getGlobalContext(), "entry", function, 0);
	Builder.SetInsertPoint(bblock);
	context.pushBlock(bblock);
	Function::arg_iterator args_it = function->arg_begin();
	for(i=0;i<var_decls.size();i++)
	{
		Value* argVal = var_decls[i].codeGen(context);
		new StoreInst(args_it++,argVal,context.currentBlock());
	}


	
	int f=0;
	if(!ret_type.compare("void")){
		for(int i=0;i<bl->stmts.size();i++)
		{
			if(bl->stmts[i].flag ==1)	
			{	
				f=1;
				break;
			}
		}
		if(f!=1)
			//ReturnInst::Create(getGlobalContext(),bblock);	
			bl->stmts.push_back(*(new Statement(*(new ReturnStmt()))));
	}

	bl->codeGen(context);
	context.popBlock();
	return function;
}


Value *VarsSub::codeGen(CodeGenContext& context)
{
	int i;

	AllocaInst *alloc;
	for(i=0;i<vars.size();i++)
	{
		cout<<"declaring "<<vars[i]<<endl;
		alloc = new AllocaInst(typeOf(type_id), vars[i], context.currentBlock());
		context.locals()[vars[i]] = alloc;
	}
	return alloc;
}



