#pragma once

#include <iostream>
#include <string>
#include <utility>
#include <type_traits>
#include <vector>


template<typename T>
struct ArgProvider {
    T _arg;
    std::string _name;

    explicit ArgProvider(std::string name) : _name(std::move(name)) {}

    void Provide() {
        std::cout << "provide arg for " << _name << ": ";
        std::cin >> _arg;
        std::cout << std::endl;
    }

    void ProvideStr(const std::string &str) {
        _arg = str;
    }
};

template<>
struct ArgProvider<int> {
    int _arg;
    std::string _name;

    explicit ArgProvider(std::string name) : _name(std::move(name)) {}

    void Provide() {
        std::cout << "provide int arg for " << _name << ": ";
        std::cin >> _arg;
    }

    void ProvideStr(const std::string &str) {
        _arg = std::stoi(str);
    }

    operator int() const {
        return _arg;
    }
};

template<>
struct ArgProvider<std::string> {
    std::string _arg;
    std::string _name;

    explicit ArgProvider(std::string name) : _name(std::move(name)) {}

    void Provide() {
        std::cout << "provide string arg for " << _name << ": ";
        std::cin >> _arg;
    }

    void ProvideStr(const std::string &str) {
        _arg = str;
    }

    operator std::string() const {
        return _arg;
    }
};

template<>
struct ArgProvider<bool> {
    bool _arg;
    std::string _name;

    ArgProvider(const std::string &name) : _name(name) {}

    void Provide() {
        std::cout << "provide boolean arg for " << _name << ": ";
        std::cin >> _arg;
    }

    void ProvideStr(const std::string &str) {
        _arg = (str == "true" || str == "1");
    }

    operator bool() const {
        return _arg;
    }
};