#include <iostream>

#include "expression_parser.h"
int main(int argc, char** argv)
{
    if(argc != 2)
        return -1;

    try{
        std::cout << argv[1] << "\t";
        std::cout << expressionParser::solve(argv[1])
                  << std::endl;

    }catch( expressionParser::exception exception ){
        std::cout << "ERROR : " << exception.what()
                  << "\nat location " << exception.location
                  << std::endl;

        return -1;
    }

    return 0;
}
