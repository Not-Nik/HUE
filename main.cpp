#include <map>
#include <string>
#include <vector>
#include <iostream>

bool Throw(const std::string & s)
{ throw std::runtime_error(s); }

#define assert(cond) (cond ? true : Throw("Assertion failed; line " + std::to_string(lex_pos.l) + " column " + std::to_string(lex_pos.c)))
#define match(s, r) (assert((r ? read() : t) == s) ? s : "")
#define assert_do(c, d) ((assert(c) ? d : ""))

/// Structure used for all expressions
/// This is the stuff the AST is made of
struct Expr
{
    bool nameless;
    std::string x;
    std::vector<Expr> exprs;

    bool operator==(const Expr & o) const
    { return nameless == o.nameless and x == o.x and exprs == o.exprs; }

    bool operator!=(const Expr & o) const
    { return !(this->operator==(o)); }
};

typedef std::function<std::string(Expr)> overruling;
/// Everything that causes the lexer to end a token
std::string stoppers = "() \n;";
/// (Lexer internal) current char
char c = ' ';
/// Current token
std::string t;
/// Generated functions (strings of c code)
std::vector<std::string> functions;
/// Overwrites for certain expressions
std::map<std::string, overruling> overrulings;
/// Total count of functions
int function_counter = 0;

struct {int l; int c;} lex_pos;

std::string return_type = "void";

// -------------------- Lexer --------------------

/// Read character from input file
char readChar()
{
    char buffer[1];
    std::cin.read(buffer, 1);
    lex_pos.c++;
    if (buffer[0] == '\n')
    {
        lex_pos.c = 0;
        lex_pos.l++;
    }
    return c = (std::cin.gcount() < 1 ? (char) -1 : buffer[0]);
}

/// Read token from input file
std::string read()
{
    std::string token;

    while (c == ' ' or c == '\t' or c == '\n' or c == ';')
    {
        if (c == ';')
            while (c != '\n') readChar();
        readChar();
    }

    if (c < 0) return "";
    else if (c == '"')
    {
        token += c;
        do
        {
            readChar();
            token += c;
        } while (c != '"');
        readChar();
    }
    else
        do
        {
            if (stoppers.find(c) == std::string::npos)
                token += c;
            else
            {
                if (token.empty())
                {
                    token = {c};
                    readChar();
                }
                break;
            }
        } while ((readChar()) >= 0);
    return t = token;
}

// -------------------- Parser --------------------

Expr read_instr(bool);

/// Read a value [primitives, strings or (do stuff)]
Expr read_val(bool r)
{
    if (r) read();
    if (t == "(") return read_instr(false);
    return Expr{true, t, {}};
}

/// Read an instruction [looks like this: (do stuff)]
Expr read_instr(bool r)
{
    match("(", r);
    Expr expr = {.nameless = false, .x = read()};

    while (true)
    {
        read();
        if (t != ")") expr.exprs.push_back(read_val(false));
        else break;
    }
    return expr;
}

// -------------------- Code Generator --------------------

/// Generate c code for an expression
std::string gen_expr(Expr expr)
{
    if (!expr.nameless)
    {
        if (overrulings.count(expr.x))
            return overrulings[expr.x](expr);
        std::string call_l = expr.x + "(";
        for (const auto & e : expr.exprs)
            call_l += gen_expr(e) + (e != expr.exprs.back() ? ", " : "");
        return call_l + ")";
    }
    else
        return expr.x;
}

#define type_overruling(type) overrulings.emplace(#type, [](Expr expr){ \
    return assert_do(expr.exprs.size() == 1, #type" " + expr.exprs[0].x); \
})

#define scope_overruling(name) overrulings.emplace(#name, [](Expr expr){ \
    std::string res = #name" (" + gen_expr(expr.exprs[0]) + "){"; \
    expr.exprs.erase(expr.exprs.begin()); \
    for (const auto & e : expr.exprs) \
        res += gen_expr(e) + ";"; \
    return res + "}"; \
})

#define operator_overruling(name, op) overrulings.emplace(#name, [](Expr expr) { \
    std::string res; \
    for (auto e : expr.exprs) \
        res += e.x + (e != expr.exprs.back() ? #op : ""); \
    return res; \
})

int main()
{
    overrulings.emplace("return", [](const Expr & expr)
    {
        return_type = assert_do(expr.exprs.size() == 2, expr.exprs[0].x);
        std::string res = "return " + gen_expr(expr.exprs[1]);
        return "return " + gen_expr(expr.exprs[1]);
    });

    overrulings.emplace("set", [](Expr expr)
    {
        return assert_do(expr.exprs.size() == 2, expr.exprs[0].x + "=" + gen_expr(expr.exprs[1]));
    });


    overrulings.emplace("include", [](Expr expr)
    {
        functions.push_back("#include " + expr.exprs[0].x + "\n");
        return assert_do(expr.exprs.size() == 1, "");
    });

    overrulings.emplace("do", [](const Expr & expr)
    {
        std::string function_id = std::to_string(++function_counter);
        std::string function = "f" + function_id + "()\n{";

        for (const auto & e : expr.exprs)
            function += gen_expr(e) + ";";
        functions.push_back(return_type + " " + function + "}\n");
        return_type = "void";

        return "f" + function_id + "()";
    });

    operator_overruling(add, +);
    operator_overruling(sub, -);
    operator_overruling(mul, *);
    operator_overruling(div, /);
    operator_overruling(mod, %);

    scope_overruling(if);
    scope_overruling(while);

    type_overruling(int);
    type_overruling(float);
    /// Open input file
    freopen("input.hue", "r", stdin);
    /// Read and parse
    Expr root = read_instr(true);
    /// Open output file
    freopen("out.c", "w", stdout);
    /// Write all the functions
    std::string e = gen_expr(root);
    std::string o;
    for (const auto & func : functions)
        o += func + "\n";
    /// Make sure everything is called and print the code
    std::cout << o + "int main(){" + e + ";}" << std::endl;
    return 0;
}
