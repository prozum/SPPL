#include "Compiler.h"
#include "Printer.h"


namespace compiler {

    Compiler::Compiler(istream* in, ostream* out) : input(in), output(out)
    {
    }

    Compiler::Compiler(istream *in, ostream *out, ostream *hout) : input(in), output(out), header_output(hout) {

    }

    void Compiler::set_backend(Backend backend)
    {

        switch (backend)
        {
#ifdef CCPP
            #include "CppCodeGenerator.h"
            case Backend::CPP:
                generator = make_unique<codegen::CCodeGenerator>(*output, *header_output);
                break;
#endif
#ifdef CGNUASM
            #include "GasCodeGenerator.h"
            case Backend::GNUASM:
                generator = make_unique<codegen::GasCodeGenerator>(*output);
                break;
#endif CHASKELL
            #include "HCodeGenerator.h"
            case Backend::HASKELL:
                generator = make_unique<codegen::HCodeGenerator>(*output);
                break;
#endif
#ifdef CLLVM
            #include "LLVMCodeGenerator.h"
            case Backend::LLVM
                generator = make_unique<codegen::LLVMCodeGenerator>(*output);
                break;
#endif
            case Backend::PPRINTER:
                generator = make_unique<codegen::Printer>(*output);
        }
    }

    void Compiler::compile() {

        driver.parse_stream(*input);

        generator->visit(*driver.main);
    }

}
