#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include "json_parser.h"

static void print_separator(char c = '-', int width = 60)
{
    std::cout << std::string(width, c) << "\n";
}

static void print_flat_map(const std::unordered_map<std::string, std::string>& m)
{
    std::vector<std::string> keys;
    keys.reserve(m.size());
    for (const auto& [k, _] : m) keys.push_back(k);
    std::sort(keys.begin(), keys.end());
    for (const auto& k : keys)
        std::cout << "  " << std::left << std::setw(40) << k << " => " << m.at(k) << "\n";
}

int main()
{
    // --- cars.json ---
    JsonValue root;
    try { root = JsonParser::parse_file("cars.json"); }
    catch (const std::exception& e) { std::cerr << "[Error] " << e.what() << "\n"; return 1; }

    print_separator('=');
    std::cout << " cars.json  =>  to_json()\n";
    print_separator('=');
    std::cout << root.to_json() << "\n\n";

    print_separator('=');
    std::cout << " cars.json  =>  to_flat_map()\n";
    print_separator('=');
    print_flat_map(root.to_flat_map());
    std::cout << "\n";

    print_separator('=');
    std::cout << " cars.json  =>  Direct field access\n";
    print_separator('=');

    if (root.is_object())
    {
        auto& rootObj = root.as_object();
        if (auto it = rootObj.find("dealership"); it != rootObj.end())
        {
            auto& d = it->second.as_object();
            std::cout << "Dealership : " << d.at("name").as_string() << "\n";
            std::cout << "City       : " << d.at("city").as_string() << "\n";
            std::cout << "Open Hours : " << d.at("openHours").as_string() << "\n\n";
        }
        if (auto it = rootObj.find("inventory"); it != rootObj.end() && it->second.is_array())
        {
            print_separator('-');
            std::cout << " Car Inventory\n";
            print_separator('-');
            for (const auto& car : it->second.as_array())
            {
                auto& c = car.as_object();
                std::cout << "[" << c.at("id").as_string() << "] "
                          << c.at("brand").as_string() << " "
                          << c.at("model").as_string()
                          << " (" << static_cast<int>(c.at("year").as_number()) << ")\n";
                std::cout << "  Price    : $" << std::fixed << std::setprecision(2) << c.at("price").as_number() << "\n";
                std::cout << "  In Stock : " << (c.at("inStock").as_bool() ? "Yes" : "No") << "\n";
                const auto& colorVal = c.at("color");
                std::cout << "  Color    : " << (colorVal.is_null() ? "TBD" : colorVal.as_string()) << "\n";
                if (auto eit = c.find("engine"); eit != c.end())
                {
                    auto& eng = eit->second.as_object();
                    std::cout << "  Engine   : " << eng.at("type").as_string()
                              << "  " << eng.at("displacement").as_number() << "L"
                              << "  " << static_cast<int>(eng.at("horsepower").as_number()) << "hp"
                              << (eng.at("turbocharged").as_bool() ? "  [Turbo]" : "") << "\n";
                }
                if (auto fit = c.find("features"); fit != c.end() && fit->second.is_array())
                {
                    std::cout << "  Features : ";
                    bool first = true;
                    for (const auto& f : fit->second.as_array())
                    { if (!first) std::cout << ", "; std::cout << f.as_string(); first = false; }
                    std::cout << "\n";
                }
                std::cout << "\n";
            }
        }
    }

    // --- Inline string ---
    print_separator('=');
    std::cout << " Inline string parse\n";
    print_separator('=');
    const std::string inlineJson = R"({"task":"CarAssemble","parts":["engine","body","wheel","interior"],"ready":true,"progress":0.75})";
    auto v = JsonParser::parse(inlineJson);
    std::cout << "task     : " << v.as_object().at("task").as_string() << "\n";
    std::cout << "ready    : " << (v.as_object().at("ready").as_bool() ? "true" : "false") << "\n";
    std::cout << "progress : " << v.as_object().at("progress").as_number() * 100.0 << "%\n";
    std::cout << "parts    : ";
    for (const auto& p : v.as_object().at("parts").as_array()) std::cout << p.as_string() << " ";
    std::cout << "\n\nFlat map:\n";
    print_flat_map(v.to_flat_map());

    // --- sample.json ---
    JsonValue sample;
    try { sample = JsonParser::parse_file("sample.json"); }
    catch (const std::exception& e) { std::cerr << "[Error] sample.json: " << e.what() << "\n"; return 1; }

    std::cout << "\n";
    print_separator('=');
    std::cout << " sample.json  =>  to_json()\n";
    print_separator('=');
    std::cout << sample.to_json() << "\n\n";

    print_separator('=');
    std::cout << " sample.json  =>  to_flat_map()\n";
    print_separator('=');
    print_flat_map(sample.to_flat_map());
    std::cout << "\n";

    print_separator('=');
    std::cout << " sample.json  =>  Direct field access\n";
    print_separator('=');
    if (sample.is_object())
    {
        auto& o = sample.as_object();
        auto pstr  = [&](const std::string& k) { if (auto it = o.find(k); it != o.end()) std::cout << std::left << std::setw(14) << k << ": " << it->second.as_string() << "\n"; };
        auto pnum  = [&](const std::string& k) { if (auto it = o.find(k); it != o.end()) std::cout << std::left << std::setw(14) << k << ": " << it->second.as_number() << "\n"; };
        auto pbool = [&](const std::string& k) { if (auto it = o.find(k); it != o.end()) std::cout << std::left << std::setw(14) << k << ": " << (it->second.as_bool() ? "true" : "false") << "\n"; };
        auto pnull = [&](const std::string& k) { if (auto it = o.find(k); it != o.end()) std::cout << std::left << std::setw(14) << k << ": " << (it->second.is_null() ? "(null)" : it->second.as_string()) << "\n"; };
        pstr("brand"); pstr("model"); pnum("year"); pnum("price"); pbool("inStock"); pnull("color");
        if (auto it = o.find("tags"); it != o.end() && it->second.is_array())
        {
            std::cout << std::left << std::setw(14) << "tags" << ": ";
            for (const auto& t : it->second.as_array()) std::cout << t.as_string() << "  ";
            std::cout << "\n";
        }
        if (auto it = o.find("engine"); it != o.end() && it->second.is_object())
        {
            std::cout << "engine:\n";
            for (const auto& [k, val] : it->second.as_object())
                std::cout << "  " << std::left << std::setw(16) << k << ": " << val.to_json() << "\n";
        }
        if (auto it = o.find("options"); it != o.end() && it->second.is_object())
        {
            std::cout << "options:\n";
            for (const auto& [k, val] : it->second.as_object())
                std::cout << "  " << std::left << std::setw(16) << k << ": " << (val.as_bool() ? "true" : "false") << "\n";
        }
    }
    return 0;
}
