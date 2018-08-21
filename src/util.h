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

template<typename U, typename T>
U convertToVariant(const std::shared_ptr<T>& t) {
	return convertToVariantRecursive<U>(t);
}

template<typename U, typename T, size_t i=0>
U convertToVariantRecursive(const std::shared_ptr<T>& t) {
	using V = std::variant_alternative_t<i, U>;
	using VT = typename V::element_type;
	if (V v = std::dynamic_pointer_cast<VT>(t)) {
		return U(v);
	}
	else {
		if constexpr(i+1 == std::variant_size_v<U>) {
			throw "Could not convert to the given type";
		}
		else {
			return convertToVariantRecursive<U, T, i+1>(t);
		}
	}
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

template<typename T>
using FunctionEquivalent = std::function<decltype(T::operator())>;

// --- https://stackoverflow.com/a/42583794/3300171
template <class T, class... U>
struct contains : std::disjunction<std::is_same<T, U>...> {};

template <typename, typename>
struct is_subset_of : std::false_type {};

template <typename... Types1, typename ... Types2>
struct is_subset_of<std::tuple<Types1...>, std::tuple<Types2...>> : std::conjunction<contains<Types1, Types2...>...> {};

// ----
template<typename, typename>
struct concat;

template<template <typename...> typename Pack, typename... Ts, typename T>
struct concat<Pack<Ts...>, T> {
	using type = Pack<Ts..., T>;
};

template<typename, typename>
struct prepend;

template<template <typename...> typename Pack, typename... Ts, typename T>
struct prepend<Pack<Ts...>, T> {
	using type = Pack<T, Ts...>;
};

template<typename>
struct reverse_pack;

template<template <typename...> typename Pack, typename T, typename... Ts>
struct reverse_pack<Pack<T, Ts...>> {
	using type = typename concat<typename reverse_pack<Pack<Ts...>>::type, T>::type;
};

template<template <typename...> typename Pack>
struct reverse_pack<Pack<>> {
	using type = Pack<>;
};

template<typename, typename, typename, typename...>
struct union_packs_impl {
	using type = std::false_type;
};

template<template <typename...> typename Pack, typename... SoFar, typename T, typename... Types1, typename... Rest>
struct union_packs_impl<std::enable_if_t<std::negation_v<contains<T, SoFar...>>>, Pack<SoFar...>, Pack<T, Types1...>, Rest...> {
	using next = typename concat<Pack<SoFar...>, T>::type;
	using type = typename union_packs_impl<void, Pack<next...>, Pack<Types1...>, Rest...>::type;
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
using union_packs = typename union_packs_impl<void, T, Args...>::type;

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
		return new Derived(static_cast<const Derived & >(*this));
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
		return new Derived(static_cast<const Derived & >(*this));
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
		fits = sizeof(typename std::decay<T>::type) <= N;

public:
	template<typename D, typename V>
	D *copy(V &&v) noexcept(noexcept(D(std::forward<V>(v))))
	{
		return fits<D> ? new(space) D( std::forward<V>(v) ) :
			new        D( std::forward<V>(v) );
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
	struct base {
		virtual ~base() {}
		virtual T& getValue() noexcept = 0;
		virtual const T& getValue() const noexcept = 0;
		virtual base* copy(Store& store) const noexcept = 0;
		virtual base* move(Store& store, base*& other) const noexcept = 0;
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
	struct data final : base  {
		using type = std::remove_const_t<std::remove_reference_t<U>>;
		type val;
		
		type& get() noexcept { return val; }
		const type& get() const noexcept { return val; }

		type& getValue() noexcept override { return val; }
		const T& getValue() const noexcept override { return val; }

		base* copy(Store& store) const noexcept override {
			return store.template copy<data<U>>(get());
		}

		base* move(Store& store, base*& other) const noexcept override {
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
	template<typename U, typename Enable=std::enable_if_t<std::conjunction_v<std::is_base_of<T, std::decay_t<U>>,
																			 std::negation<std::is_base_of<polyValue<T>, std::decay_t<U>>>>>>
	polyValue(U&& val) noexcept
		: val(new data<U>(std::forward<U>(val)))
	{}

	template<typename U, typename Enable=std::enable_if_t<std::is_base_of<T, U>::value>>
	polyValue(const polyValue<U>& other) noexcept
		: val(other.val->copy(*this))
	{}

	template<typename U, typename Enable = std::enable_if_t<std::is_base_of<T, U>::value>>
	polyValue(polyValue<U>&& other) noexcept
		: val(other.val->move(*this, other.val))
	{}

	template<typename U>
	polyValue<T> operator=(const polyValue<U>& other) noexcept {
		this->val->free(*this);
		this->val = other.val->copy(*this);
	}
	
	template<typename U>
		polyValue<T> operator=(polyValue<U>&& other) noexcept {
		this->val->free(*this);
		this->val = other.val->move(*this, other.val);
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

	virtual ~polyValue()  noexcept {
		// this->val->free(*this);
	}
};
#endif