#include "CppCodeGenerator.h"
#include <string>
#include <iostream>
#include <sstream>
#include <Scope.h>

using namespace common;
using namespace std;

namespace codegen
{
    CCodeGenerator::CCodeGenerator(shared_ptr<std::ostream> &out, shared_ptr<std::ostream> &head)
            : CodeGenerator::CodeGenerator(out), header(head)
    {
        _char = make_shared<Type>(Types::CHAR);
        real_string = make_shared<Type>(Types::LIST);
        fake_string = make_shared<Type>(Types::STRING);
        string_list = make_shared<Type>(Types::LIST);
        real_string->types.push_back(_char);
        string_list->types.push_back(fake_string);
        list_offsets.push_back(0);
    }

    void CCodeGenerator::visit(Program &node)
    {
        Function* main = nullptr;
        generate_std();

        for (auto i : node.funcs)
            if (i->id == "main")
                main = i.get();

        if (!main)
            throw "No main, help!";

        main->node_type->accept(*this);

        *output << "#include \"test.h\" \n"
                  " \n"
                  "int main(int argc, char** argv) { \n"
                  "    " << lists[*string_list] << " *args = " << g_generated << g_create << lists[*string_list] << "(0); \n"
                  "    int i; \n"
                  " \n"
                  "    for(i = argc - 1; i >= 0; i--) { \n"
                  "        args = " << g_generated << g_add << lists[*string_list] << "(args, " << g_generated << g_create << g_string << "(argv[i])); \n"
                  "    } \n"
                  " \n"
                  "    " << g_generated << g_print << g_string << "("  << to_strings[*main->node_type] << "(" <<  g_user << g_main << "(args))); \n"
                  "    return 0; \n"
                  "} \n"
                  " \n";

        for (auto f : node.funcs) {
            f->accept(*this);
            *output << endl;
        }
    }

    void CCodeGenerator::visit(Function &node)
    {
        stringstream function;
        stringstream arg_name;

        current_func = &node;

        node.types.back()->accept(*this);

        // generate function name and return type
        function << last_type << " " << g_user << node.id << "(";

        // generate function arguments
        for (size_t i = 0; i < node.types.size() - 1; i++) {
            node.types[i]->accept(*this);
            arg_name << g_generated << g_arg << i;
            arg_names.push_back(arg_name.str());
            function << last_type << " " << arg_name.str();

            if (i < node.types.size() - 2) // don't generate ", " after last argument
                function << ", ";

            arg_name.str("");
        }

        function << ")";

        // generate function decleration in header
        *header << function.str() << ";" << endl;
        *header << endl;

        // generate function in *output
        *output << function.str() << " { \n";

        // generate cases
        for (auto c : node.cases) {
            c->accept(*this);
            *output << endl;

            // clear assigments specific for current case
            assignments.clear();
        }

        // generate error, for when program doesn't realize a case
        *output << "    printf(\"No cases realized!\\n\"); \n"
                  "    exit(1); \n"
                  "} \n"
                  " \n";

        // clear arg_names for current function
        arg_names.clear();
    }

    void CCodeGenerator::visit(Case &node)
    {
        stringstream pattern;
        bool empty = true;

        // generate if-statement for matching
        pattern << "    if (";

        id_context = IdContext::PATTERN;
        for (size_t i = 0; i < node.patterns.size(); i++) {
            // push arg_name on get_value_builder. get_value_builder is used for generate
            // assignments in a case
            get_value_builder.push_back(arg_names[i]);

            // generate pattern
            node.patterns[i]->accept(*this);

            // cleanup
            get_value_builder.pop_back();

            // only add pattern, if pattern is not "1"
            if (last_pattern != "1") {

                if (!empty)
                    pattern << " && ";

                empty = false;
                pattern << last_pattern;
            }
        }

        pattern << ")";

        // only generate if statement, if the pattern wasn't empty
        if (!empty) {
            *output << pattern.str() << endl;
        }

        *output << "    { \n";

        // generate all nessesary assigments
        for (auto assign : assignments) {
            *output << "        " << assign << endl;

            if (assign == assignments.back())
                *output << endl;
        }

        // generate return expression
        id_context = IdContext::EXPR;
        current_func->types.back()->accept(*this);
        *output << "        " << last_type << " res = ";
        node.expr->accept(*this);
        *output << ";" << endl;

        *output <<  "        return res; \n"
                   "    } \n"
                   " \n";
    }

    void CCodeGenerator::visit(Or &node)
    {
        *output << "(";
        node.left->accept(*this);
        *output << "||";
        node.right->accept(*this);
        *output << ")";
    }

