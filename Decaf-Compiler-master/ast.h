#include <bits/stdc++.h>
#include <llvm/IR/Value.h>
using namespace std;

class CodeGenContext;

class Node
{
virtual	void print(){};
};

class Program;
class MetDecl;
class Location;
class Statement;
class VarsSub;
class Expr;
class MethodCall;
class NormalMethodCall;
class CalloutMethodCall;
class Binexpr;
class Unexpr;
class Literal;
class AssignmentStmt;
class ReturnStmt;
class ContStmt;
class Block;
class ForStmt;
class IFStmt;
class BreakStmt;
class FieldDecl;
class IntLiteral;
class Variable;



class Program : public Node
{
    public:
        vector<MetDecl> mtl_;
        vector<FieldDecl> fdl_;

        Program(vector<FieldDecl> f,vector<MetDecl> m)
        {
            mtl_=m;
            fdl_=f;
        }
        
        Program(vector<FieldDecl> f)
        {
            fdl_ = f;
        }
        
        Program(){}
        
        Program(vector<MetDecl> m)
        {
                mtl_=m;
        }

	virtual	void print(){};

	virtual llvm::Value* codeGen(CodeGenContext& context);
	
};



class Variable : public Node
{
    public:
        string id_;
        IntLiteral *lit_;
        int flag;
        Variable(string id, IntLiteral &lit)
        {
            id_=id;
            lit_=&lit;
            flag = 0;
        }

        Variable(string id)
        {
            id_=id;
            flag=1;
        }
	virtual	void print(){};

	virtual llvm::Value* codeGen(CodeGenContext& context);
};




class FieldDecl : public Node
{
    public:
        string type_;
        vector<Variable> var;

        FieldDecl(string type,vector<Variable> ivar)
        {
            type_=type;
            var = ivar;    
        }
	virtual llvm::Value* codeGen(CodeGenContext& context);
	virtual	void print(){};

};


class Statement : public Node
{
    
    public:
        int flag;
        AssignmentStmt *ass;
        ReturnStmt *rt;
        ContStmt *cn;
        Block *blk;
        ForStmt *fr;
        IFStmt *ifel;
        BreakStmt *brk;
        MethodCall *mc;

        Statement(){}

        Statement(AssignmentStmt &iass)
        {
            ass = &iass;
            flag=0;
        }

        Statement(ReturnStmt &irt)
        {
            rt=&irt;
            flag=1 ;
        }
    
        Statement(ContStmt &icn )
        {
            cn = &icn ;
            flag=2 ;
        }

        Statement(Block &iblk)
        {

            blk = &iblk ;
            flag= 3;
        }
        Statement(ForStmt &ifr)
        {

            fr = &ifr;
            flag=4 ;
        }
        Statement(IFStmt &iifel)
        {

            ifel = &iifel;
            flag=5 ;
        }
        Statement(BreakStmt &ibrk)
        {

            brk = &ibrk;
            flag = 6;
        }
        Statement(MethodCall &imc)
        {
            mc=&imc;
            flag=7;
        }

	virtual llvm::Value* codeGen(CodeGenContext& context);

	virtual	void print(){};
};


class Block : public Statement
{
    public:
        int flag ;
        vector<VarsSub> v_sub;
        vector<Statement> stmts;
        Block()
        {
            flag = -1;
        }
        Block(vector<VarsSub> vs, vector<Statement> st)
        {
            v_sub = vs;
            stmts = st;
            flag = 0 ;
        }
        Block(vector<Statement>st)
        {
            stmts = st;
            flag = 1;
        }
	virtual llvm::Value* codeGen(CodeGenContext& context);
	virtual	void print(){};
};




class MetDecl : public Node
{
    public:
        string ret_type;
        string id_;
        Block *bl;
        vector<VarsSub> var_decls;
        MetDecl (string rt, string id, vector<VarsSub> vars, Block &b)
        {
            ret_type = rt;
            id_ = id;
            bl = &b;
            var_decls = vars;
        }
        MetDecl (string rt, string id, Block &b)
        {
            ret_type = rt;
            id_ =id;
            bl = &b;
        }
	virtual llvm::Value* codeGen(CodeGenContext& context);
	virtual	void print(){};
};


class VarsSub : public Node
{
    public:
        string type_id;
        vector<string> vars;

        VarsSub(string id, vector<string> v)
        {
            type_id = id;
            vars = v;
        }
	virtual llvm::Value* codeGen(CodeGenContext& context);
	virtual	void print(){};
};

class Expr: public Node
{
    public:
        MethodCall * m;
        Binexpr *be;
        Location *lc;
        Literal *li;
        Unexpr *ue;
        int flag;
        Expr (Literal &ili)
        {
            
            li = &ili;
            flag = 3;
        }
        
        Expr(){}
        
        
        Expr( MethodCall &  im)
        {
            m = &im;
            flag = 0;
        }
        Expr(Binexpr &ibe)
        {
            be = &ibe;
            flag = 1;
        }
        Expr(Location &ilc)
        {
            lc = &ilc;
            flag = 2;
        }
        
        Expr (Unexpr &iue)
        {
            ue = &iue;
            flag = 4;
        }
	virtual llvm::Value* codeGen(CodeGenContext& context);
	virtual	void print(){};
};

