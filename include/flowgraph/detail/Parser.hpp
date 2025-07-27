#pragma once

#include "AST.hpp"
#include <string>
#include <istream>
#include <memory>

namespace FlowGraph {

/**
 * @brief Token types for lexical analysis
 */
enum class TokenType {
    // Literals
    Identifier,
    String,
    Number,
    Boolean,
    
    // Keywords
    Title,
    Params,
    Returns,
    Nodes,
    Flow,
    Start,
    End,
    
    // Node types
    Proc,
    Assign,
    Cond,
    
    // Operators and symbols
    Arrow,          // ->
    InputBinding,   // >>
    OutputBinding,  // <<
    Dot,           // .
    Colon,         // :
    Question,      // ?
    
    // Comments
    LineComment,   // //
    BlockComment,  // /* */
    
    // Special
    Newline,
    EOF_Token,
    Invalid
};

/**
 * @brief Single token
 */
struct Token {
    TokenType type;
    std::string text;
    Location location;
    
    Token(TokenType t, const std::string& txt, Location loc = {})
        : type(t), text(txt), location(loc) {}
};

/**
 * @brief Lexical analyzer for FlowGraph files
 */
class Lexer {
public:
    explicit Lexer(const std::string& content, const std::string& filename = "");
    
    Token nextToken();
    Token peekToken();
    bool hasMoreTokens() const;
    
    Location getCurrentLocation() const;
    
private:
    std::string content_;
    std::string filename_;
    size_t position_ = 0;
    size_t line_ = 1;
    size_t column_ = 1;
    
    char currentChar() const;
    char peekChar(size_t offset = 1) const;
    void advance();
    void skipWhitespace();
    Token readString();
    Token readNumber();
    Token readIdentifier();
    Token readLineComment();
    Token readBlockComment();
    bool isAtEnd() const;
};

/**
 * @brief Parser for FlowGraph files
 */
class Parser {
public:
    Parser() = default;
    
    /**
     * @brief Parse FlowGraph from string content
     */
    std::unique_ptr<FlowAST> parse(const std::string& content, const std::string& filename = "");
    
    /**
     * @brief Parse FlowGraph from file
     */
    std::unique_ptr<FlowAST> parseFile(const std::string& filepath);
    
private:
    Lexer lexer_;
    Token currentToken_{TokenType::Invalid, ""};
    
    void advance();
    bool match(TokenType type);
    bool check(TokenType type) const;
    Token consume(TokenType type, const std::string& errorMessage);
    
    // Parsing methods
    std::unique_ptr<FlowAST> parseFlow();
    void parseTitle(FlowAST& ast);
    void parseParams(FlowAST& ast);
    void parseReturns(FlowAST& ast);
    void parseNodes(FlowAST& ast);
    void parseFlow(FlowAST& ast);
    
    std::unique_ptr<FlowNode> parseNode();
    std::unique_ptr<AssignNode> parseAssignNode(const std::string& id);
    std::unique_ptr<CondNode> parseCondNode(const std::string& id);
    std::unique_ptr<ProcNode> parseProcNode(const std::string& id);
    
    FlowConnection parseConnection();
    Parameter parseParameter();
    TypeInfo parseType();
    
    // Helper methods
    void skipComments();
    std::string collectComment();
    
    // Error handling
    void error(const std::string& message);
    void warning(const std::string& message);
};

} // namespace FlowGraph