#ifndef _UTIL_H_
#define _UTIL_H_

#include <type_traits>

template<class InputIt1, class InputIt2>
bool intersect(InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2) noexcept
{
	while (first1 != last1 && first2 != last2) {
		if (*first1 < *first2) {
			++first1;
			continue;
		}
		if (*first2 < *first1) {
			++first2;
			continue;
		}
		return true;
	}
	return false;
}

template<class T, class U>
U tryAtMap(const std::map<T, U>& map, const T& key, const U& def) noexcept {
	auto iter = map.find(key);
	if (iter != map.end()) return iter->second;
	return def;
}

template<typename U, typename... Ts>
U convertVariant(const std::variant<Ts...>& variant) noexcept
{
	return std::visit([](auto& x) { return U{ x }; }, variant);
}

template<typename U, typename T, size_t i = 0>
U convertToVariantRecursive(const std::shared_ptr<T>& t) {
	using V = std::variant_alternative_t<i, U>;
	using VT = typename V::element_type;
	if (V v = std::dynamic_pointer_cast<VT>(t)) {
		return U(v);
	}
	else {
		if constexpr (i + 1 == std::variant_size_v<U>) {
			throw "Could not convert to the given type";
		}
		else {
			return convertToVariantRecursive<U, T, i + 1>(t);
		}
	}
}

template<typename U, typename T>
U convertToVariant(const std::shared_ptr<T>& t) {
	return convertToVariantRecursive<U>(t);
}

template<typename T, typename Variant>
T& getBaseClass(Variant& variant) {
	auto visitor = [](T& base) -> T& { return base; };
	return std::visit(visitor, variant);
}

template<typename T, typename Variant>
const T* getConstPtr(Variant& variant) {
	auto visitor = [](const T* base) -> const T* { return base; };
	return std::visit(visitor, variant);
}

template<typename T, typename Variant>
std::shared_ptr<T> getBaseClassPtr(const Variant& variant) {
	auto visitor = [](auto base) -> std::shared_ptr<T> { return std::dynamic_pointer_cast<T>(base); };
	return std::visit(visitor, variant);
}

template<typename State, typename Variant, typename Enable>
struct index_of_impl;

template<typename State, template<typename...> typename Pack, typename Variant, typename... Variants>
struct index_of_impl<State, Pack<Variant, Variants...>, std::enable_if_t<!std::is_same_v<Variant, State>>> {
	constexpr static size_t value = 1 + index_of_impl<State, Pack<Variants...>, void>::value;
};

template<typename State, template<typename...> typename Pack, typename Variant, typename... Variants>
struct index_of_impl<State, Pack<Variant, Variants...>, std::enable_if_t<std::is_same_v<Variant, State>>> {
	constexpr static size_t value = 0;
};

template<typename State, typename Variant>
constexpr size_t index_of_v = index_of_impl<State, Variant, void>::value;

/// https://stackoverflow.com/a/5423637/3300171

template<typename T>
struct is_const_pointer { static const bool value = false; };

template<typename T>
struct is_const_pointer<const T*> { static const bool value = true; };

template <typename TIterator>
struct is_const_iterator
{
	using pointer = typename std::iterator_traits<TIterator>::pointer;
	static const bool value = is_const_pointer<pointer>::value;
};

/// END

template<typename T>
struct consted {
	using type = const T;
};

template<typename T>
struct consted<T*> {
	using type = const T*;
};

template<typename T>
struct consted<const T*> {
	using type = const T*;
};

template<typename T>
struct consted<T&> {
	using type = const T&;
};

template<typename T>
struct consted<const T&> {
	using type = const T&;
};

template<typename T>
using consted_t = typename consted<T>::type;

template<typename T>
struct is_pointer_or_reference {
	constexpr static bool value = false;
};

template<typename T>
struct is_pointer_or_reference<T*> {
	constexpr static bool value = true;
};

