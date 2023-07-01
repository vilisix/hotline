#pragma onces

#include <vector>
#include <string>

inline void FillArgumentName(std::vector<std::string> &argVector) {}

template<typename T, typename... Args>
void FillArgumentName(std::vector<std::string> &argVector, T &arg, Args &&... args) {
    argVector.push_back(arg._name);
    FillArgumentName(argVector, args...);
};

inline void ProcessArguments() {}

template<typename T, typename... Args>
void ProcessArguments(T &arg, Args &&... args) {
    arg.Provide();
    ProcessArguments(args...);
};

inline void ProcessStringArguments(int idx, const std::vector<std::string> &stringArgs) {}

template<typename T, typename... Args>
void ProcessStringArguments(int idx, const std::vector<std::string> &stringArgs, T &arg,
                            Args &&... args) {
    arg.ProvideStr(stringArgs[idx]);
    ProcessStringArguments(idx++, stringArgs, args...);
}

class BaseAction {
public:
    BaseAction() = default;

    virtual ~BaseAction() = default;

    virtual void Start() {}

    virtual void Start(const std::vector<std::string> &stringArgs) {}

    virtual std::string &GetName() = 0;

    virtual std::vector<std::string> &GetArguments() = 0;
};

template<typename Func, typename... Ts>
class Action : public BaseAction {
    // static_assert(!(std::is_rvalue_reference_v<Ts> && ...));
public:
    template<typename FwdF, typename... FwdTs,
            typename = std::enable_if_t<(std::is_convertible_v<FwdTs &&, Ts> && ...)>>
    Action(const std::string &name, FwdF &&func, FwdTs &&... args)
            : _name(name),
              _func(std::forward<FwdF>(func)),
              _args{std::forward<FwdTs>(args)...},
              _size(std::tuple_size_v<decltype(_args)>) {
        auto processor = [&](auto &&... p_args) { ((FillArgumentName(_stringArgs, p_args)), ...); };
        std::apply(processor, _args);
    }

    ~Action() override {}

    std::string &GetName() override {
        return _name;
    }

    std::vector<std::string> &GetArguments() override {
        return _stringArgs;
    }

    void Start() override {
        auto processor = [](auto &&... args) { ((ProcessArguments(args)), ...); };
        std::apply(processor, _args);
        std::apply(_func, _args);
    }

    void Start(const std::vector<std::string> &stringArgs) override {
        if (_size <= stringArgs.size()) {
            int idx = 0;
            auto processor = [&](auto &&... args) { ((ProcessStringArguments(idx, stringArgs, args), idx++), ...); };
            std::apply(processor, _args);
            std::apply(_func, _args);
        } else {
            Start();
        }
    }

private:
    std::string _name;
    std::vector<std::string> _stringArgs;
    Func _func;
    std::tuple<Ts...> _args;
    std::size_t _size;
};