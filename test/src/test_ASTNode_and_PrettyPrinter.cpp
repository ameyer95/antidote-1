#include "catch.hpp"
#include "ASTNode.h"
#include "PrettyPrinter.h"
#include <string>
using namespace std;

// I don't know a good way to test individual units
// other than to build a tree
// and check that the pretty printer makes the expected code string

const int DEPTH_0_CODE_NUM_LINES = 2;
const char * const DEPTH_0_CODE_LINES[2] = {
"p <- summary(T);\n",
"return p;\n"
};

const int DEPTH_1_CODE_NUM_LINES = 12;
const char * const DEPTH_1_CODE_LINES[12] = {
"if(impurity(T) = 0) {\n",
"  p <- summary(T);\n",
"} else {\n",
"  phi <- bestsplit(T);\n",
"  if(x models phi) {\n",
"    T <- filter(T, phi);\n",
"  } else {\n",
"    T <- filter(T, not phi);\n",
"  }\n",
"  p <- summary(T);\n",
"}\n",
"return p;\n"
};

const int DEPTH_2_CODE_NUM_LINES = 22;
const char * const DEPTH_2_CODE_LINES[22] = {
"if(impurity(T) = 0) {\n",
"  p <- summary(T);\n",
"} else {\n",
"  phi <- bestsplit(T);\n",
"  if(x models phi) {\n",
"    T <- filter(T, phi);\n",
"  } else {\n",
"    T <- filter(T, not phi);\n",
"  }\n",
"  if(impurity(T) = 0) {\n",
"    p <- summary(T);\n",
"  } else {\n",
"    phi <- bestsplit(T);\n",
"    if(x models phi) {\n",
"      T <- filter(T, phi);\n",
"    } else {\n",
"      T <- filter(T, not phi);\n",
"    }\n",
"    p <- summary(T);\n",
"  }\n",
"}\n",
"return p;\n"
};


const int NUM_DEPTHS = 3;
const char * const * CODE_LINES[3] = {
    DEPTH_0_CODE_LINES, DEPTH_1_CODE_LINES, DEPTH_2_CODE_LINES
};
const int CODE_NUM_LINES[3] = {
    DEPTH_0_CODE_NUM_LINES, DEPTH_1_CODE_NUM_LINES, DEPTH_2_CODE_NUM_LINES
};

string lines_to_string(const char * const *lines, const int num_lines) {
    string ret = "";
    for(int i = 0; i < num_lines; i++) {
        ret += lines[i];
    }
    return ret;
}

TEST_CASE("Check ASTNode::buildTree has expected PrettyPrinter::getString") {
    ASTNode *program;
    PrettyPrinter p;
    string code_string, expected_code_string;

    for(int i = 0; i < NUM_DEPTHS; i++) {
        // catch2 runs each path through a section individually
        SECTION("Depth " + to_string(i)) {
            program = ASTNode::buildTree(i);
            expected_code_string = lines_to_string(CODE_LINES[i], CODE_NUM_LINES[i]);
        }
    }

    program->accept(p);
    code_string = p.getString();

    REQUIRE(code_string == expected_code_string);
}
