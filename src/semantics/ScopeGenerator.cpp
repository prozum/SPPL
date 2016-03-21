
#include "Scope.h"
#include <iostream>
#include "ScopeGenerator.h"

namespace semantics {
    ScopeGenerator::ScopeGenerator() : is_valid(true), printer(cerr), error(false) {

    }

    void ScopeGenerator::visit(Program &node) {
        res = new Scope();
        current_scope = res;
        garbage.push_back(current_scope);

        /* Visit all children */
        for (auto func : node.funcs) {
            func->accept(*this);
        }
        /* Visit stops here */
    }

    void ScopeGenerator::visit(Function &node) {
        current_func = &node;
        node.scope = current_scope;

        if (!current_scope->exists(node.id)) {
            auto type = new Type(Types::SIGNATURE, node.types);
            current_scope->decls.insert({node.id, type});
            garbage.push_back(type);

            /* Visit children */
            for (auto type : node.types) {
                type->accept(*this);
            }
            for (auto cse : node.cases) {
                cse->accept(*this);
                error = false;
            }
            /* Visit stops here */
        } else {
            is_valid = false;
            cerr << "error - line " << node.line_no << ": " << node.id << " has already been declared.";
            node.accept(printer);
            cerr << endl;
            return;
        }
    }

    void ScopeGenerator::visit(Case &node) {
        auto case_scope = new Scope(current_scope);
        current_scope->children.push_back(case_scope);
        current_scope = case_scope;
        garbage.push_back(current_scope);

        if (node.patterns.size() == current_func->types.size() - 1) {

            /* Visit children */
            context = ScopeContext::PATTERN;

            for (size_t i = 0; i < node.patterns.size(); i++) {
                type_stack.push(current_func->types[i]);
                node.patterns[i]->accept(*this);
                type_stack.pop();
            }

            context = ScopeContext::EXPR;
            node.expr->accept(*this);
        } else {
            is_valid = false;
            cerr << "error - line " << node.line_no << ": Case didn't have the correct number of patterns.";
            cerr << " Expected patterns: " << current_func->types.size() - 1 << ".";
            cerr << " Actual patterns: " << node.patterns.size() << "." << endl;
            node.accept(printer);
            cerr << endl;
            return;
        }

        current_scope = current_scope->parent;
    }

    void ScopeGenerator::visit(Or &node) {
        /* Visit children */
        node.left->accept(*this);
        node.right->accept(*this);

        if (error)
            return;
        /* Visit stops here */
    }

    void ScopeGenerator::visit(And &node) {
        /* Visit children */
        node.left->accept(*this);
        node.right->accept(*this);

        if (error)
            return;
        /* Visit stops here */
    }

    void ScopeGenerator::visit(Equal &node) {
        /* Visit children */
        node.left->accept(*this);
        node.right->accept(*this);

        if (error)
            return;
        /* Visit stops here */
    }

    void ScopeGenerator::visit(NotEqual &node) {
        /* Visit children */
        node.left->accept(*this);
        node.right->accept(*this);

        if (error)
            return;
        /* Visit stops here */
    }

    void ScopeGenerator::visit(Lesser &node) {
        /* Visit children */
        node.left->accept(*this);
        node.right->accept(*this);

        if (error)
            return;
        /* Visit stops here */
    }

    void ScopeGenerator::visit(Greater &node) {
        /* Visit children */
        node.left->accept(*this);
        node.right->accept(*this);

        if (error)
            return;
        /* Visit stops here */
    }

    void ScopeGenerator::visit(LesserEq &node) {
        /* Visit children */
        node.left->accept(*this);
        node.right->accept(*this);

        if (error)
            return;
        /* Visit stops here */
    }

    void ScopeGenerator::visit(GreaterEq &node) {
        /* Visit children */
        node.left->accept(*this);
        node.right->accept(*this);

        if (error)
            return;
        /* Visit stops here */
    }

    void ScopeGenerator::visit(Add &node) {
        /* Visit children */
        node.left->accept(*this);
        node.right->accept(*this);

        if (error)
            return;
        /* Visit stops here */
    }

    void ScopeGenerator::visit(Sub &node) {
        /* Visit children */
        node.left->accept(*this);
        node.right->accept(*this);

        if (error)
            return;
        /* Visit stops here */
    }

    void ScopeGenerator::visit(Mul &node) {
        /* Visit children */
        node.left->accept(*this);
        node.right->accept(*this);

        if (error)
            return;
        /* Visit stops here */
    }

    void ScopeGenerator::visit(Div &node) {
        /* Visit children */
        node.left->accept(*this);
        node.right->accept(*this);

        if (error)
            return;
        /* Visit stops here */
    }

    void ScopeGenerator::visit(Mod &node) {
        /* Visit children */
        node.left->accept(*this);
        node.right->accept(*this);

        if (error)
            return;
        /* Visit stops here */
    }

