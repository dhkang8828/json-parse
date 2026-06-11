# json-parse

C++20 JSON parser — no external libraries required.

## Features

- Parse JSON from file or inline string
- Supports all JSON types: `null`, `bool`, `number`, `string`, `array`, `object`
- Serialize back to `std::string` via `to_json()`
- Flatten to `std::unordered_map<string, string>` via `to_flat_map()`
  - Nested keys: `engine.type`
  - Array items: `tags[0]`

## Quick Start

```cpp
// Parse from file
JsonValue root = JsonParser::parse_file("cars.json");

// Serialize to string
std::string json_str = root.to_json();

// Flatten to map
auto flat = root.to_flat_map();
// flat["inventory[0].engine.type"] == "GDI"
// flat["dealership.name"]          == "Seoul Motors"

// Direct field access
auto& inv = root.as_object().at("inventory").as_array();
std::string brand = inv[0].as_object().at("brand").as_string(); // "Hyundai"
```

## Build

Open `json_project/json_project.vcxproj` with Visual Studio 2022 and build (C++20, x64).

## Files

| File | Description |
|------|-------------|
| `json_parser.h` | `JsonValue` struct + `JsonParser` class declaration |
| `json_parser.cpp` | Recursive-descent parser implementation |
| `main.cpp` | Demo: parses `cars.json` and `sample.json` |
| `cars.json` | Sample car inventory data |
| `sample.json` | Sample single-car data |
