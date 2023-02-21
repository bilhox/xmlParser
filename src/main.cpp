#include <iostream>
#include "tree.hpp"

int main(){

    auto tree = parseFile("../example.xml");
    std::cout << tree.content.attributes["what"] << std::endl;
    std::cout << tree.content.value << std::endl;
    return EXIT_SUCCESS;
}