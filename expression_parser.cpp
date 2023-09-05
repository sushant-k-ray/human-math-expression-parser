#include "expression_parser.h"
#include <tuple>
#include <ctype.h>

using namespace expressionParser;
using namespace expressionParser::_Impl__;

void _Impl__::tokenise(std::string_view string, std::vector<token>& tokens)
{
    auto parseNumber = [&](int beg, int end){
        while(beg < end && isspace(string[beg]))
            beg++;

        while(end > 0 && isspace(string[end - 1]))
            end--;

        if( beg >= end )
            return;

        int shiftDecimals = 0;
        double number;

        for(; beg < end; beg += 1){
            auto ch = string[beg];

            if(isspace(ch))
                continue;

            switch(ch)
            {
                case '0' ... '9' :
                    if( shiftDecimals )
                        number += (ch - '0') * std::pow(10.0, shiftDecimals--);
                    else
                        number = number * 10 + ch - '0';

                    break;

                case '.':
                    if( shiftDecimals )
                        throw exception("multiple dots encountered", beg);

                    shiftDecimals = -1;
                    break;

                default:
                    throw exception("unknown character", beg);
            }
        }

        tokens.push_back(token(number));
    };

    //tuple of operatorIndex, startLocation, and endLocation
    std::vector<std::tuple<int, int, int>> posOperators;

    //add operators to posOperators
    //ignore unary + and unary -
    for(int i = 2; i < operators.size(); i++)
    {
        auto& _operator = operators[i];
        auto size = _operator.symbol.size();
        int index = 0;

        while((index = string.find(_operator.symbol, index)) != -1){
            posOperators.push_back(std::make_tuple(i, index, index + size));
            index += size;
        }
    }

    //remove ambiguities
    std::sort(posOperators.begin(), posOperators.end(), [](auto i, auto j){
        if(std::get<1>(i) == std::get<1>(j))
            return std::get<2>(i) < std::get<2>(j);

        return std::get<1>(i) < std::get<1>(j);
    });

    for(int i = posOperators.size() - 1; i > 0; i--)
    {
        auto& a = posOperators[i];
        auto& b = posOperators[i - 1];

        if(std::get<1>(a) == std::get<1>(b))
            posOperators.erase(posOperators.begin() + (i - 1));

        else if(std::get<1>(a) < std::get<2>(b))
            throw exception("ambigous operator", std::get<1>(a));
    }

    //tokenize the input
    if(posOperators.size() == 0)
    {
        parseNumber(0, string.size());

        if(tokens.size() == 0)
            tokens.push_back(token(0.0));

        return;
    }

    for(int i = 0, curPos = 0; i < posOperators.size(); i++){
        auto& a = posOperators[i];
        parseNumber(curPos, std::get<1>(a));

        tokens.push_back(token(std::get<0>(a), std::get<1>(a)));

        curPos = std::get<2>(a);
    }

    parseNumber(std::get<2>(posOperators.back()), string.size());
}

double expressionParser::solve(std::string_view string)
{
    std::vector<token> tokens;
    tokenise(string, tokens);

    if(tokens.size() == 0)
        return 0.0;

    std::vector<double> numberStack;
    std::vector<int> operatorStack;

    auto execute = [&](int operatorNumber){
        auto& i = operators[operatorNumber];

        double num_1 = 0.0;
        double num_2 = numberStack.back();

        if(i.arity == _operator::binary){
            numberStack.pop_back();
            num_1 = numberStack.back();
        }

        numberStack.back() = i.execute(num_1, num_2);
    };

    auto addOperator = [&](int operatorNumber){
        auto& i = operators[operatorNumber];

        while(operatorStack.size()){
            auto& j = operators[operatorStack.back()];

            if ( j.precedence > i.precedence
            || ( j.precedence == i.precedence
            &&   i.order == _operator::LTR )){
                execute(operatorStack.back());
                operatorStack.pop_back();
            }else
                break;
        }

        operatorStack.push_back(operatorNumber);
    };

    int parenthesisDepth = 0;

    if(tokens.front().type == token::_operator)
    {
        auto& front = tokens.front();

        if(front.operatorIndex == 2 || front.operatorIndex == 3)
            front.operatorIndex -= 2;

        auto& frontOperator = front.getOperator();

        if(frontOperator.arity == _operator::binary)
            throw exception( "binary operator at start of the string"
                           , front.startLocation );

        if(frontOperator.isRightUnary())
            throw exception( "operator at start without prior argument"
                           , front.startLocation );

        if(front.operatorIndex == findOperator("("))
            parenthesisDepth = 1;

        addOperator(front.operatorIndex);
    }else
        numberStack.push_back(tokens.front().number);

    for(int i = 1; i < tokens.size(); i++)
    {
        auto& a = tokens[i - 1];
        auto& b = tokens[i];

        if(b.type == token::decimal){
            if(a.getOperator().isRightUnary())
                addOperator(findOperator("*"));

            numberStack.push_back(b.number);
            continue;
        }

        if(a.type == token::_operator)
        {
            if(a.getOperator().isRightUnary()){
                if(b.getOperator().isLeftUnary())
                    addOperator(findOperator("*"));
            }

            else if(( b.operatorIndex == 2 || b.operatorIndex == 3)
                   && a.getOperator().arity == _operator::binary )
                      b.operatorIndex -= 2;

            else if(b.getOperator().isLeftUnary() == false)
                throw exception( "multiple consecutive operators"
                               , b.startLocation );
        }

        else if(b.getOperator().isLeftUnary())
            addOperator(findOperator("*"));

        parenthesisDepth += b.operatorIndex == findOperator("(");

        if(b.operatorIndex == findOperator(")")){
            parenthesisDepth -= 1;

            if(parenthesisDepth < 0)
                throw exception( "parenthesis close without previous open"
                               , b.startLocation );

            while(operatorStack.back() != findOperator("(")){
                execute(operatorStack.back());
                operatorStack.pop_back();
            }

            //remove parenthesis open from stack
            operatorStack.pop_back();
        }else
            addOperator(b.operatorIndex);
    }

    while(operatorStack.size()){
        execute(operatorStack.back());
        operatorStack.pop_back();
    }

    if(numberStack.size() != 1)
        throw exception("expression error");

    return numberStack.front();
}