    void CCodeGenerator::visit(And &node)
    {
        *output << "(";
        node.left->accept(*this);
        *output << "&&";
        node.right->accept(*this);
        *output << ")";
    }

    void CCodeGenerator::visit(Equal &node)
    {
        unordered_map<Type, string>::iterator got;
        string name;

        switch (node.left->node_type->type) {
            case Types::TUPLE:
                got = tuples.find(*node.left->node_type);

                if (got == tuples.end()) {
                    name = generate_tuple(*node.left->node_type);
                } else {
                    name = got->second;
                }

                *output << g_generated << g_compare << name << "(";
                node.left->accept(*this);
                *output << ", ";
                node.right->accept(*this);
                *output << ")";
                break;
            case Types::LIST:
            case Types::STRING:
                got = lists.find(*node.left->node_type);

                if (got == tuples.end()) {
                    name = generate_list(*node.left->node_type);
                } else {
                    name = got->second;
                }

                *output << g_generated << g_compare << name << "(";
                node.left->accept(*this);
                *output << ", ";
                node.right->accept(*this);
                *output << ")";
                break;
            default:
                *output << "(";
                node.left->accept(*this);
                *output << "==";
                node.right->accept(*this);
                *output << ")";
                break;
        }
    }

    void CCodeGenerator::visit(NotEqual &node)
    {
        Equal eq;
        eq.left = node.left;
        eq.right = node.right;

        *output << "(!";
        eq.accept(*this);
        *output << ")";
    }

    void CCodeGenerator::visit(Lesser &node)
    {
        *output << "(";
        node.left->accept(*this);
        *output << "<";
        node.right->accept(*this);
        *output << ")";
    }

    void CCodeGenerator::visit(LesserEq &node)
    {
        *output << "(";
        node.left->accept(*this);
        *output << "<=";
        node.right->accept(*this);
        *output << ")";
    }

    void CCodeGenerator::visit(Greater &node)
    {
        *output << "(";
        node.left->accept(*this);
        *output << ">";
        node.right->accept(*this);
        *output << ")";
    }

    void CCodeGenerator::visit(GreaterEq &node)
    {
        *output << "(";
        node.left->accept(*this);
        *output << ">=";
        node.right->accept(*this);
        *output << ")";
    }

    void CCodeGenerator::visit(Add &node)
    {
        *output << "(";
        node.left->accept(*this);
        *output << "+";
        node.right->accept(*this);
        *output << ")";
    }

    void CCodeGenerator::visit(Sub &node)
    {
        *output << "(";
        node.left->accept(*this);
        *output << "-";
        node.right->accept(*this);
        *output << ")";
    }

    void CCodeGenerator::visit(Mul &node)
    {
        *output << "(";
        node.left->accept(*this);
        *output << "*";
        node.right->accept(*this);
        *output << ")";
    }

    void CCodeGenerator::visit(Div &node)
    {
        *output << "(";
        node.left->accept(*this);
        *output << "/";
        node.right->accept(*this);
        *output << ")";
    }

    void CCodeGenerator::visit(Mod &node)
    {
        *output << "(";
        node.left->accept(*this);
        *output << "%";
        node.right->accept(*this);
        *output << ")";
    }

    void CCodeGenerator::visit(ListAdd &node)
    {
        string name;
        auto got = lists.find(*node.right->node_type);

        // If current list doesn't exists. Create it
        if (got == lists.end()) {
            name = generate_list(*node.node_type);
        } else {
            name = got->second;
        }

        // use pregenerated push function to push thing onto list
        // look at generate_list(Type &type) for the generation of this function
        *output << g_generated << g_add << name << "(";
        node.right->accept(*this);
        *output << ", ";
        node.left->accept(*this);
        *output << ")";
    }

    void CCodeGenerator::visit(Par &node)
    {
        *output << "(";
        node.child->accept(*this);
        *output << ")";
    }

    void CCodeGenerator::visit(Not &node)
    {
        *output << "(";
        *output << "!";
        node.child->accept(*this);
        *output << ")";
    }