template<typename T>
struct is_pointer_or_reference<T&> {
	constexpr static bool value = true;
};

template<typename T>
struct is_pointer {
	constexpr static bool value = false;
};

template<typename T>
struct is_pointer<T*> {
	constexpr static bool value = true;
};

template<typename T>
struct is_reference {
	constexpr static bool value = false;
};

template<typename T>
struct is_reference<T&> {
	constexpr static bool value = true;
};

/// https://stackoverflow.com/a/29634934/3300171

namespace detail
{
	// To allow ADL with custom begin/end
	using std::begin;
	using std::end;

	template <typename T>
	auto is_iterable_impl(int)
		-> decltype (
			begin(std::declval<T&>()) != end(std::declval<T&>()), // begin/end and operator !=
			void(), // Handle evil operator ,
			++std::declval<decltype(begin(std::declval<T&>()))&>(), // operator ++
			void(*begin(std::declval<T&>())), // operator*
			std::true_type{});

	template <typename T>
	std::false_type is_iterable_impl(...);
}

template <typename T>
using is_iterable = decltype(detail::is_iterable_impl<T>(0));

/// END

template<template <typename...> typename Container, typename... Args,
	typename Enable = std::enable_if_t<is_iterable<Container<Args...>>::value
	&& !std::is_same_v<std::decay_t<Container<Args...>>, std::string>>>
	std::ostream& operator<<(std::ostream& os, const Container<Args...>& t) {
	os << "( ";
	for (auto& val : t) {
		std::cout << val << " ";
	}
	os << ")";
	return os;
}

template<template <typename...> typename A>
struct RefMap {
	template<typename... Args>
	const A<Args...>& operator()(const A<Args...>& a) { return a; }
};

template<typename T, template <typename...> typename A>
using ParentOfForm = std::remove_const_t<std::remove_reference_t<std::invoke_result_t<RefMap<A>, T>>>;

template<int N, typename... Ts> using NthTypeOf =
typename std::tuple_element<N, std::tuple<Ts...>>::type;

template<template<typename> typename Pack, typename T>
struct ExtractParameterIfPack {
	using type = T;
};

template<template<typename> typename Pack, typename T>
struct ExtractParameterIfPack<Pack, Pack<T>> {
	using type = T;
};

template<template<typename> typename Pack, typename T>
using extract_parameter_if_pack_t = typename ExtractParameterIfPack<Pack, T>::type;

// --- https://stackoverflow.com/a/42583794/3300171
template <class T, class... Us>
struct contains : std::disjunction<std::is_same<T, Us>...> {};

template <typename, typename>
struct is_subset_of;

template <template<typename...> typename Pack, typename... Types1, typename ... Types2>
struct is_subset_of<Pack<Types1...>, Pack<Types2...>> : std::conjunction<contains<Types1, Types2...>...> {};

// ----
template<typename>
struct is_pack : std::false_type {};

template<template<typename...> typename Pack, typename... Args>
struct is_pack<Pack<Args...>> : std::true_type {};

template<template<typename...> typename, typename>
struct is_specific_pack : std::false_type {};

template<template<typename...> typename Pack, typename... Args>
struct is_specific_pack<Pack, Pack<Args...>> : std::true_type {};

template<typename, typename>
struct pack_contains : std::false_type {};

template<template <typename...> typename Pack, typename T, typename... Args>
struct pack_contains<T, Pack<Args...>> : contains<T, Args...> {};

template<typename T, typename U>
constexpr bool pack_contains_v = pack_contains<T, U>::value;

template<typename, typename...>
struct append;

template<template <typename...> typename Pack, typename... Ts, typename... Us>
struct append<Pack<Ts...>, Us...> {
	using type = Pack<Ts..., Us...>;
};

template<typename Pack, typename... Ts>
using append_t = typename append<Pack, Ts...>::type;

template<typename, typename...>
struct prepend;

