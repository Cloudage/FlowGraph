#pragma once

#include "AST.hpp"
#include <string>
#include <istream>
#include <memory>
#include <cctype>
#include <algorithm>

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
    Errors,
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
    Lexer() = default;
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
    void parseErrors(FlowAST& ast);
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

// Minimal parser implementation (header-only)

inline std::unique_ptr<FlowAST> Parser::parse(const std::string& content, const std::string& filename) {
    lexer_ = Lexer(content, filename);
    advance();
    return parseFlow();
}

inline std::unique_ptr<FlowAST> Parser::parseFile(const std::string& /*filepath*/) {
    // For now, just return empty AST for file parsing
    // Full implementation would read file and call parse()
    return std::make_unique<FlowAST>();
}

inline std::unique_ptr<FlowAST> Parser::parseFlow() {
    auto ast = std::make_unique<FlowAST>();
    
    while (currentToken_.type != TokenType::EOF_Token) {
        switch (currentToken_.type) {
            case TokenType::Title:
                parseTitle(*ast);
                break;
            case TokenType::Params:
                parseParams(*ast);
                break;
            case TokenType::Returns:
                parseReturns(*ast);
                break;
            case TokenType::Errors:
                parseErrors(*ast);
                break;
            case TokenType::Nodes:
                parseNodes(*ast);
                break;
            case TokenType::Flow:
                parseFlow(*ast);
                break;
            default:
                advance(); // Skip unknown tokens
                break;
        }
    }
    
    return ast;
}

inline void Parser::parseTitle(FlowAST& ast) {
    advance(); // consume TITLE
    if (currentToken_.type == TokenType::Colon) {
        advance(); // consume :
        
        // Read the rest of the line as title
        std::string title;
        while (currentToken_.type != TokenType::Newline && 
               currentToken_.type != TokenType::EOF_Token) {
            if (!title.empty()) {
                title += " ";
            }
            title += currentToken_.text;
            advance();
        }
        ast.title = title;
    }
}

inline void Parser::parseParams(FlowAST& /*ast*/) {
    advance(); // consume PARAMS
    if (currentToken_.type == TokenType::Colon) {
        advance(); // consume :
        // For now, skip parameters parsing
        while (currentToken_.type != TokenType::Returns && 
               currentToken_.type != TokenType::Errors &&
               currentToken_.type != TokenType::Nodes && 
               currentToken_.type != TokenType::Flow &&
               currentToken_.type != TokenType::EOF_Token) {
            advance();
        }
    }
}

inline void Parser::parseReturns(FlowAST& /*ast*/) {
    advance(); // consume RETURNS
    if (currentToken_.type == TokenType::Colon) {
        advance(); // consume :
        // For now, skip returns parsing
        while (currentToken_.type != TokenType::Errors &&
               currentToken_.type != TokenType::Nodes && 
               currentToken_.type != TokenType::Flow &&
               currentToken_.type != TokenType::EOF_Token) {
            advance();
        }
    }
}

inline void Parser::parseErrors(FlowAST& ast) {
    advance(); // consume ERRORS
    if (currentToken_.type == TokenType::Colon) {
        advance(); // consume :
        
        // Parse error names
        while (currentToken_.type == TokenType::Identifier || currentToken_.type == TokenType::Newline) {
            if (currentToken_.type == TokenType::Identifier) {
                ast.errors.emplace_back(currentToken_.text);
                advance();
            } else {
                advance(); // skip newlines
            }
        }
    }
}

inline void Parser::parseNodes(FlowAST& /*ast*/) {
    advance(); // consume NODES
    if (currentToken_.type == TokenType::Colon) {
        advance(); // consume :
        // For now, skip nodes parsing
        while (currentToken_.type != TokenType::Flow &&
               currentToken_.type != TokenType::EOF_Token) {
            advance();
        }
    }
}

inline void Parser::parseFlow(FlowAST& /*ast*/) {
    advance(); // consume FLOW
    if (currentToken_.type == TokenType::Colon) {
        advance(); // consume :
        // For now, skip flow parsing
        while (currentToken_.type != TokenType::EOF_Token) {
            advance();
        }
    }
}

inline void Parser::advance() {
    currentToken_ = lexer_.nextToken();
}

inline bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

inline bool Parser::check(TokenType type) const {
    return currentToken_.type == type;
}

inline Token Parser::consume(TokenType type, const std::string& errorMessage) {
    if (check(type)) {
        Token token = currentToken_;
        advance();
        return token;
    }
    error(errorMessage);
    return currentToken_;
}