    void ScopeGenerator::visit(ListAdd &node) {
        /* Visit children */
        node.left->accept(*this);
        node.right->accept(*this);

        if (error)
            return;
        /* Visit stops here */
    }

    void ScopeGenerator::visit(Par &node) {
        /* Visit children */
        node.child->accept(*this);

        if (error)
            return;
        /* Visit stops here */
    }

    void ScopeGenerator::visit(Not &node) {
        /* Visit children */
        node.child->accept(*this);

        if (error)
            return;
        /* Visit stops here */
    }

    void ScopeGenerator::visit(Int &node) {

    }

    void ScopeGenerator::visit(Float &node) {

    }

    void ScopeGenerator::visit(Bool &node) {

    }

    void ScopeGenerator::visit(Char &node) {

    }

    void ScopeGenerator::visit(String &node) {

    }

    void ScopeGenerator::visit(ListPattern &node) {
        if (type_stack.top()->type == Types::LIST) {
            type_stack.push(type_stack.top()->types[0]);

            /* Visit children */
            for (auto pattern : node.patterns) {
                pattern->accept(*this);

                if (error)
                    return;
            }
            /* Visit stops here */

            type_stack.pop();
        } else {
            is_valid = false;
            error = true;
            cerr << "error - line " << node.line_no << ": Pattern is not consistent with the function signature.";
            cerr << " Expected: ";
            type_stack.top()->accept(printer);
            cerr << ", not a List pattern." << endl;
            node.accept(printer);
            cerr << endl;
            return;
        }

    }

    void ScopeGenerator::visit(TuplePattern &node) {
        if (type_stack.top()->type == Types::TUPLE) {
            if (node.patterns.size() == type_stack.top()->types.size()) {
                /* Visit children */
                for (size_t i = 0; i < node.patterns.size(); i++) {
                    type_stack.push(type_stack.top()->types[i]);
                    node.patterns[i]->accept(*this);
                    type_stack.pop();

                    if (error)
                        return;
                }
            } else {
                is_valid = false;
                error = true;
                cerr << "error - line " << node.line_no << ": Tuple pattern did not specify the correct number of items.";
                cerr << " Expected size: " << type_stack.top()->types.size();
                cerr << " Actual size: " << node.patterns.size() << endl;
                node.accept(printer);
                cerr << endl;
                return;
            }
        } else {
            is_valid = false;
            error = true;
            cerr << "error - line " << node.line_no << ": Pattern is not consistent with the function signature.";
            cerr << " Expected: ";
            type_stack.top()->accept(printer);
            cerr << ", not a Tuple pattern." << endl;
            node.accept(printer);
            cerr << endl;
            return;
        }
    }

    void ScopeGenerator::visit(ListSplit &node) {
        if (type_stack.top()->type == Types::LIST) {
            type_stack.push(type_stack.top()->types[0]);

            /* Visit children */
            node.left->accept(*this);
            type_stack.pop();
            node.right->accept(*this);

            if (error)
                return;
            /* Visit stops here */
        } else if (type_stack.top()->type == Types::STRING){
            auto _char = new Type(Types::CHAR);
            type_stack.push(_char);
            garbage.push_back(_char);

            /* Visit children */
            node.left->accept(*this);
            type_stack.pop();
            node.right->accept(*this);

            if (error)
                return;
            /* Visit stops here */
        } else {
            is_valid = false;
            error = true;
            cerr << "error - line " << node.line_no << ": Can't split type ";
            type_stack.top()->accept(printer);
            cerr << "." << endl;
            node.accept(printer);
            cerr << endl;
            return;
        }
    }

    void ScopeGenerator::visit(List &node) {
        /* Visit children */
        for (auto expr : node.exprs) {
            expr->accept(*this);

            if (error)
                return;
        }
        /* Visit stops here */
    }

    void ScopeGenerator::visit(Tuple &node) {
        /* Visit children */
        for (auto expr : node.exprs) {
            expr->accept(*this);

            if (error)
                return;
        }
        /* Visit stops here */
    }

    void ScopeGenerator::visit(Id &node) {
        node.scope = current_scope;

        if (context == ScopeContext::PATTERN) {
            if (!current_scope->exists(node.id)) {
                current_scope->decls.insert({node.id, type_stack.top()});
            } else {
                is_valid = false;
                error = true;
                cerr << "error - line " << node.line_no << ": Can't declare an id twice in same scope." << endl;
                node.accept(printer);
                cerr << endl;
                return;
            }
        }
    }

    void ScopeGenerator::visit(Call &node) {
        /* Visit children */
        node.callee->accept(*this);
        for (auto expr : node.exprs) {
            expr->accept(*this);

            if (error)
                return;
        }
        /* Visit stops here */
    }

    void ScopeGenerator::visit(Type &node) {

    }
}