template<template <typename...> typename Pack, typename... Ts, typename... Us>
struct prepend<Pack<Ts...>, Us...> {
	using type = Pack<Us..., Ts...>;
};

template<typename Pack, typename... Ts>
using prepend_t = typename prepend<Pack, Ts...>::type;

template<typename, typename...>
struct concatenate;

template<template <typename...> typename Pack, typename... Ts, typename... Us, typename... Rest>
struct concatenate<Pack<Ts...>, Pack<Us...>, Rest...> {
	using type = typename concatenate<Pack<Ts..., Us...>, Rest...>::type;
};

template<template<typename...> typename Pack, typename...Ts>
struct concatenate<Pack<Ts...>> {
	using type = Pack<Ts...>;
};

template<typename... Ts>
using concatenate_t = typename concatenate<Ts...>::type;

template<typename>
struct reverse_pack;

template<template <typename...> typename Pack, typename T, typename... Ts>
struct reverse_pack<Pack<T, Ts...>> {
	using type = typename append<typename reverse_pack<Pack<Ts...>>::type, T>::type;
};

template<template <typename...> typename Pack>
struct reverse_pack<Pack<>> {
	using type = Pack<>;
};

template<typename Pack>
using reverse_pack_t = typename reverse_pack<Pack>::type;

template<typename, typename, typename...>
struct union_packs_impl;

template<template <typename...> typename Pack, typename... SoFar, typename T, typename... Types1, typename... Rest>
struct union_packs_impl<std::enable_if_t<std::negation_v<contains<T, SoFar...>>>, Pack<SoFar...>, Pack<T, Types1...>, Rest...> {
	using type = typename union_packs_impl<void, append_t<Pack<SoFar...>, T>, Pack<Types1...>, Rest...>::type;
};

template<template <typename...> typename Pack, typename... SoFar, typename T, typename... Types1, typename... Rest>
struct union_packs_impl<std::enable_if_t<contains<T, SoFar...>::value>, Pack<SoFar...>, Pack<T, Types1...>, Rest...> {
	using type = typename union_packs_impl<void, Pack<SoFar...>, Pack<Types1...>, Rest...>::type;
};

template<template <typename...> typename Pack, typename... SoFar, typename... Rest>
struct union_packs_impl<std::enable_if_t<sizeof...(Rest)>, Pack<SoFar...>, Pack<>, Rest...> {
	using type = typename union_packs_impl<void, Pack<SoFar...>, Rest...>::type;
};

template<template <typename...> typename Pack, typename... SoFar, typename... Rest>
struct union_packs_impl<std::enable_if_t<sizeof...(Rest) == 0>, Pack<SoFar...>, Pack<>, Rest...> {
	using type = Pack<SoFar...>;
};

template<typename T, typename... Args>
using union_packs_t = typename union_packs_impl<void, T, Args...>::type;

template<typename, typename, typename...>
struct intersect_packs_impl;

template<template <typename...> typename Pack, typename T, typename... Args, typename... Rest>
struct intersect_packs_impl<std::enable_if_t<std::conjunction_v<pack_contains<T, Rest>...>>, Pack<T, Args...>, Rest...> {
	using type = prepend_t<T, intersect_packs_impl<void, Pack<Args...>, Rest...>>;
};

template<template <typename...> typename Pack, typename T, typename... Args, typename... Rest>
struct intersect_packs_impl<std::enable_if_t<std::negation_v<std::conjunction<pack_contains<T, Rest>...>>>, Pack<T, Args...>, Rest...> {
	using type = intersect_packs_impl<void, Pack<Args...>, Rest...>;
};

template<template <typename...> typename Pack, typename... Rest>
struct intersect_packs_impl<void, Pack<>, Rest...> {
	using type = Pack<>;
};

template<typename T, typename... Args>
using intersect_packs_t = intersect_packs_impl<void, T, Args...>;

template<typename, typename, typename>
struct difference_packs_impl;