inline void Parser::error(const std::string& message) {
    throw FlowGraphError(FlowGraphError::Type::Parse, message, currentToken_.location);
}

inline void Parser::warning(const std::string& /*message*/) {
    // For now, just ignore warnings
}

inline void Parser::skipComments() {
    // Implementation for skipping comments
}

inline std::string Parser::collectComment() {
    // Implementation for collecting comments
    return "";
}

// Minimal Lexer implementation
inline Lexer::Lexer(const std::string& content, const std::string& filename) 
    : content_(content), filename_(filename) {}

inline Token Lexer::nextToken() {
    if (content_.empty()) {
        return Token(TokenType::EOF_Token, "");
    }
    
    skipWhitespace();
    
    if (isAtEnd()) {
        return Token(TokenType::EOF_Token, "");
    }
    
    char c = currentChar();
    
    // Handle specific patterns
    if (c == '\n') {
        advance();
        return Token(TokenType::Newline, "\n");
    }
    
    if (c == ':') {
        advance();
        return Token(TokenType::Colon, ":");
    }
    
    if (c == '"') {
        return readString();
    }
    
    if (std::isdigit(c)) {
        return readNumber();
    }
    
    if (std::isalpha(c) || c == '_') {
        return readIdentifier();
    }
    
    // Skip unknown characters
    advance();
    return nextToken();
}

inline Token Lexer::peekToken() {
    size_t savePos = position_;
    size_t saveLine = line_;
    size_t saveCol = column_;
    
    Token token = nextToken();
    
    position_ = savePos;
    line_ = saveLine;
    column_ = saveCol;
    
    return token;
}

inline bool Lexer::hasMoreTokens() const {
    return !isAtEnd();
}

inline Location Lexer::getCurrentLocation() const {
    return Location(filename_, line_, column_);
}

inline char Lexer::currentChar() const {
    if (isAtEnd()) return '\0';
    return content_[position_];
}

inline char Lexer::peekChar(size_t offset) const {
    if (position_ + offset >= content_.length()) return '\0';
    return content_[position_ + offset];
}

inline void Lexer::advance() {
    if (!isAtEnd()) {
        if (content_[position_] == '\n') {
            line_++;
            column_ = 1;
        } else {
            column_++;
        }
        position_++;
    }
}

inline void Lexer::skipWhitespace() {
    while (!isAtEnd() && std::isspace(currentChar()) && currentChar() != '\n') {
        advance();
    }
}

inline Token Lexer::readString() {
    advance(); // skip opening quote
    std::string value;
    
    while (!isAtEnd() && currentChar() != '"') {
        value += currentChar();
        advance();
    }
    
    if (!isAtEnd()) {
        advance(); // skip closing quote
    }
    
    return Token(TokenType::String, value);
}

inline Token Lexer::readNumber() {
    std::string value;
    
    while (!isAtEnd() && (std::isdigit(currentChar()) || currentChar() == '.')) {
        value += currentChar();
        advance();
    }
    
    return Token(TokenType::Number, value);
}

inline Token Lexer::readIdentifier() {
    std::string value;
    
    while (!isAtEnd() && (std::isalnum(currentChar()) || currentChar() == '_')) {
        value += currentChar();
        advance();
    }
    
    // Check for keywords
    if (value == "TITLE") return Token(TokenType::Title, value);
    if (value == "PARAMS") return Token(TokenType::Params, value);
    if (value == "RETURNS") return Token(TokenType::Returns, value);
    if (value == "ERRORS") return Token(TokenType::Errors, value);
    if (value == "NODES") return Token(TokenType::Nodes, value);
    if (value == "FLOW") return Token(TokenType::Flow, value);
    if (value == "START") return Token(TokenType::Start, value);
    if (value == "END") return Token(TokenType::End, value);
    if (value == "PROC") return Token(TokenType::Proc, value);
    if (value == "ASSIGN") return Token(TokenType::Assign, value);
    if (value == "COND") return Token(TokenType::Cond, value);
    if (value == "true" || value == "false") return Token(TokenType::Boolean, value);
    
    return Token(TokenType::Identifier, value);
}

inline Token Lexer::readLineComment() {
    return Token(TokenType::LineComment, "");
}

inline Token Lexer::readBlockComment() {
    return Token(TokenType::BlockComment, "");
}

inline bool Lexer::isAtEnd() const {
    return position_ >= content_.length();
}

} // namespace FlowGraph