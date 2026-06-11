#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>

struct JsonValue
{
    enum class Type { Null, Bool, Number, String, Array, Object };

    Type   type = Type::Null;
    bool   b    = false;
    double d    = 0.0;
    std::string s;
    std::vector<JsonValue>                      arr;
    std::unordered_map<std::string, JsonValue>  obj;

    bool is_null()   const { return type == Type::Null;   }
    bool is_bool()   const { return type == Type::Bool;   }
    bool is_number() const { return type == Type::Number; }
    bool is_string() const { return type == Type::String; }
    bool is_array()  const { return type == Type::Array;  }
    bool is_object() const { return type == Type::Object; }

    bool          as_bool()   const { return b; }
    double        as_number() const { return d; }
    const std::string& as_string() const { return s; }
    const std::vector<JsonValue>& as_array()  const { return arr; }
    const std::unordered_map<std::string, JsonValue>& as_object() const { return obj; }

    std::vector<JsonValue>& as_array()  { return arr; }
    std::unordered_map<std::string, JsonValue>& as_object() { return obj; }

    std::string to_json(int indent = 0) const;
    std::unordered_map<std::string, std::string> to_flat_map(const std::string& prefix = "") const;
};

class JsonParser
{
public:
    static JsonValue parse(const std::string& json);
    static JsonValue parse_file(const std::string& filepath);

private:
    const std::string& src;
    size_t             pos;

    explicit JsonParser(const std::string& s) : src(s), pos(0) {}

    JsonValue   parse_value();
    JsonValue   parse_object();
    JsonValue   parse_array();
    std::string parse_string();
    JsonValue   parse_literal();
    JsonValue   parse_number();

    void skip_ws();
    char peek()    const;
    char consume();
    void expect(char c);
};