template<template <typename...> typename Pack, typename T, typename... Args, typename... OArgs>
struct difference_packs_impl<std::enable_if_t<contains<T, OArgs...>::value>, Pack<T, Args...>, Pack<OArgs...>> {
	using type = difference_packs_impl<void, Pack<Args...>, Pack<OArgs...>>;
};

template<template <typename...> typename Pack, typename T, typename... Args, typename... OArgs>
struct difference_packs_impl<std::enable_if_t<std::negation_v<contains<T, OArgs...>>>, Pack<T, Args...>, Pack<OArgs...>> {
	using type = prepend_t<T, difference_packs_impl<void, Pack<Args...>, Pack<OArgs...>>>;
};

template<template <typename...> typename Pack, typename Other>
struct difference_packs_impl<void, Pack<>, Other> {
	using type = Pack<>;
};

template<typename T, typename U>
using difference_packs_t = difference_packs_impl<void, T, U>;

template<template<typename> typename Filter, typename T>
struct filter_pack;

template<template<typename> typename Filter, template <typename...> typename Pack, typename... Args>
struct filter_pack<Filter, Pack<Args...>> {
	using type = Pack<typename Filter<Args>::type...>;
};

template<template<typename> typename Filter, typename T>
using filter_pack_t = typename filter_pack<Filter, T>::type;

template<template<typename...> typename, typename, typename = void>
struct flatten_pack_impl;

template<template<typename...> typename Pack, typename... Args>
struct flatten_pack_impl<Pack, Pack<Args...>, void> {
	using type = concatenate_t<typename flatten_pack_impl<Pack, Args, void>::type...>;
};

template<template<typename...> typename Pack, typename T>
struct flatten_pack_impl<Pack, T, std::enable_if_t<std::negation_v<is_specific_pack<Pack, T>>>> {
	using type = Pack<T>;
};

template<template<typename...> typename Pack, typename T>
using flatten_pack_t = typename flatten_pack_impl<Pack, T, void>::type;

template<template<typename...> typename Pack, typename T>
struct switch_pack;

template<template<typename...> typename Pack1, template<typename...> typename Pack2, typename... Args>
struct switch_pack<Pack2, Pack1<Args...>> {
	using type = Pack2<Args...>;
};

template<typename T, typename = void>
struct unique_pack_impl;

template<typename T>
using unique_pack_t = typename unique_pack_impl<T, void>::type;

template<template<typename...> typename Pack, typename T, typename... Args>
struct unique_pack_impl<Pack<Args..., T>, std::enable_if_t<std::negation_v<contains<T, Args...>>>> {
	using type = append_t<unique_pack_t<Pack<Args...>>, T>;
};

template<template<typename...> typename Pack, typename T, typename... Args>
struct unique_pack_impl<Pack<Args..., T>, std::enable_if_t<contains<T, Args...>::value>> {
	using type = typename unique_pack_impl<Pack<Args...>, void>::type;
};

template<template<typename...> typename Pack>
struct unique_pack_impl<Pack<>, void> {
	using type = Pack<>;
};

template<typename T>
struct FunctionType {
	using type = decltype(&T::operator());
};

template<typename Result, typename... Args>
struct FunctionType<Result(*)(Args...)> {
	using type = Result(Args...);
};

template<typename T>
using FunctionType_t = typename FunctionType<T>::type;

template<typename Func, template<typename...> typename Pack>
struct FunctionToPack {
	using type = typename FunctionToPack<FunctionType_t<Func>, Pack>::type;
	using returnType = typename FunctionToPack<FunctionType_t<Func>, Pack>::returnType;
};

template<typename Result, typename T, typename... Args, template<typename...> typename Pack>
struct FunctionToPack<Result(T::*)(Args...) const, Pack> {
	using type = Pack<Args...>;
	using returnType = Result;
};

template<typename Result, typename T, typename... Args, template<typename...> typename Pack>
struct FunctionToPack<Result(T::*)(Args...), Pack> {
	using type = Pack<Args...>;
	using returnType = Result;
};

