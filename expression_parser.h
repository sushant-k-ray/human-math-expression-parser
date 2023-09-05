#ifndef _EXPRESSION_PARSER_INCLUDED_
#define _EXPRESSION_PARSER_INCLUDED_

#include <array>
#include <vector>
#include <string_view>
#include <stdexcept>
#include <cmath>

namespace expressionParser{
    class exception : public std::runtime_error
    {
    public:
        int location;
        exception(const char* string, int location = -1)
            : std::runtime_error(string), location(location) {}
    };

    namespace _Impl__{
        struct _operator{
            std::string_view symbol;
            int precedence;
            enum {unary, binary} arity;
            enum {LTR, RTL} order;

            //std::function isn't constexpr compliant
            double (*execute)(double, double);

            bool isRTLBinary()  const { return arity == binary && order == RTL; }
            bool isLTRBinary()  const { return arity == binary && order == LTR; }

            //left unary has RTL order and right unary has LTR order
            bool isLeftUnary()  const { return arity == unary  && order == RTL; }
            bool isRightUnary() const { return arity == unary  && order == LTR; }
        };

        //CTAD is supported by compilers supporting c++17
        constexpr static std::array operators = {
            //by design, unary operators are to operate on 2nd argument
            _operator({"+", 256, _operator::unary, _operator::RTL,
                      [](double a, double b) { return b; }}),

            _operator({"-", 256, _operator::unary, _operator::RTL,
                      [](double a, double b) { return -b; }}),

            _operator({"+", 1, _operator::binary, _operator::LTR,
                      [](double a, double b) { return a + b; }}),

            _operator({"-", 1, _operator::binary, _operator::LTR,
                      [](double a, double b) { return a - b; }}),

            _operator({"*", 2, _operator::binary, _operator::LTR,
                      [](double a, double b) { return a * b; }}),

            _operator({"/", 2, _operator::binary, _operator::LTR,
                      [](double a, double b) {
                          if(b == 0.0)
                              throw exception("divide by zero encountered");

                          return a / b;
                      }}),

            _operator({"^", 3, _operator::binary, _operator::RTL,
                      [](double a, double b) { return std::pow(a, b); }}),

            _operator({"!", 256, _operator::unary, _operator::LTR,
                      [](double a, double b) {
                          double ret = 1.0;
                          int i = b;
                          while(i > 1){
                              ret *= i;
                              i -= 1;
                          }

                          return ret;
                      }}),

            _operator({"sin", 256, _operator::unary, _operator::RTL,
                      [](double a, double b) { return std::sin(b); }}),

            _operator({"cos", 256, _operator::unary, _operator::RTL,
                      [](double a, double b) { return std::cos(b); }}),

            _operator({"tan", 256, _operator::unary, _operator::RTL,
                      [](double a, double b) { return std::tan(b); }}),

            _operator({"(", 0, _operator::unary, _operator::RTL,
                      [](double a, double b) { return b; }}),

            _operator({")", 0, _operator::unary, _operator::LTR,
                      [](double a, double b) { return b; }}),
        };

        struct token{
            enum { decimal, _operator } type;
            union{ struct { int operatorIndex; int startLocation; };
                   double number; };

            token(double number)
                : type(decimal), number(number) {}

            token(int operatorIndex, int startLocation)
                : type(_operator),
                  operatorIndex(operatorIndex),
                  startLocation(startLocation) {}

            const auto& getOperator() { return operators[operatorIndex]; }
        };

        consteval static int findOperator(std::string_view symbol)
        {
            for(int i = 0; i < operators.size(); i++)
                if(operators[i].symbol == symbol)
                    return i;

            return -1;
        }

        void tokenise(std::string_view, std::vector<token>&);
    };

    double solve(std::string_view);
};

#endif