    void CCodeGenerator::visit(ListPattern &node)
    {
        // result is needed, so we don't generate something else, while the ListPattern is being generated
        stringstream result;
        string value_gotten = "";

        // visit nodes type, to ensure that a list of this type has be generated
        node.node_type->accept(*this);

        // generate the gotten value from get_value_builder
        for (auto str : get_value_builder) {
            value_gotten += str;
        }

        result << "(" << value_gotten << "->" << g_size << " - " << list_offsets.back() << " == " << node.patterns.size() << ")";

        for (size_t i = 0; i < node.patterns.size(); i++) {
            // add "gat_"name"(" and ", i + offset)" to get_value_builder
            get_value_builder.insert(get_value_builder.begin(),  g_generated + g_valueat + lists[*node.node_type] + "(");
            get_value_builder.push_back(", " + to_string(i + list_offsets.back()) + ")");

            // push new offset.
            // lists accesed later down in pattern should not be offseted by the same as current list.
            list_offsets.push_back(0);

            // generate pattern
            node.patterns[i]->accept(*this);

            // cleanup
            list_offsets.pop_back();
            get_value_builder.pop_back();
            get_value_builder.erase(get_value_builder.begin());

            // don't add pattern, if pattern is "1"
            if (last_pattern != "1") {
                result << " && " << last_pattern;
            }
        }

        last_pattern = "(" + result.str() + ")";
    }

    void CCodeGenerator::visit(TuplePattern &node)
    {
        // result is needed, so we don't start generating something in a signature in the header file
        stringstream result;
        bool empty = true;

        // visit nodes type, to ensure that a tuple of this type has be generated
        node.node_type->accept(*this);

        // iterate through all items in tuple
        for (size_t i = 0; i < node.patterns.size(); i++) {
            // add ".gi"i"" to get_value_builder
            get_value_builder.push_back("." + g_generated + g_item + to_string(i));

            // generate pattern
            node.patterns[i]->accept(*this);

            // cleanup
            get_value_builder.pop_back();

            // don't add pattern, if pattern is "1"
            if (last_pattern != "1") {
                if (!empty)
                    result << " && ";

                empty = false;
                result << last_pattern;
            }

        }

        // if empty, then let last_pattern be "1"
        if (empty) {
            last_pattern = "1";
        } else {
            last_pattern = "(" + result.str() + ")";
        }
    }

    void CCodeGenerator::visit(ListSplit &node)
    {
        stringstream result;
        bool empty = true;

        // add "gat_"name"(" and ", offset)" to get_value_builder
        // this is done, so that patterns on the left of node, will use
        // the first + offset item in the list
        get_value_builder.insert(get_value_builder.begin(), g_generated + g_valueat + lists[*node.node_type] + "(");
        get_value_builder.push_back(", " + to_string(list_offsets.back()) + ")");

        // push new offset.
        // lists accesed later down in pattern should not be offseted by the same as current list.
        list_offsets.push_back(0);

        // generate pattern
        node.left->accept(*this);

        // cleanup
        list_offsets.pop_back();
        get_value_builder.pop_back();
        get_value_builder.erase(get_value_builder.begin());

        // don't add pattern, if pattern is "1"
        if (last_pattern != "1") {
            empty = false;
            result << last_pattern;
        }

        // right side of a list split, will be the list, but with the first + offset elements missing.
        // this is why we track an offset, so that we don't clone a list, unless we have to.
        list_offsets[list_offsets.size() - 1]++;

        // generate pattern
        node.right->accept(*this);

        // reverse offset back to what it was
        list_offsets[list_offsets.size() - 1]--;

        // don't add pattern, if pattern is "1"
        if (last_pattern != "1") {
            if (!empty)
                result << " && ";

            empty = false;
            result << last_pattern;
        }

        // if empty, then let last_pattern be "1"
        if (empty) {
            last_pattern = "1";
        } else {
            last_pattern = "(" + result.str() + ")";
        }
    }

    void CCodeGenerator::visit(Int &node)
    {
        // if pattern, then generate an expression for matching on this pattern
        if (id_context == IdContext::PATTERN) {
            string value = "";

            for (auto str : get_value_builder) {
                value += str;
            }

            last_pattern = value + " == " + to_string(node.value);

        // else, just output value
        } else {
            *output << node.value;
        }
    }

    void CCodeGenerator::visit(Float &node)
    {
        // if pattern, then generate an expression for matching on this pattern
        if (id_context == IdContext::PATTERN) {
            string value = "";

            for (auto str : get_value_builder) {
                value += str;
            }

            last_pattern = value + " == " + to_string(node.value);

        // else, just output value
        } else {
            *output << node.value;
        }

    }

    void CCodeGenerator::visit(Bool &node)
    {
        // if pattern, then generate an expression for matching on this pattern
        if (id_context == IdContext::PATTERN) {
            string value = "";

            for (auto str : get_value_builder) {
                value += str;
            }

            last_pattern = value + " == " + to_string(node.value);

        // else, just output value
        } else {
            *output << node.value;
        }
    }

