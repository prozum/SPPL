#include "Compiler.h"
#include "SpplJit.h"

#include <iostream>
#include <memory>
#include <istream>
#include <sstream>
#include <string>

int main(int argc, char *argv[]) {

    SpplJit::init_llvm();

    std::stringstream input;

    SpplJit jit(&input, &cout);

    string tmp("");
    while (tmp.compare("q") != 0)
    {
        cout << "input: ";
        cin >> tmp;
        jit.eval(tmp);
        cout << endl << endl;
    }
}
