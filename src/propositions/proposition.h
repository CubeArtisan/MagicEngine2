#ifndef _PROPOSITION_H_
#define _PROPOSITION_H_

#include <type_traits>

#include "../environment.h"
#include "../linq/util.h"

template<typename... Args>
class Proposition {
public:
	virtual bool operator()(const Args&... args) const = 0;
};
template<typename T, typename... Args>
using is_proposition = std::is_base_of<Proposition<Args...>, T>;

template<typename T, typename... Args>
constexpr bool is_proposition_v = is_proposition<T, Args...>::value;

template<typename T, typename U>
struct can_upcast_proposition;

template<typename... Args1, typename... Args2>
struct can_upcast_proposition<Proposition<Args1...>, Proposition<Args2...>> {
	constexpr static bool value = is_subset_of<std::tuple<Args1...>, std::tuple<Args2...>>::value;
};

template<typename T, typename... Us>
struct call_operator_with_args;

template<typename... Args1, typename... Args2>
struct call_operator_with_args<Proposition<Args1...>, Args2...> {
	static bool make_call(const Proposition<Args1...>& prop, const Args2&... args) {
		std::tuple<const Args2&...> tuple(args...);
		return prop(std::get<const Args1&>(tuple)...);
	}
};

template<typename Enable, typename Prop, typename... Args>
class UpcastPropositionImpl;

template<typename Prop, typename... Args>
class UpcastPropositionImpl<std::enable_if_t<can_upcast_proposition<ParentOfForm<Prop, Proposition>, Proposition<Args...>>::value>, Prop, Args...> : public Proposition<Args...> {
public:
	using ChildType = ParentOfForm<Prop, ::Proposition>;
	bool operator()(const Args&... args) const override {
		return call_operator_with_args<ChildType, Args...>::make_call(this->prop, args...);
	}

	UpcastPropositionImpl(const Prop& prop)
		: prop(prop)
	{}

private:
	Prop prop;
};

template<typename Prop, typename... Args>
using UpcastProposition = UpcastPropositionImpl<void, Prop, Args...>;

template<typename... Args>
class TrueProposition : public Proposition<Args...> {
public:
	bool operator()(const Args&...) const override {
		return true;
	}
};

template<typename... Args>
class FalseProposition : public Proposition<Args...> {
public:
	bool operator()(const Args&...) const override {
		return false;
	}
};

template<typename... Args>
class PropositionValue : public polyValue<Proposition<Args...>>, public Proposition<Args...> {
public:
	using polyValue<Proposition<Args...>>::polyValue;

	bool operator()(const Args&... args) const override {
		return this->value()(args...);
	}
	PropositionValue()
		: PropositionValue(TrueProposition<Args...>())
	{}
	template<typename Prop, typename Enable = std::enable_if_t<can_upcast_proposition<ParentOfForm<Prop, Proposition>, Proposition<Args...>>::value>>
	PropositionValue(const Prop& other)
		: PropositionValue(UpcastProposition<Prop, Args...>(other))
	{}

	template<typename Prop, typename Enable = std::enable_if_t<can_upcast_proposition<ParentOfForm<Prop, Proposition>, Proposition<Args...>>::value>>
	PropositionValue& operator=(const Prop& other) {
		*this = UpcastProposition<Prop, Args...>(other);
	}
};

template<typename Prop>
class NotProposition;

template<typename... Args>
class NotProposition<Proposition<Args...>> : public Proposition<Args...> {
public:
	bool operator()(const Args&... args) const override {
		return !this->prop(args...);
	}

	template<typename T>
	NotProposition(const T& prop)
		: prop(prop)
	{ }

private:
	PropositionValue<Args...> prop;
};

template<typename T>
NotProposition(T)->NotProposition<ParentOfForm<T, Proposition>>;

template<typename Enable, typename ArgsProp, typename... Args>
class AndPropositionImpl;

template<typename... ArgsPropArgs, typename... Args>
class AndPropositionImpl<std::enable_if_t<std::conjunction_v<can_upcast_proposition<ParentOfForm<Args, Proposition>, Proposition<ArgsPropArgs...>>...>>, Proposition<ArgsPropArgs...>, Args...> : public Proposition<ArgsPropArgs...> {
public:
	bool operator()(const ArgsPropArgs&... args) const override {
		return andAll(args...);
	}
	template<size_t I = 0>
	bool andAll(const ArgsPropArgs&... args) const {
		const Proposition<ArgsPropArgs...>& prop = std::get<I>(children);
		bool result = prop(args...);
		if constexpr (I == sizeof...(Args) - 1) return result;
		else return result && andAll<I + 1>(args...);
	}

	AndPropositionImpl()
	{}

	AndPropositionImpl(Args... args)
		: children{ UpcastProposition<Args, ArgsPropArgs...>(args)... }
	{}

private:
	std::tuple<UpcastProposition<Args, ArgsPropArgs...>...> children;
};

template<typename... Args>
AndPropositionImpl(Args...)->AndPropositionImpl<void, typename union_packs_impl<void, ParentOfForm<Args, Proposition>...>::type, Args...>;

template<typename ArgsProp, typename... Args>
using AndProposition = AndPropositionImpl<void, ArgsProp, Args...>;

template<typename Enable, typename ArgsProp, typename... Args>
class OrPropositionImpl;

template<typename... ArgsPropArgs, typename... Args>
class OrPropositionImpl<std::enable_if_t<std::conjunction_v<is_proposition<Args, ArgsPropArgs...>...>>, std::tuple<ArgsPropArgs...>, Args...> : public Proposition<ArgsPropArgs...> {
public:
	bool operator()(const ArgsPropArgs&... args) const override {
		return orAll<Args...>(args...);
	}
	template<typename T, typename... Left>
	bool orAll(const ArgsPropArgs&... args) const {
		bool result = std::get<T>(children)(args...);
		if constexpr (sizeof...(Left) == 0) return result;
		else return result || orAll<Left...>(args...);
	}

	OrPropositionImpl()
	{}

	OrPropositionImpl(Args... args)
		: children(args...)
	{}

private:
	std::tuple<Args...> children;
};

template<typename... Args>
OrPropositionImpl(Args...)->OrPropositionImpl<void, ParentOfForm<NthTypeOf<0, Args...>, Proposition>, Args...>;

template<typename ArgsTuple, typename... Args>
using OrProposition = OrPropositionImpl<void, ArgsTuple, Args...>;

template<typename Prop>
class LambdaProposition;

template<typename... Args>
class LambdaProposition<Proposition<Args...>> : public Proposition<Args...> {
public:
	using functionType = std::function<bool(const Args&...)>;

	bool operator()(const Args&... args) const override {
		return this->prop(args...);
	}

	template<typename T>
	LambdaProposition(T prop)
		: prop(prop)
	{}

private:
	functionType prop;
};

template<typename T>
LambdaProposition(T)->LambdaProposition<filter_pack_t<std::decay, FunctionToPack_t<T, Proposition>>>;

using EnvProposition = Proposition<Environment>;
using EnvPropositionValue = PropositionValue<Environment>;
using ChangeProposition = Proposition<Changeset>;
using ChangePropositionValue = PropositionValue<Changeset>;

#endif