    void CCodeGenerator::visit(Char &node)
    {
        // if pattern, then generate an expression for matching on this pattern
        if (id_context == IdContext::PATTERN) {
            string value = "";

            for (auto str : get_value_builder) {
                value += str;
            }

            last_pattern = value + " == '" + node.value + "'";

        // else, just output value
        } else {
            *output << "'" << node.value << "'";
        }
    }

    void CCodeGenerator::visit(String &node)
    {
        // if pattern, then generate an expression for matching on this pattern
        if (id_context == IdContext::PATTERN) {
            string value = "";

            for (auto str : get_value_builder) {
                value += str;
            }

            // gcompare_string is generated by generate_std. it compares the custome string type, to a char*
            // it also takes an offset, for when list splits occur on strings
            last_pattern = g_generated + g_compare + g_string + "(" + value + ", \"" + node.value + "\", " + to_string(list_offsets.back()) + ")";

        // else, just output value
        } else {
            // gcreate_string is generate by generate_std. It creates string base on a char*
            *output << g_generated << g_create << g_string << "(\"" << node.value << "\")";
        }
    }

    void CCodeGenerator::visit(List &node)
    {
        string name;

        // find generated name for the list
        auto got = lists.find(*node.node_type);

        // If current list doesn't exists. Create it
        if (got == lists.end()) {
            name = generate_list(*node.node_type);
        } else {
            name = got->second;
        }

        // create list
        *output << g_generated << g_create << name << "(" << node.exprs.size();
        for (int i = node.exprs.size() - 1; i >= 0; i--) {
            *output << ", ";

            node.exprs[i]->accept(*this);
        }

        *output << ")";
    }

    void CCodeGenerator::visit(Tuple &node)
    {
        string name;

        // find generated name for the tuple
        auto got = tuples.find(*(node.node_type));

        // If current tuple doesn't exists. Create it
        if (got == tuples.end()) {
            name = generate_tuple(*node.node_type);
        } else {
            name = got->second;
        }

        // create tuple
        *output << g_generated << g_create << name << "(";
        for (auto expr : node.exprs){
            expr->accept(*this);

            if (expr != node.exprs.back())
                *output << ", ";
        }
        *output << ")";
    }

    void CCodeGenerator::visit(Id &node)
    {
        stringstream assign;
        string name = "";

        if (id_context == IdContext::PATTERN) {
            // created name from get_value_builder
            for (auto str : get_value_builder) {
                name += str;
            }

            // get id's type
            node.node_type->accept(*this);

            // generate an assignment for the id, which should occur after the if-statement
            assign << last_type << " " << g_user << node.id << " = ";

            if ((node.node_type->type == Types::LIST || node.node_type->type == Types::STRING) &&
                 list_offsets.back() > 0) {
                assign << g_generated << g_at << lists[*node.node_type] << "(" << name << ", " << list_offsets.back() << ");";
            } else {
                assign << name << ";";
            }

            // save the assigment untill after the pattern has been generated
            assignments.push_back(assign.str());

            // since and id, in a pattern is allways true, then last_pattern is just set to "1"
            last_pattern = "1";
        } else {
            *output << g_user << node.id;
        }
    }

    void CCodeGenerator::visit(Call &node)
    {
        *output << "(";
        // generate the callee (aka, the function being called)
        node.callee->accept(*this);

        // generate the arguments the function is being called with
        *output << "(";
        for (auto expr : node.exprs){
            expr->accept(*this);

            if (expr != node.exprs.back())
                *output << ", ";
        }
        *output << ")";

        *output << ")";
    }

    void CCodeGenerator::visit(Type &node) {
        unordered_map<Type, string>::iterator got;

        // generate the currect c type, based on sppl's types
        switch (node.type) {
            case Types::FLOAT:
                last_type = g_float;
                break;
            case Types::CHAR:
                last_type = g_char;
                break;
            case Types::INT:
                last_type = g_int;
                break;
            case Types::BOOL:
                last_type = g_bool;
                break;
            // for tuples, lists, signatures and strings, custom types will be generated
            case Types::TUPLE:
                // find custom type
                got = tuples.find(node);

                if (got != tuples.end()) {
                    last_type = got->second;
                } else {
                    // if type didn't exist, then generate it
                    last_type = generate_tuple(node);
                }
                break;
            case Types::SIGNATURE:
                // find custom type
                got = signatures.find(node);

                if (got != signatures.end()) {
                    last_type = got->second;
                } else {
                    // if type didn't exist, then generate it
                    last_type = generate_signature(node);
                }
                break;
            case Types::STRING:
                // the string type is generate at the start of the generation.
                // we therefor just use the name that this type was give.
                last_type = string_type_name;

                // strings are allocated on the heap, and we therefor need a pointer to it.
                last_type += "*";
                break;
            case Types::LIST:
                // find custom type
                got = lists.find(node);

                if (got != lists.end()) {
                    last_type = got->second;
                } else {
                    // if type didn't exist, then generate it
                    last_type = generate_list(node);
                }

                // lists are allocated on the heap, and we therefor need a pointer to it.
                last_type += "*";
                break;
            default:
                break;
        }
    }

