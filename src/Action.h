#pragma onces

#include <vector>
#include <string>

enum ActionStartResult {
	Success,
    Failure
};

enum ArgumentProvidingState {
    InProgress,
	Provided,
    Cancelled
};

inline void FillArgumentName(std::vector<std::string> &argVector) {}

template<typename T, typename... Args>
void FillArgumentName(std::vector<std::string> &argVector, T &arg, Args &&... args) {
    argVector.push_back(arg._name);
    FillArgumentName(argVector, args...);
};

inline void ProcessArguments(ArgumentProvidingState& state) { state = Provided; }

template<typename T, typename... Args>
void ProcessArguments(ArgumentProvidingState& state, T &arg, Args &&... args) {
    if(state == InProgress || state == Cancelled) return;
	state = arg.Provide();
};

inline void ProcessStringArguments(int idx, const std::vector<std::string> &stringArgs) {}

template<typename T, typename... Args>
void ProcessStringArguments(int idx, const std::vector<std::string> &stringArgs, T &arg,
                            Args &&... args) {
    arg.ProvideFromString(stringArgs[idx]);
    ProcessStringArguments(idx++, stringArgs, args...);
}

inline void ResetArguments() {}

template<typename T, typename... Args>
void ResetArguments(T &arg, Args &&... args) {
    arg.Reset();
    ResetArguments(args...);
}

class BaseAction {
public:
    BaseAction() = default;

    virtual ~BaseAction() = default;

    virtual ArgumentProvidingState UpdateProviding() = 0;

    virtual ActionStartResult Start(const std::vector<std::string> &stringArgs) = 0;

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

    ArgumentProvidingState UpdateProviding() override {
        auto state = ArgumentProvidingState::Provided;
        auto processor = [&state](auto &&... args) { ((ProcessArguments(state, args)), ...); };
        std::apply(processor, _args);
    	if (state == Provided || state == Cancelled) {
            if (state == Provided) {
                std::apply(_func, _args);
            }
			auto resetter = [](auto &&... args) { ((ResetArguments(args)), ...); };
            std::apply(resetter, _args);
        }
        return state;
    }

    ActionStartResult Start(const std::vector<std::string> &stringArgs) override {
        if (_size <= stringArgs.size()) {
            int idx = 0;
            auto processor = [&](auto &&... args) { ((ProcessStringArguments(idx, stringArgs, args), idx++), ...); };
            std::apply(processor, _args);
            std::apply(_func, _args);

            auto resetter = [](auto &&... args) { ((ResetArguments(args)), ...); };
            std::apply(resetter, _args);
            // todo handle wrong argument providing
            return ActionStartResult::Success;
        }

    	return ActionStartResult::Failure;
    }

private:
    std::string _name;
    std::vector<std::string> _stringArgs;
    Func _func;
    std::tuple<Ts...> _args;
    std::size_t _size;
};