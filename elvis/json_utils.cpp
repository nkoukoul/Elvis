//
// Copyright (c) 2020-2023 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot
// com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md)
//
// repository: https://github.com/nkoukoul/Elvis
//

#include "json_utils.h"
#include <iostream>

bool Elvis::ValidateStrategy::Validate(std::string const& input) const
{
    std::unordered_map<char, char> pairs = {{'}', '{'}, {']', '['}, {'"', '"'}, {'\'', '\''}};
    std::unordered_set<char> closures = {'}', ']', '"', '\''};
    std::unordered_set<char> openings = {'{', '[', '"', '\''};
    std::stack<char> checker;
    for (auto c : input)
    {
        if (openings.find(c) != openings.end())
        {
            if ((c != '"' || checker.top() != '"') && (c != '\'' || checker.top() != '\''))
                checker.push(c);
            else
                checker.pop();
        }
        else if (closures.find(c) != closures.end())
        {
            if (checker.top() == pairs[c])
                checker.pop();
            else
                return false;
        }
    }
#ifdef DEBUG
    std::cout << "ValidateStrategy::Validate: " << (checker.empty() ? "valid" : "invalid") << "\n";
#endif
    return checker.empty();
}

std::list<std::unordered_map<std::string, std::string>> Elvis::DeserializeStrategy::Deserialize(
    std::string&& input) const
{
#ifdef DEBUG
    std::cout << "DeserializeStrategy::Deserialize: " << input << "\n";
#endif
    std::stack<int> mstack;
    std::list<std::unordered_map<std::string, std::string>> listOfObjects;
    std::unordered_map<std::string, std::string> umap;
    // std::cout << "input is " << input <<"\n";
    int i = 0;
    bool key_turn = true, value_turn = false, count_started = false, isString = false;
    std::string key, value;
    int start_of_key, end_of_key, start_of_value, end_of_value;
    while (i < input.size())
    {
        if (input[i] == '{' && key_turn)
        { // find openining cbrackets
            ;
            ;
        }

        if (input[i] == '\"' && key_turn)
        { // find quotes
            if (!count_started)
            { // start of key
                start_of_key = i + 1;
                count_started = true;
            }
            else
            { // end of key
                end_of_key = i - 1;
                int key_length = end_of_key - start_of_key + 1; // length of value in chars
                key = input.substr(start_of_key, key_length);
                // std::cout << key << ": ";
                key_turn = false;
                value_turn = true;
                count_started = false;
            }
        }

        if (input[i] == ':' && value_turn && !count_started)
        {        // start of value, not part of key/value
            i++; // skip ':'
            while (i < input.size() && input[i] == ' ')
                i++; // skip spaces
            if (i >= input.size())
                return {}; // safe guard
            if (input[i] == '{' || input[i] == '[')
            { // value is list
                char open, close;
                if (input[i] == '{')
                {
                    open = '{';
                    close = '}';
                }
                else
                {
                    open = '[';
                    close = ']';
                }
                start_of_value = i;
                mstack.push(i);
                while (!mstack.empty())
                {
                    i++;
                    if (i >= input.size())
                        return {}; // safe guard
                    if (input[i] == open)
                        mstack.push(i);
                    if (input[i] == close)
                        mstack.pop();
                }
                end_of_value = i;
                int value_length = end_of_value - start_of_value + 1; // length of value in chars
                value = input.substr(start_of_value, value_length);
                // std::cout << value << "\n";
                umap.insert(std::make_pair(key, value));
                // key.erase();
                // value.erase();
                value_turn = false;
                key_turn = true;
                i++; // skip closing bracket
            }
            else
            { // value is not a list
                if (input[i] == '\"')
                {
                    isString = true;
                    i++;
                } // value is string
                start_of_value = i;
                count_started = true;
            }
        }

        if (value_turn && count_started &&
            ((!isString && (input[i] == ',' || input[i] == '{' || input[i] == '}')) || (isString && input[i] == '\"')))
        { // end of value
            end_of_value = i - 1;
            int value_length = end_of_value - start_of_value + 1; // length of value in chars
            value = input.substr(start_of_value, value_length);
            // std::cout << value << "\n";
            umap.insert(std::make_pair(key, value));
            // key.erase();
            // value.erase();
            isString = false;
            value_turn = false;
            key_turn = true;
            count_started = false;
        }

        if (input[i] == '}' && key_turn && !count_started)
        { // find closing cbrackets
            listOfObjects.push_back(umap);
            umap.clear();
        }
        i++;
    }
    return listOfObjects;
}
