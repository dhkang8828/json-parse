#include "json_parser.h"
#include <fstream>
#include <sstream>
#include <cctype>
#include <cstdio>
#include <algorithm>

std::string JsonValue::to_json(int indent) const
{
    std::string pad(static_cast<size_t>(indent) * 2, ' ');
    std::string inner(static_cast<size_t>(indent + 1) * 2, ' ');

    switch (type)
    {
    case Type::Null:   return "null";
    case Type::Bool:   return b ? "true" : "false";
    case Type::Number:
    {
        if (d == static_cast<double>(static_cast<long long>(d)))
            return std::to_string(static_cast<long long>(d));
        char buf[64];
        std::snprintf(buf, sizeof(buf), "%.10g", d);
        return buf;
    }
    case Type::String: return "\"" + s + "\"";
    case Type::Array:
    {
        if (arr.empty()) return "[]";
        std::string out = "[\n";
        for (size_t i = 0; i < arr.size(); ++i)
        {
            out += inner + arr[i].to_json(indent + 1);
            if (i + 1 < arr.size()) out += ",";
            out += "\n";
        }
        return out + pad + "]";
    }
    case Type::Object:
    {
        if (obj.empty()) return "{}";
        std::string out = "{\n";
        bool first = true;
        for (const auto& [key, val] : obj)
        {
            if (!first) out += ",\n";
            out += inner + "\"" + key + "\": " + val.to_json(indent + 1);
            first = false;
        }
        return out + "\n" + pad + "}";
    }
    }
    return "null";
}

static void flatten(const JsonValue& v,
                    const std::string& key,
                    std::unordered_map<std::string, std::string>& out)
{
    if (v.is_object())
    {
        for (const auto& [k, child] : v.as_object())
            flatten(child, key.empty() ? k : key + "." + k, out);
    }
    else if (v.is_array())
    {
        const auto& a = v.as_array();
        for (size_t i = 0; i < a.size(); ++i)
            flatten(a[i], key + "[" + std::to_string(i) + "]", out);
    }
    else
    {
        if      (v.is_null())   out[key] = "null";
        else if (v.is_bool())   out[key] = v.as_bool() ? "true" : "false";
        else if (v.is_number()) out[key] = v.to_json();
        else                    out[key] = v.as_string();
    }
}

std::unordered_map<std::string, std::string>
JsonValue::to_flat_map(const std::string& prefix) const
{
    std::unordered_map<std::string, std::string> result;
    flatten(*this, prefix, result);
    return result;
}

void JsonParser::skip_ws()
{
    while (pos < src.size() && std::isspace(static_cast<unsigned char>(src[pos])))
        ++pos;
}

char JsonParser::peek() const { return (pos < src.size()) ? src[pos] : '\0'; }

char JsonParser::consume()
{
    if (pos >= src.size()) throw std::runtime_error("Unexpected end of input");
    return src[pos++];
}

void JsonParser::expect(char c)
{
    skip_ws();
    char got = consume();
    if (got != c)
    {
        std::string msg = "Expected '";
        msg += c; msg += "' but got '"; msg += got; msg += "'";
        throw std::runtime_error(msg);
    }
}

JsonValue JsonParser::parse_value()
{
    skip_ws();
    char c = peek();
    if (c == '{') return parse_object();
    if (c == '[') return parse_array();
    if (c == '"') { JsonValue jv; jv.type = JsonValue::Type::String; jv.s = parse_string(); return jv; }
    if (c == 't' || c == 'f' || c == 'n') return parse_literal();
    if (c == '-' || std::isdigit(static_cast<unsigned char>(c))) return parse_number();
    std::string msg = "Unexpected character: '"; msg += c; msg += "'";
    throw std::runtime_error(msg);
}

JsonValue JsonParser::parse_object()
{
    expect('{');
    JsonValue jv;
    jv.type = JsonValue::Type::Object;
    skip_ws();
    if (peek() == '}') { consume(); return jv; }
    while (true)
    {
        skip_ws();
        std::string key = parse_string();
        expect(':');
        jv.obj.emplace(std::move(key), parse_value());
        skip_ws();
        char ch = peek();
        if (ch == '}') { consume(); break; }
        if (ch == ',') { consume(); continue; }
        throw std::runtime_error("Expected ',' or '}' in object");
    }
    return jv;
}

JsonValue JsonParser::parse_array()
{
    expect('[');
    JsonValue jv;
    jv.type = JsonValue::Type::Array;
    skip_ws();
    if (peek() == ']') { consume(); return jv; }
    while (true)
    {
        jv.arr.push_back(parse_value());
        skip_ws();
        char ch = peek();
        if (ch == ']') { consume(); break; }
        if (ch == ',') { consume(); continue; }
        throw std::runtime_error("Expected ',' or ']' in array");
    }
    return jv;
}

std::string JsonParser::parse_string()
{
    skip_ws();
    expect('"');
    std::string result;
    while (true)
    {
        if (pos >= src.size()) throw std::runtime_error("Unterminated string");
        char c = consume();
        if (c == '"') break;
        if (c == '\\')
        {
            char esc = consume();
            switch (esc)
            {
            case '"':  result += '"';  break;
            case '\\': result += '\\'; break;
            case '/':  result += '/';  break;
            case 'n':  result += '\n'; break;
            case 'r':  result += '\r'; break;
            case 't':  result += '\t'; break;
            case 'b':  result += '\b'; break;
            case 'f':  result += '\f'; break;
            default:   result += '\\'; result += esc;
            }
        }
        else { result += c; }
    }
    return result;
}

JsonValue JsonParser::parse_literal()
{
    JsonValue jv;
    if (src.compare(pos, 4, "true") == 0)  { pos += 4; jv.type = JsonValue::Type::Bool; jv.b = true; }
    else if (src.compare(pos, 5, "false") == 0) { pos += 5; jv.type = JsonValue::Type::Bool; jv.b = false; }
    else if (src.compare(pos, 4, "null") == 0)  { pos += 4; jv.type = JsonValue::Type::Null; }
    else throw std::runtime_error("Unknown literal at position " + std::to_string(pos));
    return jv;
}

JsonValue JsonParser::parse_number()
{
    size_t start = pos;
    if (peek() == '-') ++pos;
    while (pos < src.size() && std::isdigit(static_cast<unsigned char>(src[pos]))) ++pos;
    if (pos < src.size() && src[pos] == '.')
    {
        ++pos;
        while (pos < src.size() && std::isdigit(static_cast<unsigned char>(src[pos]))) ++pos;
    }
    if (pos < src.size() && (src[pos] == 'e' || src[pos] == 'E'))
    {
        ++pos;
        if (pos < src.size() && (src[pos] == '+' || src[pos] == '-')) ++pos;
        while (pos < src.size() && std::isdigit(static_cast<unsigned char>(src[pos]))) ++pos;
    }
    JsonValue jv;
    jv.type = JsonValue::Type::Number;
    jv.d    = std::stod(src.substr(start, pos - start));
    return jv;
}

JsonValue JsonParser::parse(const std::string& json)
{
    JsonParser p(json);
    JsonValue result = p.parse_value();
    p.skip_ws();
    if (p.pos != json.size())
        throw std::runtime_error("Trailing characters at position " + std::to_string(p.pos));
    return result;
}

JsonValue JsonParser::parse_file(const std::string& filepath)
{
    std::ifstream file(filepath);
    if (!file.is_open())
        throw std::runtime_error("Cannot open file: " + filepath);
    std::ostringstream oss;
    oss << file.rdbuf();
    return parse(oss.str());
}
