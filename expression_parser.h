#ifndef _EXPRESSION_PARSER_INCLUDED_
#define _EXPRESSION_PARSER_INCLUDED_

#ifndef __cplusplus
    #error please use a valid c++ compiler
#elif __cplusplus < 201703L
    #error a c++17 compliant compiler is required for this project
#elif __cplusplus < 202002L
    #define compile_time_function constexpr
#else
    #define compile_time_function consteval
#endif

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

            bool isRTLBinary()  const {return arity == binary && order == RTL;}
            bool isLTRBinary()  const {return arity == binary && order == LTR;}

            //left unary has RTL order and right unary has LTR order
            bool isLeftUnary()  const {return arity == unary  && order == RTL;}
            bool isRightUnary() const {return arity == unary  && order == LTR;}
        };

        constexpr int precedenceFunc = 256;
        constexpr int precedenceConst = -1;

        //CTAD is supported by compilers supporting c++17
        constexpr static std::array operators = {
            //by design, unary operators are to operate on 2nd argument
            _operator({"+", precedenceFunc, _operator::unary, _operator::RTL,
                      [](double a, double b) { return b; }}),

            _operator({"-", precedenceFunc, _operator::unary, _operator::RTL,
                      [](double a, double b) { return -b; }}),

            _operator({"+", 2, _operator::binary, _operator::LTR,
                      [](double a, double b) { return a + b; }}),

            _operator({"-", 2, _operator::binary, _operator::LTR,
                      [](double a, double b) { return a - b; }}),

            _operator({"*", 3, _operator::binary, _operator::LTR,
                      [](double a, double b) { return a * b; }}),

            _operator({"/", 3, _operator::binary, _operator::LTR,
                      [](double a, double b) {
                          if(b == 0.0)
                              throw exception("divide by zero encountered");

                          return a / b;
                      }}),

            _operator({"^", 4, _operator::binary, _operator::RTL,
                      [](double a, double b) { return std::pow(a, b); }}),

            _operator({"e", 0, _operator::binary, _operator::LTR,
                      [](double a, double b) { return a * std::pow(10,b); }}),

            _operator({"mod", 1, _operator::binary, _operator::LTR,
                      [](double a, double b) { return std::fmod(a,b); }}),

            _operator({"!", precedenceFunc, _operator::unary, _operator::LTR,
                      [](double a, double b) {
                          double ret = 1.0;
                          int i = b;
                          while(i > 1){
                              ret *= i;
                              i -= 1;
                          }

                          return ret;
                      }}),

            _operator({"%", precedenceFunc, _operator::unary, _operator::LTR,
                      [](double a, double b) { return b / 100.0; }}),

            _operator({"sin", precedenceFunc, _operator::unary, _operator::RTL,
                      [](double a, double b) { return std::sin(b); }}),

            _operator({"sinh",precedenceFunc, _operator::unary, _operator::RTL,
                      [](double a, double b) { return std::sinh(b); }}),

            _operator({"arcsin",precedenceFunc,_operator::unary,_operator::RTL,
                      [](double a, double b) { return std::asin(b); }}),

            _operator({"cos", precedenceFunc, _operator::unary, _operator::RTL,
                      [](double a, double b) { return std::cos(b); }}),

            _operator({"cosh",precedenceFunc, _operator::unary, _operator::RTL,
                      [](double a, double b) { return std::cosh(b); }}),

            _operator({"arccos",precedenceFunc,_operator::unary,_operator::RTL,
                      [](double a, double b) { return std::acos(b); }}),

            _operator({"tan", precedenceFunc, _operator::unary, _operator::RTL,
                      [](double a, double b) { return std::tan(b); }}),

            _operator({"tanh",precedenceFunc, _operator::unary, _operator::RTL,
                      [](double a, double b) { return std::tanh(b); }}),

            _operator({"arctan",precedenceFunc,_operator::unary,_operator::RTL,
                      [](double a, double b) { return std::atan(b); }}),

            _operator({"(", -1, _operator::unary, _operator::RTL,
                      [](double a, double b) { return b; }}),

            _operator({")", -1, _operator::unary, _operator::LTR,
                      [](double a, double b) { return b; }}),

            //constants are to be placed below this
            _operator({"pi", precedenceConst, _operator::unary, _operator::RTL,
                      [](double a, double b) { return 3.14159; }}),

            _operator({"phi",precedenceConst, _operator::unary, _operator::RTL,
                      [](double a, double b) { return 1.61803; }}),

            _operator({"euler",precedenceConst,_operator::unary,_operator::RTL,
                      [](double a, double b) { return 2.71828; }}),
        };

        compile_time_function static int findOperator(std::string_view symbol)
        {
            for(int i = 0; i < operators.size(); i++)
                if(operators[i].symbol == symbol)
                    return i;

            return -1;
        }

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
            bool isConstant() const {return type == _operator
                                         && operatorIndex > findOperator(")");}
        };

        void tokenise(std::string_view, std::vector<token>&);
    };

    double solve(std::string_view);
};

#endif

