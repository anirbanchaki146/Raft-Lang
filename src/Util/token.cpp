#include <string>
#include <vector>
#include <iostream>

#include "Util/token.h"

void Token::dbPrint() {
    std::cout << 
    "{" << type << ", " << value << ", " << line << "}\n";
}