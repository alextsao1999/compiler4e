//
// Created by 曹顺 on 2019/5/5.
//

#include "test.h"
#include "ast.h"
#include "ECodeParser.h"

DumpVisitor::DumpVisitor(ECode *code, ESub *current) : code(code), current(current) {}

void DumpVisitor::visit(ASTProgram *node) {
    for (auto &stmt : node->stmts) {
        if (stmt != nullptr) {
            stmt->accept(this);
            cout << endl;
        }
    }
}

void DumpVisitor::visit(ASTArgs *node) {
    cout << "(";
    for (auto &arg : node->args) {
        if (arg != nullptr) {
            arg->accept(this);
            if (arg != node->args.back()) {
                cout << ", ";
            }
        }
    }
    cout << ")";
}

void DumpVisitor::visit(ASTBlock *node) {
    indent += 4;
    for (auto &arg : node->element) {
        if (arg != nullptr) {
            print_indent();
            arg->accept(this);
            cout << endl;
        }
    }
    indent -= 4;
}

void DumpVisitor::visit(ASTFunCall *node) {
    if (node->key.value == 0) {
        // 空行
        return;
    }
    if (node->lib >= 0) {
        cout << (const char *) code->libraries[node->lib].info->m_pBeginCmdInfo[node->key.value].m_szName;
        node->args->accept(this);
        return;
    }
    if (node->key.type == KeyType_Sub) {
        for (auto & sub : code->subs) {
            if (sub.key.value == node->key.value) {
                cout << sub.name;
            }
        }
    } else if (node->key.type == KeyType_DllFunc) {
        for (auto & dll : code->dlls) {
            if (dll.key.index == node->key.index) {
                cout << dll.name;
            }
        }
    }
    node->args->accept(this);

}

void DumpVisitor::visit(ASTIfStmt *node) {
    print_indent();
    cout << "if (";
    node->condition->accept(this);
    cout << ")" << endl;
    node->then_block->accept(this);

    if (node->else_block != nullptr) {
        print_indent();
        cout << "else" << endl;
        node->else_block->accept(this);
    }
}

void DumpVisitor::visit(ASTLiteral *node) {
    switch (node->value.type) {
        case 0:
            cout << "null";
            break;
        case 1:
            cout << node->value.val_int;
            break;
        case 2:
            cout << node->value.val_bool;
            break;
        case 3:
            cout << "\"" << node->value.val_string << "\"";
            break;
        case 4:
            cout << node->value.val_time;
            break;
        case 5:
            cout << node->value.val_double;
            break;
        default:
            break;
    }
}

void DumpVisitor::visit(ASTConstant *node) {
    for (auto & constant : code->constants) {
        if (constant.key.value == node->key.value) {
            cout << "#" << constant.name;
        }
    }
}

void DumpVisitor::visit(ASTLibConstant *node) {
    cout << "#" << code->libraries[node->index].info->m_pLibConst[node->member].m_szName;
}

void DumpVisitor::visit(ASTAddress *node) {
    for (auto & sub : code->subs) {
        if (sub.key.value == node->key.value) {
            cout << "&" << sub.name;
        }
    }
}

void DumpVisitor::visit(ASTSubscript *node) {
    cout << "[";
    node->value->accept(this);
    cout << "]";
}

void DumpVisitor::visit(ASTEnumConstant *node) {

}

void DumpVisitor::visit(ASTStructMember *node) {
    for (auto & i : code->structs) {
        if (i.key.value == node->key.value) {
            for (auto & member : i.members) {
                if (member.key.value == node->member.value) {
                    cout << "." << member.name;
                    break;
                }
            }
            break;
        }
    }

}

void DumpVisitor::visit(ASTVariable *node) {
    if (node->key.type == KeyType_LocalOrParam) {
        for (auto & local : current->locals) {
            if (local.key.value == node->key.value) {
                cout << local.name;
                return;
            }
        }
        for (auto & param : current->params) {
            if (param.key.value == node->key.value) {
                cout << param.name;
                return;
            }
        }

    } else if (node->key.type == KeyType_ProgramVar) {
        EModule *module = current->belong;
        for (auto & var : module->vars) {
            if (var.key.value == node->key.value) {
                cout << var.name;
                return;
            }
        }
    }

}

void DumpVisitor::visit(ASTPostfix *node) {
    node->var->accept(this);
    node->field->accept(this);
}

void DumpVisitor::visit(ASTJudge *node) {
    for (int i = 0; i < node->conditions.size(); ++i) {
        print_indent();
        cout << "case ";
        node->conditions[i]->accept(this);
        cout << " : " << endl;
        node->blocks[i]->accept(this);
    }
    if (node->default_block != nullptr) {
        print_indent();
        cout << "default : " << endl;
        node->default_block->accept(this);
    }
}

void DumpVisitor::visit(ASTLoop *node) {
    print_indent();
    node->head->accept(this);
    cout << endl;
    node->block->accept(this);
    if (node->tail != nullptr) {
        print_indent();
        node->tail->accept(this);
    }
}

void DumpVisitor::print_indent() {
    for (int i = 0; i < indent; ++i) {
        cout << " ";
    }
}

void DumpVisitor::visit(ASTBrace *node) {
    cout << "{" ;
    node->args->accept(this);
    cout << "}";

}