template<typename Result, typename... Args, template<typename...> typename Pack>
struct FunctionToPack<Result(*)(Args...), Pack> {
	using type = Pack<Args...>;
	using returnType = Result;
};

template<typename T, template <typename...> typename Pack>
using FunctionToPack_t = typename FunctionToPack<T, Pack>::type;

template<template<typename...> typename Pack, typename... Args>
struct DeductionGuideEvaluate {
	using type = decltype(Pack(std::declval<Args>()...));
};

// -------------------------------------------------------------------
// --- Reversed iterable
// --- https://stackoverflow.com/a/28139075/3300171
template <typename T>
struct reversion_wrapper { T& iterable; };

template <typename T>
auto begin(reversion_wrapper<T> w) noexcept { return std::rbegin(w.iterable); }

template <typename T>
auto end(reversion_wrapper<T> w) noexcept { return std::rend(w.iterable); }

template <typename T>
reversion_wrapper<T> reverse(T&& iterable) noexcept { return { iterable }; }

// https://www.fluentcpp.com/2017/09/12/how-to-return-a-smart-pointer-and-use-covariance/
///////////////////////////////////////////////////////////////////////////////

template <typename T>
class abstract_method
{
};

///////////////////////////////////////////////////////////////////////////////

template <typename T>
class virtual_inherit_from : virtual public T
{
	using T::T;
};

///////////////////////////////////////////////////////////////////////////////

template <typename Derived, typename ... Bases>
class clone_inherit : public Bases...
{
public:
	virtual ~clone_inherit() = default;

	std::shared_ptr<Derived> clone() const noexcept
	{
		return std::shared_ptr<Derived>(static_cast<Derived *>(this->clone_impl()));
	}

protected:
	using NthTypeOf<0, Bases...>::NthTypeOf;

private:
	virtual clone_inherit * clone_impl() const noexcept override
	{
		return new Derived(static_cast<const Derived &>(*this));
	}
};

///////////////////////////////////////////////////////////////////////////////

template <typename Derived, typename ... Bases>
class clone_inherit<abstract_method<Derived>, Bases...> : public Bases...
{
public:
	virtual ~clone_inherit() = default;

	std::shared_ptr<Derived> clone() const noexcept
	{
		return std::shared_ptr<Derived>(static_cast<Derived *>(this->clone_impl()));
	}

protected:
	using NthTypeOf<0, Bases...>::NthTypeOf;

private:
	virtual clone_inherit * clone_impl() const noexcept = 0;
};

///////////////////////////////////////////////////////////////////////////////

template <typename Derived>
class clone_inherit<Derived>
{
public:
	virtual ~clone_inherit() = default;

	std::shared_ptr<Derived> clone() const noexcept
	{
		return std::shared_ptr<Derived>(static_cast<Derived *>(this->clone_impl()));
	}

private:
	virtual clone_inherit * clone_impl() const noexcept override
	{
		return new Derived(static_cast<const Derived &>(*this));
	}
};

///////////////////////////////////////////////////////////////////////////////

template <typename Derived>
class clone_inherit<abstract_method<Derived>>
{
public:
	virtual ~clone_inherit() = default;

	std::shared_ptr<Derived> clone() const noexcept
	{
		return std::shared_ptr<Derived>(static_cast<Derived *>(this->clone_impl()));
	}

private:
	virtual clone_inherit * clone_impl() const noexcept = 0;
};

///////////////////////////////////////////////////////////////////////////////

template<size_t N = 16>
class store
{
	std::byte space[N];

	template<typename T>
	static constexpr bool
		fits = sizeof(std::decay_t<T>) <= N;

public:
	template<typename D, typename V>
	D *copy(V &&v) noexcept(noexcept(D(std::forward<V>(v))))
	{
		return fits<D> ? new(space) D(std::forward<V>(v)) :
			new        D(std::forward<V>(v));
	}

