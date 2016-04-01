#include "TestMaster.h"

void TestMaster::setUp() {

}

void TestMaster::tearDown() {
    remove("source.sppl");
    remove("target.body");
    remove("target.head");
    status = 0;
}

void TestMaster::compileChecker(bool success) {
    in = ifstream("source.sppl");
    out = ofstream("target.body");
    out = ofstream("target.header");

    try {
        compiler::Compiler compiler(&in, &out, &hout);
        compiler.set_backend(backend);
        status = compiler.compile();
    }
    catch (...)
    {
        CPPUNIT_ASSERT(false);  // Exceptions should never be thrown by the compiler
    }

    if ((status != 0) == success) {
        CPPUNIT_ASSERT(false);
    } else {
        CPPUNIT_ASSERT(true);
    }

    remove("source.sppl");
    remove("target.body");
    remove("target.head");
}

// Addition - Integer

void TestMaster::addIntPosPos() {
    ofstream sourceFile("source.sppl");
    sourceFile << "def main : Int | = 2 + 2";
    sourceFile.close();
    compileChecker(true);
}

void TestMaster::addIntZeroZero() {
    ofstream sourceFile("source.sppl");
    sourceFile << "def main : Int | = 0 + 0";
    sourceFile.close();
    compileChecker(true);
}

void TestMaster::addIntPosNeg() {
    ofstream sourceFile("source.sppl");
    sourceFile << "def main : Int | = 2 + -2";
    sourceFile.close();
    compileChecker(true);
}

void TestMaster::addIntNegPos() {
    ofstream sourceFile("source.sppl");
    sourceFile << "def main : Int | = -2 + 2";
    sourceFile.close();
    compileChecker(true);
}

void TestMaster::addIntNegNeg() {
    ofstream sourceFile("source.sppl");
    sourceFile << "def main : Int | = -2 + -2";
    sourceFile.close();
    compileChecker(true);
}

// Addition - Float

void TestMaster::addFloatPosPos() {
    ofstream sourceFile("source.sppl");
    sourceFile << "def main : Float | = 2.0 + 2.0";
    sourceFile.close();
    compileChecker(true);
}

void TestMaster::addFloatZeroZero() {
    ofstream sourceFile("source.sppl");
    sourceFile << "def main : Float | = 0.0 + 0.0";
    sourceFile.close();
    compileChecker(true);
}

void TestMaster::addFloatPosNeg() {
    ofstream sourceFile("source.sppl");
    sourceFile << "def main : Float | = 2.0 + -2.0";
    sourceFile.close();
    compileChecker(true);
}

void TestMaster::addFloatNegPos() {
    ofstream sourceFile("source.sppl");
    sourceFile << "def main : Float | = -2.0 + 2.0";
    sourceFile.close();
    compileChecker(true);
}

void TestMaster::addFloatNegNeg() {
    ofstream sourceFile("source.sppl");
    sourceFile << "def main : Float | = -2.0 + -2.0";
    sourceFile.close();
    compileChecker(true);
}

// Addition - Boolean

void TestMaster::addBoolTrueTrue() {
    ofstream sourceFile("source.sppl");
    sourceFile << "def main : Bool | = True + True";
    sourceFile.close();
    compileChecker(false);
}

void TestMaster::addBoolTrueFalse() {
    ofstream sourceFile("source.sppl");
    sourceFile << "def main : Bool | = True + False";
    sourceFile.close();
    compileChecker(false);
}

void TestMaster::addBoolFalseTrue() {
    ofstream sourceFile("source.sppl");
    sourceFile << "def main : Bool | = False + True";
    sourceFile.close();
    compileChecker(false);
}

void TestMaster::addBoolFalseFalse() {
    ofstream sourceFile("source.sppl");
    sourceFile << "def main : Bool | = False + False";
    sourceFile.close();
    compileChecker(false);
}

// Addition - Character

void TestMaster::addCharStrStr() {
    ofstream sourceFile("source.sppl");
    sourceFile << "def main : Str | = \"string\" + \"string\"";
    sourceFile.close();
    compileChecker(false);
}

void TestMaster::addCharStrChar() {
    ofstream sourceFile("source.sppl");
    sourceFile << "def main : Str | = \"string\" + 'c'";
    sourceFile.close();
    compileChecker(false);
}

void TestMaster::addCharCharStr() {
    ofstream sourceFile("source.sppl");
    sourceFile << "def main : Char | = 'c' + \"string\"";
    sourceFile.close();
    compileChecker(false);
}

void TestMaster::addCharCharChar() {
    ofstream sourceFile("source.sppl");
    sourceFile << "def main : Char | = 'c' + 'c'";
    sourceFile.close();
    compileChecker(false);
}