    string CCodeGenerator::generate_list(Type &type, bool isstring) {
        // while generating a type, other types could be generated at the same time.
        // we therefor need to save our generated code in a temporary variable, so
        // that these generations doesn't write on top of eachother
        stringstream result;
        string name = g_generated + g_list;
        name += to_string(list_count);

        if (isstring){
            string_type_name = name;
            *header << "typedef struct " << string_type_name << " " << string_type_name << "; \n";
            *header << string_type_name << "* " << to_strings[Type(Types::BOOL)] << "(" << g_bool << " value); \n";
            *header << string_type_name << "* " << to_strings[Type(Types::INT)] << "(" << g_int << " value); \n";
            *header << string_type_name << "* " << to_strings[Type(Types::FLOAT)] << "(" << g_float << " value); \n";
            *header << string_type_name << "* " << to_strings[Type(Types::CHAR)] << "(" << g_char << " value); \n";
            *header << string_type_name << "* " << to_strings[Type(Types::STRING)] << "(" << string_type_name << "* value); \n";
            *header << string_type_name << "* " << g_generated << g_create << g_string << "(char*);\n\n";
        }

        // increase list count, so next list doesn't have the same name
        list_count++;

        // get name of the items types
        type.types.front()->accept(*this);

        /* generation of list starts here */
        result << "typedef struct " << name << " { \n"
                  "    struct " << name << "* " << g_next << "; \n"
                  "    " << last_type << " " << g_value << "; \n"
                  "    int " << g_empty << "; \n"
                  "    int " << g_size << "; \n"
                  "} " << name << "; \n"
                  " \n";
        /* generation of list ends here */

        /* generation of list push starts here */
        result << name << "* " << g_generated << g_add << name << "(" << name << "* current, " << last_type << " item) { \n"
                  "    " << name << "* res = malloc(sizeof(" << last_type << "));\n"
                  "    res->" << g_value << " = item; \n"
                  "    res->" << g_next << " = current; \n"
                  "    res->" << g_empty << " = 0; \n"
                  "    res->" << g_size << " = current->" << g_size << " + 1; \n"
                  "    return res; \n"
                  "} \n"
                  " \n";
        /* generation of list push ends here */

        /* generation of list constructer starts here */
        result << name << "* " << g_generated << g_create << name << "(int count, ...) { \n"
                  "    int i; \n"
                  "    va_list args; \n"
                  "    " << name << "* res = malloc(sizeof(" << last_type << ")); \n"
                  "    res->" << g_empty << " = 1; \n"
                  "    res->" << g_size << " = 0; \n"
                  "\n"
                  "    va_start(args, count); \n"
                  "\n"
                  "    for (i = 0; i < count; i++) { \n"
                  "        res = " << g_generated << g_add << name << "(res, va_arg(args, " << last_type << ")); \n"
                  "    } \n"
                  "\n"
                  "    va_end(args); \n"
                  "    return res; \n"
                  "} \n"
                  " \n";
        /* generation of list constructer ends here */

        /* generation of at function starts here */
        result << name << "* " << g_generated << g_at << name << "(" << name << "* current, int index) { \n"
                  "    while (index-- > 0) { \n"
                  "        if (current->" << g_empty << ") { \n"
                  "            return current; \n"
                  "        } \n"
                  "\n"
                  "        current = current->" << g_next << "; \n"
                  "    } \n"
                  "\n"
                  "    return current; \n"
                  "} \n"
                  " \n";

        result << last_type << " " << g_generated << g_valueat << name << "(" << name << "* current, int index) { \n"
                "    " << name << "* res = " << g_generated << g_at << name << "(current, index); \n"
                "\n"
                "    if (res->" << g_empty << ") { \n"
                "        printf(\"Out of bound! " << oob_count++ << "\\n\"); \n"
                "        exit(1); \n"
                "    }"
                "\n"
                "    return res->" << g_value << "; \n"
                "} \n"
                " \n";
        /* generation of at function ends here */

        /* generation of compare function starts here */
        result << "int " << g_generated << g_compare << name << "(" << name << "* list1, " << name << "* list2) { \n"
                  "    int i; \n"
                  "    if (list1->" << g_size << " != list2->" << g_size << ") { \n"
                  "         return 0; \n"
                  "    } \n"
                  " \n"
                  "    for (i = 0; i < list1->" << g_size << "; i++) { \n"
                  "        if(";

        switch (type.types.front()->type) {
            case Types::LIST:
                result << "!" << g_generated << g_compare << lists[*type.types.front()] <<
                        "(" << g_generated << g_valueat << name << "(list1, i), " << g_generated << g_valueat << name << "(list2, i))";
                break;
            case Types::TUPLE:
                result << "!" << g_generated << g_compare << tuples[*type.types.front()] <<
                        "(" << g_generated << g_valueat << name << "(list1, i), " << g_generated << g_valueat << name << "(list2, i))";
                break;
            default:
                result << g_generated << g_valueat << name << "(list1, i) != " << g_generated << g_valueat << name << "(list2, i)";
                break;
        }

        result << ") \n"
                  "            return 0; \n"
                  "    } \n"
                  " \n"
                  "    return 1; \n"
                  "} \n"
                  " \n";
        /* generation of compare function ends here */

        // generation of concat
        result << name << "* " << g_generated << g_concat << name << "(" << name << "* list1, " << name << "* list2) { \n"
                   "    int i; \n"
                   "    " << name << "* res = " << g_generated << g_create << name << "(0); \n"
                   " \n"
                   "    for (i = list2->" << g_size << " - 1; i >= 0; --i) { \n"
                   "        res = " << g_generated << g_add << name << "(res, " << g_generated << g_valueat << name << "(list2, i)); \n"
                   "    } \n"
                   " \n"
                   "    for (i = list1->" << g_size << " - 1; i >= 0; --i) { \n"
                   "        res = " << g_generated << g_add << name << "(res, " << g_generated << g_valueat << name << "(list1, i)); \n"
                   "    } \n"
                   " \n"
                   "    return res; \n"
                   "} \n"
                   "\n";

        // generation of tostring
        type.types.front()->accept(*this);
        auto childts = to_strings[*type.types.front()];
        to_strings[type] = g_generated + g_tostring + name;

        result << string_type_name << "* " << to_strings[type] << "(" << name << "* value) { \n"
                "    " << string_type_name << "* comma = " << g_generated << g_create << g_string << "(\", \"); \n"
                "    " << string_type_name << "* res = " << g_generated << g_create << g_string << "(\"]\"); \n"
                "    int i; \n"
                "\n"
                "    for (i = value->" << g_size << " - 1; i >= 0; i--) { \n"
                "        res = " << g_generated << g_concat << string_type_name << "(" << childts << "(" << g_generated << g_valueat << name << "(value, i)), res); \n"
                " \n"
                "        if (i != 0) \n"
                "            res = " << g_generated << g_concat << string_type_name << "(comma, res); \n"
                "    } \n"
                " \n"
                "    res = " << g_generated << g_add << string_type_name << "(res, '['); \n"
                "    return res; \n"
                "} \n"
                " \n";

        // writing list to headerfile
        *header << result.str();

        // save list in signature hash map
        lists[type] = name;

        // return name of list generated
        return name;
    }