	template<typename D, typename V, typename B>
	B *move(V &&v, B *&p) noexcept(noexcept(D(std::forward<V>(v))))
	{
		B *q = fits<D> ? copy<D>(std::forward<V>(v)) : p;
		p = nullptr;
		return q;
	}

	template<typename D>
	void free(D *p) noexcept(noexcept(p->~D())) { fits<D> ? p->~D() : delete p; }
};

template<typename T, typename Store = store<sizeof(T) + 16>>
class polyValue : Store {
private:
	struct base {
		virtual ~base() {}
		virtual T& getValue() noexcept = 0;
		virtual const T& getValue() const noexcept = 0;
		virtual base* copy(Store& store) const noexcept = 0;
		virtual base* move(Store& store, base*& other) noexcept = 0;
		virtual void free(Store& store) const noexcept = 0;
		virtual T* getValPtr() noexcept = 0;
		virtual const T* getValPtr() const noexcept = 0;
		template<typename U>
		U* poly_cast() noexcept {
			return dynamic_cast<U>(getValPtr());
		}
		template<typename U>
		const U* poly_cast() const noexcept {
			return dynamic_cast<U>(getValPtr());
		}
	} *val;

	template<typename U>
	struct data final : base {
		using type = std::remove_const_t<std::remove_reference_t<U>>;
		type val;

		type& get() noexcept { return val; }
		const type& get() const noexcept { return val; }

		type& getValue() noexcept override { return val; }
		const T& getValue() const noexcept override { return val; }

		base* copy(Store& store) const noexcept override {
			return store.template copy<data<U>>(get());
		}

		base* move(Store& store, base*& other) noexcept override {
			return store.template move<data<U>>(std::move(get()), other);
		}

		void free(Store& store) const noexcept override {
			store.free(this);
		}

		template<typename V>
		data(V&& val) noexcept
			: val(std::forward<V>(val))
		{}

		T* getValPtr() noexcept override {
			return &val;
		}

		const T* getValPtr() const noexcept override {
			return &val;
		}
	};

public:
	template<typename U, typename Enable = std::enable_if_t<std::conjunction_v<std::is_base_of<T, std::decay_t<U>>,
		std::negation<std::is_base_of<polyValue<T>, std::decay_t<U>>>>>>
		polyValue(U&& val) noexcept
		: val(new data<U>(std::forward<U>(val)))
	{}

	template<typename U, typename Enable = std::enable_if_t<std::is_base_of<T, U>::value>>
	polyValue(const polyValue<U>& other) noexcept
		: val(other.val->copy(*this))
	{}

	template<typename U, typename Enable = std::enable_if_t<std::is_base_of<T, U>::value>>
	polyValue(polyValue<U>&& other) noexcept
		: val(other.val->move(*this, other.val))
	{}

	template<typename U>
	polyValue<T>& operator=(const polyValue<U>& other) noexcept {
		this->val->free(*this);
		this->val = other.val->copy(*this);
		return *this;
	}

	template<typename U>
	polyValue<T>& operator=(polyValue<U>&& other) noexcept {
		if (this->val) this->val->free(*this);
		this->val = other.val->move(*this, other.val);
		return *this;
	}

	operator T&()  noexcept { return val->getValue(); }

	T& value()  noexcept { return val->getValue(); }
	const T& value() const  noexcept { return val->getValue(); }

	template<typename U>
	bool isCastable()  noexcept {
		return val->poly_cast<U>();
	}

	template<typename U>
	friend U* poly_cast(polyValue<T> poly)  noexcept {
		return poly.val->poly_cast<U>();
	}

	T* operator->()  noexcept {
		return val->getValPtr();
	}

	const T* operator->() const noexcept {
		return val->getValPtr();
	}

	virtual ~polyValue()  noexcept {
		// this->val->free(*this);
	}
};
#endif