#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "Lexer.h"
#include "parser.h"
#include "SemanticAnalyzer.h"
#include "IRGenerator.h"

using namespace std;

int main(int argc, char** argv) {
    // Ensure the user provided a file
    if (argc < 2) {
        cerr << "Usage: kizen <source_file.kzn>" << endl;
        return 1;
    }

    // DECLARE IT ONLY ONCE HERE:
    string filename = argv[1];

    // Check the extension
    if (filename.substr(filename.find_last_of(".") + 1) != "kzn") {
        cerr << "Error: Kizen-N compiler only accepts '.kzn' files." << endl;
        return 1;
    }
    // Read the Tejas source file
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not open file '" << filename << "'" << endl;
        return 1;
    }
    stringstream buffer;
    buffer << file.rdbuf();
    string sourceCode = buffer.str();

    try {
        // [PHASE 1]: Lexical Analysis
        Lexer lexer(sourceCode);
        vector<Token> tokens;
        Token t = lexer.getNextToken();
        while (t.type != TokenType::EF) {
            tokens.push_back(t);
            t = lexer.getNextToken();
        }
        tokens.push_back(t); // Push EOF

        // [PHASE 2]: Parsing (AST Generation)
        Parser parser(tokens);
        auto ast = parser.parse_program();

        // [PHASE 3]: Semantic Analysis (Type Checking)
        SemanticAnalyzer semanticAnalyzer;
        semanticAnalyzer.analyze(ast.get());

        // [PHASE 4]: LLVM IR Generation
        IRGenerator irGenerator;
        irGenerator.visit(ast.get()); 

        // [PHASE 5]: Object File Generation
        // Note: Make sure this calls whatever function you currently use 
        // to spit out "output.o" at the end of your compiler pipeline!
        irGenerator.generateObjectFile("output.o");

    } catch (const std::exception& e) {
        // If there is a syntax or semantic error, print it and stop compiling
        cerr << "\n❌ COMPILATION FAILED:\n" << e.what() << endl;
        return 1;
    }

    return 0;
}