    string CCodeGenerator::generate_signature(Type &type) {
        // result is needed, so we don't start generating something in a signature in the headerfile
        stringstream result;
        string name = g_generated + g_signature;
        name += to_string(sig_count);

        // increase signature count, so next signature doesn't have the same name
        sig_count++;

        type.types.back()->accept(*this);
        result << "typedef " << last_type << " (* " << name << ")(";

        for (size_t i = 0; i < type.types.size() - 1; ++i) {
            type.types[i]->accept(*this); // generate the actual type of the argument
            result << last_type;

            if (i < type.types.size() - 2) // don't print ", " after the last argument
                result << ", ";
        }

        result << ");" << endl;
        result << endl;

        // generation of signature to string
        to_strings[type] = g_generated + g_tostring + name;

        result << string_type_name << "* " << to_strings[type] << "(" << name << " value) { \n"
                   "    return " << g_generated << g_create << g_string << "(\"" << type.str() << "\"); \n"
                   "} \n"
                   " \n";

        // writing signature to headerfile
        *header << result.str();

        // save signature in signature hash map
        signatures[type] = name;

        // return name of signature generated
        return name;
    }

    string CCodeGenerator::generate_tuple(Type &type) {
        // result is needed, so we don't start generating something in a tuple in the headerfile
        stringstream result;
        string name = g_generated + g_tuple;
        name += to_string(tuple_count);

        // increase tuple count, so next tuple doesn't have the same name
        tuple_count++;

        /* generation of tuple starts here */
        result << "typedef struct " << name << " {" << endl;
        // generate an item for each type in the tuple
        for (size_t i = 0; i < type.types.size(); ++i) {
            type.types[i]->accept(*this); // generate the actual type of the item

            result << "    " << last_type << " " << g_generated << g_item << i << ";" << endl; // give this item a unique name

        }
        result << "} " << name << ";" << endl;
        result << endl;
        /* generation of tuple ends here */

        /* generation of tuple contructor starts here */
        // give contructor a unique name
        result << name << " " << g_generated << g_create << name << "(";

        // generate an argument for each item in the struct
        for (size_t i = 0; i < type.types.size(); ++i) {
            type.types[i]->accept(*this); // generate the actual type of the argument
            result << last_type << " " << g_generated << g_item << i;

            if (i < type.types.size() - 1) // don't print ", " after the last argument
                result << ", ";
        }

        result << ") {" << endl;

        // generate a result variable
        result << "    " << name << " " << "res; \n";

        // for each item in res, assign values
        for (size_t i = 0; i < type.types.size(); ++i) {
            result << "    res." << g_generated << g_item << i << " = " << g_generated << g_item << i << "; \n";
        }
        result << " \n"
                  "    return res; \n"
                  "} \n"
                  " \n";
        /* generation of tuple contructor ends here */

        /* generation of compare function starts here */
        result << "int " << g_generated << g_compare << name << "(" << name << " tuple1, " << name << " tuple2) { \n";

        for (size_t i = 0; i < type.types.size(); ++i) {
            result << "    if (";

            switch (type.types[i]->type) {
                case Types::STRING:
                case Types::LIST:
                    result << "!" << g_generated << g_compare << lists[*type.types[i]] << "(tuple1." << g_generated << g_item << i << ", tuple2." << g_generated << g_item << i << ")";
                    break;
                case Types::TUPLE:
                    result << "!" << g_generated << g_compare << tuples[*type.types[i]] << "(tuple1." << g_generated << g_item << i << ", tuple2." << g_generated << g_item << i << ")";
                    break;
                default:
                    result << "(tuple1." << g_generated << g_item << i << " != tuple2." << g_generated << g_item << i << ")";
                    break;
            }

            result << ") \n"
                      "        return 0; \n";
        }

        result << " \n"
                  "    return 1; \n"
                  "} \n"
                  "\n";
        /* generation of compare function ends here */

        // generation of tostring
        to_strings[type] = g_generated + g_tostring + name;

        result << string_type_name << "* " << to_strings[type] << "(" << name << " value) { \n"
                   "    " << string_type_name << "* comma = " << g_generated << g_create << g_string << "(\", \"); \n"
                   "    " << string_type_name << "* res = " << g_generated << g_create << g_string << "(\")\"); \n"
                   "\n";

        for (size_t i = type.types.size() - 1; i != 0; --i) {
            result << "    res = " << g_generated << g_concat << string_type_name << "(" << to_strings[*type.types[i]] << "(value." << g_generated << g_item << i << ")" << ", res); \n";
            result << "    res = " << g_generated << g_concat << string_type_name << "(comma, res); \n";
        }


        result << "    res = " << g_generated << g_concat << string_type_name << "(" << to_strings[*type.types.front()] << "(value." << g_generated << g_item << "0)" << ", res); \n"
                   "    res = " << g_generated << g_add << string_type_name << "(res, '('); \n"
                   " \n"
                   "    return res; \n"
                   "} \n"
                   "\n";


        // writing tuple to headerfile
        *header << result.str();

        // save tuple in tuple hash map
        tuples[type] = name;

        // return name of tuple generated
        return name;
    }