class AssignmentStmt : public Statement
{
    public:
        Location *lval;
        string op;
        Expr e_;
        AssignmentStmt ( Location &val, string iop, Expr ine)
        {
            lval = &val;
            op = iop;
            e_ = ine;
        }
	virtual llvm::Value* codeGen(CodeGenContext& context);
	virtual	void print(){};
};

class ReturnStmt : public Statement 
{
    public:
        Expr *e_;
        int flag;
        ReturnStmt(){flag=1;}// No return value along with return statement
        ReturnStmt(Expr &ine)
        {
            e_=&ine;
            flag=0;
        };
	virtual llvm::Value* codeGen(CodeGenContext& context);
	virtual	void print(){};
};

class BreakStmt : public Statement
{
	virtual	void print(){};
};

class ContStmt : public Statement
{
	virtual	void print(){};
};

class Assign :public Node
{
    public:
        string id_;
        Expr e_;
        Assign()
        {}
        Assign (string id, Expr ine)
        {
            id_ = id;
            e_ = ine;
        }
	virtual llvm::Value* codeGen(CodeGenContext& context);
	virtual	void print(){};
};

class ForStmt : public Statement
{
    public:
        Assign ass;
        Expr e_;
        Block bl;
        ForStmt (Assign as, Expr e, Block b)
        {
            ass = as;
            e_ = e;
            bl = b;
        }
	virtual llvm::Value* codeGen(CodeGenContext& context);
	virtual	void print(){};
};

class IFStmt : public Statement
{
    public:
        Expr e_;
        Block ifb;
        Block elb;
        IFStmt(Expr e, Block ifbl, Block elbl)
        {
            e_ = e;
            ifb = ifbl;
            elb = elbl;
        }
	virtual llvm::Value* codeGen(CodeGenContext& context);
	virtual	void print(){};
};


class MethodCall : public Expr, Statement
{
    public:
        NormalMethodCall *nm;
        CalloutMethodCall * cm;
        int flag;
        MethodCall(){}
        MethodCall(NormalMethodCall &inm)
        {
            nm = &inm;
            flag = 0;
        }
        MethodCall(CalloutMethodCall &icm)
        {
            cm = &icm;
            flag=1;
        }
	virtual llvm::Value* codeGen(CodeGenContext& context);
	virtual	void print(){};
        
};

class NormalMethodCall : public MethodCall
{
    public:
        string id_;
        vector<Expr> el_;
        NormalMethodCall(string id, vector<Expr> el) //not using pointer here remember to put *$2
        {
            id_ = id;
            el_ = el;
        }
	virtual llvm::Value* codeGen(CodeGenContext& context);
	virtual	void print(){};
};

class CalloutSub : public Node
{
    public:
        string strlit;
        Expr e_;
	map <char,string> m;
        int flag;
        CalloutSub (string inc)
        {
		m['n'] = "\n";
		m['t'] = "\t";
		m['a'] = "\a";
		m['r'] = "\r";
		m['b'] = "\b";
		m['v'] = "\v";
		m['0'] = "\0";
		m['\''] = "\'";
		m['\"'] = "\"";
		m['\\'] = "\\";                                                                                                                      
		strlit = ""; 
		for(int i=0; i<inc.size(); i++)
                {   
                        if(inc[i] == '\\')
                        {   
                                strlit += m[inc[i+1]];
                                i++;
                        }   
                        else strlit += inc[i];
                }   
		cout<<"checkval: "<<strlit<<"checkvalue: "<<inc<<endl;
    //        strlit = inc;
                flag = 1;
        }
        CalloutSub (Expr ine)
        {
            e_ = ine;
            flag = 0;
        }
};


class CalloutMethodCall : public MethodCall
{
    public:
        string strlit;
	map <char,string> m;
        vector<CalloutSub> argmt;
        CalloutMethodCall(string inc, vector<CalloutSub> args)
        {
            strlit = inc;
            argmt = args;
        }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class Binexpr : public Expr
{
    public:
        string op_;
        Expr lhs;
        Expr rhs;
        Binexpr( Expr llhs,string opr, Expr rrhs)
        {
            op_ = opr;
            lhs = llhs;
            rhs = rrhs;
        }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class Unexpr : public Expr
{
    public :
        string op_;
        Expr sc;
        Unexpr( string op, Expr ss)
        {
            op_ = op;
            sc = ss;
        }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class Location :public Expr
{
    public :
        string id_;
        Expr e_;
        int flag;

        Location(){flag=-1;}
        Location(string id, Expr ine)
        {
            id_ = id;
            e_ = ine;
            flag = 0;
        }
        Location(string id)
        {
            id_ = id;
            flag = 1;
        }
        virtual void print()
        {
		cout<<"flag= "<<flag<<endl;
        }
        
};

class Literal : public Expr
{
    public:
        string val ;
        Literal(){}
        Literal(string value)
        {
		val = value;
	}
	virtual llvm::Value* codeGen(CodeGenContext& context);
	virtual	void print(){
	//	cout <<"ast "<< val<<endl;
	};
};

class IntLiteral : public Literal
{
    public:
        string val;
        IntLiteral (string  value)
        {
            val = value;
        }
        void setvalue(string value)
        {
            val = value;
        }

};

class BoolLiteral : public Literal
{
    public:
        string val;
        BoolLiteral (string value)
        {
            val = value;
        }
        void setvalue(string value)
        {
            val = value;
        }
};





