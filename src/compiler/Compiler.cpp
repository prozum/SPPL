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
            case Backend::CPP:
                generator = make_unique<codegen::CCodeGenerator>(*output, *header_output);
                break;
#endif
#ifdef CGNUASM
            case Backend::GNUASM:
                generator = make_unique<codegen::GasCodeGenerator>(*output);
                break;
#endif
#ifdef CHASKELL
            case Backend::HASKELL:
                generator = make_unique<codegen::HCodeGenerator>(*output);
                break;
#endif
#ifdef CLLVM
            case Backend::LLVM:
                generator = make_unique<codegen::LLVMCodeGenerator>(*output);
                break;
#endif
            case Backend::PPRINTER:
                generator = make_unique<codegen::Printer>(*output);
            default:
                throw "Not a valid backend";
        }
    }

    void Compiler::compile() {

        driver.parse_stream(*input);
        if (driver.main == nullptr)
            return;

        case_checker.visit(*driver.main);
        scope_generator.visit(*driver.main);
        type_checker.visit(*driver.main);

        generator->visit(*driver.main);
    }


}