    void CCodeGenerator::generate_std() {
        *header << "#include <stdarg.h> \n"
                   "#include <stdio.h> \n"
                   "#include <stdlib.h> \n"
                   "#include <string.h> \n"
                   "#include <stdint.h> \n"
                   "#include <inttypes.h> \n"
                   " \n";

        to_strings[Type(Types::INT)] = g_generated + g_tostring + "int";
        to_strings[Type(Types::BOOL)] = g_generated + g_tostring + "bool";
        to_strings[Type(Types::FLOAT)] = g_generated + g_tostring + "float";
        to_strings[Type(Types::CHAR)] = g_generated + g_tostring + "char";
        to_strings[Type(Types::STRING)] = g_generated + g_tostring + "string";
        generate_list(*real_string, true);
        lists[*fake_string] = string_type_name;
        generate_list(*string_list);

        // generation of string constructer starts here
        *header << string_type_name << "* " << g_generated << g_create << g_string << "(char* values) { \n"
                  "    int i, str_length = strlen(values); \n"
                  "    " << string_type_name << "* res = " << g_generated << g_create << string_type_name << "(0); \n"
                  " \n"
                  "    for (i = str_length - 1; i >= 0; i--) { \n"
                  "        res = " << g_generated << g_add << string_type_name << "(res, values[i]); \n"
                  "    } \n"
                  " \n"
                  "    return res; \n"
                  "} \n"
                  " \n";

        // generation of string compare
        *header << "int " << g_generated << g_compare << g_string << "(" << string_type_name << "* string, char* values, int offset) { \n"
                  "    int i, size = strlen(values); \n"
                  " \n"
                  "    if (size == string->" << g_size << ") { \n"
                  "        for (i = 0; i < size; i++) { \n"
                  "            if (" << g_generated << g_valueat << string_type_name << "(string, i) != values[i]) \n"
                  "                return 0; \n"
                  "        } \n"
                  "    } else { \n"
                  "        return 0; \n"
                  "    } \n"
                  " \n"
                  "    return 1; \n"
                  "} \n"
                  " \n";

        // generation of print string
        *header << string_type_name << "* " << g_generated << g_print << g_string << "(" << string_type_name << "* string) { \n"
                   "    char* buffer = malloc(sizeof(char) * (string->" << g_size << " + 1)); \n"
                   "    int i; \n"
                   " \n"
                   "    for (i = 0; i < string->" << g_size << "; i++) { \n"
                   "        buffer[i] = (char)" << g_generated << g_valueat << string_type_name << "(string, i); \n"
                   "    } \n"
                   " \n"
                   "    buffer[i] = '\\0'; \n"
                   "    printf(\"%s\\n\", buffer); \n"
                   "    free(buffer); \n"
                   "    return string; \n"
                   "} \n"
                   " \n";

        // generation of default to_string methods
        *header << string_type_name << "* " << to_strings[Type(Types::INT)] << "(" << g_int << " value) { \n"
                   "    char buffer[100]; \n"
                   "    sprintf(buffer, \"%\" PRId64 \"\", value); \n"
                   "    return " << g_generated << g_create << g_string << "(buffer); \n"
                   "} \n"
                   " \n";

        *header << string_type_name << "* " << to_strings[Type(Types::BOOL)] << "(" << g_bool << " value) { \n"
                   "    if (value) \n"
                   "        return " << g_generated << g_create << g_string << "(\"True\"); \n"
                   "    else \n"
                   "        return " << g_generated << g_create << g_string << "(\"False\"); \n"
                   "} \n"
                   " \n";

        *header << string_type_name << "* " << to_strings[Type(Types::FLOAT)] << "(" << g_float << " value) { \n"
                   "    char buffer[100]; \n"
                   "    sprintf(buffer, \"%lf\", value); \n"
                   "    return " << g_generated << g_create << g_string << "(buffer); \n"
                   "} \n"
                   " \n";

        *header << string_type_name << "* " << to_strings[Type(Types::CHAR)] << "(" << g_char << " value) { \n"
                   "    " << string_type_name << "* res = " << g_generated << g_create << g_string << "(\"\\'\"); \n"
                   "    res = " << g_generated << g_add << string_type_name << "(res, value); \n"
                   "    res = " << g_generated << g_add << string_type_name << "(res, '\\''); \n"
                   "    return res; \n"
                   "} \n"
                   " \n";

        *header << string_type_name << "* " << to_strings[Type(Types::STRING)] << "(" << string_type_name << "* value) { \n"
                   "    " << string_type_name << "* res = " << g_generated << g_create << g_string << "(\"\\\"\"); \n"
                   "    res = " << g_generated << g_concat << string_type_name << "(value, res); \n"
                   "    res = " << g_generated << g_add << string_type_name << "(res, '\"'); \n"
                   "} \n"
                   " \n";
    }
}
