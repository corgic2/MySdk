#pragma once
#include <iomanip>
#include <iostream>
#include <string>
#include "boost/json.hpp"
using namespace my_sdk;

void print_json_value(const boost::json::value& val, int indent = 0)
{
    const std::string indent_str(indent * 2, ' ');

    switch (val.kind())
    {
        case boost::json::kind::object:
            {
                const auto& obj = val.get_object();
                std::cout << indent_str << "{\n";
                for (const auto& pair : obj)
                {
                    std::cout << indent_str << "  \"" << pair.key() << "\": ";
                    print_json_value(pair.value(), indent + 1);
                    std::cout << ",\n";
                }
                std::cout << indent_str << "}";
                break;
            }
        case boost::json::kind::array:
            {
                const auto& arr = val.get_array();
                std::cout << indent_str << "[\n";
                for (const auto& elem : arr)
                {
                    print_json_value(elem, indent + 1);
                    std::cout << ",\n";
                }
                std::cout << indent_str << "]";
                break;
            }
        case boost::json::kind::string:
            std::cout << indent_str << "\"" << val.get_string().c_str() << "\"";
            break;
        case boost::json::kind::int64:
            std::cout << indent_str << val.get_int64();
            break;
        case boost::json::kind::uint64:
            std::cout << indent_str << val.get_uint64();
            break;
        case boost::json::kind::double_:
            std::cout << indent_str << std::setprecision(17) << val.get_double();
            break;
        case boost::json::kind::bool_:
            std::cout << indent_str << (val.get_bool() ? "true" : "false");
            break;
        case boost::json::kind::null:
            std::cout << indent_str << "null";
            break;
    }
}
void JsonObjectTest()
{
    const std::string jsonStr = FileSystem::ReadStringFromFile("../TestFiles/testJson.json");
    // 解析JSON
    boost::json::value root = boost::json::parse(jsonStr);

    // ==================== 2. Access Basic Data Types ====================
    const auto& basic = root.at("BaseData").as_object();

    // String
    std::cout << "String: " << basic.at("string").as_string().c_str() << "\n";

    // Integer and float
    std::cout << "Integer: " << basic.at("number").as_int64() << "\n";
    std::cout << "Float: " << std::setprecision(7) << basic.at("float").get_double() << "\n";

    // Boolean
    std::cout << "Boolean: " << std::boolalpha << basic.at("boolean_true").get_bool() << ", " << basic.at("boolean_false").get_bool() << "\n";

    // Null handling
    if (basic.at("null_value").is_null())
    {
        std::cout << "Null value handled successfully\n";
    }

    // ==================== 3. Process Composite Structures ====================
    const auto& composite = root.at("ComplexData").as_object();

    // Empty object check
    if (composite.at("nullObject").as_object().empty())
    {
        std::cout << "\nNull object validation passed\n";
    }

    // Array traversal
    std::cout << "\nMixed-type array contents:\n";
    for (const auto& item : composite.at("mixedTypeArray").as_array())
    {
        print_json_value(item);
        std::cout << "\n";
    }

    // Nested object access
    const std::string deep_value = root.at("ComplexData").at("nestedObject").at("layer1").at("layer2").at("layer3").at("final_value").as_string().c_str();
    std::cout << "\nDeeply nested value: " << deep_value << "\n";

    // ==================== 4. Handle Edge Cases ====================
    const auto& edge = root.at("EdgeCases").as_object();
    std::cout << "\nMax number: " << edge.at("maxNumber").get_double() << "\n";
    std::cout << "Min number: " << edge.at("minNumber").get_double() << "\n";
    std::cout << "Date string: " << edge.at("dateFormat").as_string().c_str() << "\n";

    // ==================== 5. Process Complex Cases ====================
    std::cout << "\nUser list:\n";
    for (const auto& user : root.at("ComplexCases").as_array())
    {
        const auto& user_obj = user.as_object();
        std::cout << "ID: " << user_obj.at("id").as_int64() << ", Name: " << user_obj.at("name").as_string().c_str() << ", Active: " << user_obj.at("active").get_bool() << "\n";

        // Handle nullable values
        if (!user_obj.at("contact").is_null())
        {
            const auto& contact = user_obj.at("contact").as_object();
            std::cout << "  Email: " << contact.at("email").as_string().c_str() << "\n";
            std::cout << "  Phone numbers: ";
            for (const auto& phone : contact.at("phones").as_array())
            {
                std::cout << phone.as_string().c_str() << " ";
            }
            std::cout << "\n";
        }
    }

    // ==================== 6. Full Structure Output ====================
    std::cout << "\nComplete JSON structure:\n";
    print_json_value(root);
    std::cout << "\n";
}
