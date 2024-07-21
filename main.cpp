//
// Created by ZHONG Ziwen on 21/07/2024.
//

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <memory>

class BaseParser {
public:
    virtual void parse(const std::string& input, std::shared_ptr<void>& output) = 0;
};

template<typename T>
class Parser : public BaseParser {
public:
    void parse(const std::string& input, std::shared_ptr<void>& output) override {
        std::istringstream iss(input);
        T value;
        iss >> value;
        output = std::make_shared<T>(value);
    }
};

// 特化向量类型的解析器
template<typename T>
class VectorParser : public BaseParser {
public:
    void parse(const std::string& input, std::shared_ptr<void>& output) override {
        std::istringstream iss(input);
        std::vector<T> values;
        std::string token;
        while (getline(iss, token, ',')) {
            std::istringstream elementStream(token);
            T value;
            elementStream >> value;
            values.push_back(value);
        }
        output = std::make_shared<std::vector<T>>(values);
    }
};

class Table {
    std::vector<std::unordered_map<std::string, std::shared_ptr<void>>> data;
public:
    void parse_table(const std::string& file_dir, const std::vector<std::pair<std::string, BaseParser*>>& schema) {
        std::ifstream file(file_dir);
        std::string line;
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string field;
            std::unordered_map<std::string, std::shared_ptr<void>> row;
            for (auto& [name, parser] : schema) {
                std::getline(ss, field, '\t');
                std::shared_ptr<void> parsed_data;
                parser->parse(field, parsed_data);
                row[name] = parsed_data;
            }
            data.push_back(row);
        }
    }

    template<typename T>
    T get(int index, const std::string& feature_name) {
        if (index < 0 || index >= data.size()) {
            throw std::out_of_range("Index out of range");
        }
        auto it = data[index].find(feature_name);
        if (it == data[index].end()) {
            throw std::invalid_argument("Feature name not found");
        }
        return *std::static_pointer_cast<T>(it->second);
    }

    void register_parser(const std::string& type_name, BaseParser* parser) {
        parsers[type_name] = std::unique_ptr<BaseParser>(parser);
    }

    std::unordered_map<std::string, std::unique_ptr<BaseParser>> parsers;
};

int main() {
    Table table;
    table.register_parser("int", new Parser<int>());
    table.register_parser("float", new Parser<float>());
    table.register_parser("string", new Parser<std::string>());
    table.register_parser("vector<int>", new VectorParser<int>());
    table.register_parser("vector<float>", new VectorParser<float>());
    table.register_parser("vector<string>", new VectorParser<std::string>());

    std::vector<std::pair<std::string, BaseParser*>> schema = {
            {"age", table.parsers["int"].get()},
            {"salary", table.parsers["float"].get()},
            {"name", table.parsers["string"].get()},
            {"subjects", table.parsers["vector<string>"].get()},
            {"scores", table.parsers["vector<int>"].get()}
    };

    table.parse_table("../data.txt", schema);
    std::cout << "Age: " << table.get<int>(0, "age") << std::endl;
    std::cout << "Name: " << table.get<std::string>(0, "name") << std::endl;
    std::cout << "Age: " << table.get<int>(1, "age") << std::endl;
    std::cout << "Name: " << table.get<std::string>(1, "name") << std::endl;
    std::cout << "Subjects: " << table.get<std::vector<std::string>>(1, "subjects")[1] << std::endl;
    std::cout << "Score: " << table.get<std::vector<int>>(1, "scores")[1] << std::endl;
    return 0;
}
