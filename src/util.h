#ifndef _UTIL_H_
#define _UTIL_H_

#include <type_traits>

template<class InputIt1, class InputIt2>
bool intersect(InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2)
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
U tryAtMap(const std::map<T, U> map, const T& key, const U& def) {
	auto iter = map.find(key);
	if (iter != map.end()) return iter->second;
	return def;
}

// -------------------------------------------------------------------
// --- Reversed iterable
// --- https://stackoverflow.com/a/28139075/3300171
template <typename T>
struct reversion_wrapper { T& iterable; };

template <typename T>
auto begin(reversion_wrapper<T> w) { return std::rbegin(w.iterable); }

template <typename T>
auto end(reversion_wrapper<T> w) { return std::rend(w.iterable); }

template <typename T>
reversion_wrapper<T> reverse(T&& iterable) { return { iterable }; }

template<size_t N = 16>
class store
{
	std::byte space[N];

	template<typename T>
	static constexpr bool
		fits() { return sizeof(typename std::decay<T>::type) <= N; }

public:
	template<typename D, typename V>
	D *copy(V &&v)
	{
		return fits<D>() ? new(space) D( std::forward<V>(v) ) :
			new        D( std::forward<V>(v) );
	}

	template<typename D, typename V, typename B>
	B *move(V &&v, B *&p)
	{
		B *q = fits<D>() ? copy<D>(std::forward<V>(v)) : p;
		p = nullptr;
		return q;
	}

	template<typename D>
	void free(D *p) { fits<D>() ? p->~D() : delete p; }
};

template<typename T, typename Store = store<sizeof(T) + 16>>
class polyValue : Store {
	struct base {
		virtual ~base() {}
		virtual T& getValue() = 0;
		virtual const T& getValue() const = 0;
		virtual base* copy(Store& store) const = 0;
		virtual base* move(Store& store, base*& other) const = 0;
		virtual void free(Store& store) const = 0;

	protected:
		virtual T* getValPtr() = 0;
		virtual const T* getValPtr() const = 0;
		template<typename U>
		U* poly_cast() {
			return dynamic_cast<U>(getValPtr());
		}
		template<typename U>
		const U* poly_cast() const {
			return dynamic_cast<U>(getValPtr());
		}
	} *val;

	template<typename U>
	struct data final : base  {
		U val;
		
		U& get() { return val; }
		const U& get() const { return val; }

		T& getValue() override { return val; }
		const T& getValue() const override { return val; }

		base* copy(Store& store) const override {
			return store.copy<data<U>>(get());
		}

		base* move(Store& store, base*& other) const override {
			return store.move<data<U>>(std::move(get()), other);
		}

		void free(Store& store) const override {
			store.free(this);
		}
		
		template<typename V>
		data(V&& val)
			: val(std::forward<V>(val))
		{}

	protected:
		T* getValPtr() override {
			return &val;
		}

		const T* getValPtr() const override {
			return &val;
		}
	};

public:
	template<typename U, typename Enable=std::enable_if_t<std::conjunction_v<std::is_base_of<T, std::decay_t<U>>,
																			 std::negation<std::is_base_of<polyValue<T>, std::decay_t<U>>>>>>
	polyValue(U&& val)
		: val(new data<U>(std::forward<U>(val)))
	{}

	template<typename U, typename Enable=std::enable_if_t<std::is_base_of<T, U>::value>>
	polyValue(const polyValue<U>& other)
		: val(other.val->copy(*this))
	{}

	template<typename U, typename Enable = std::enable_if_t<std::is_base_of<T, U>::value>>
	polyValue(polyValue<U>&& other)
		: val(other.val->move(*this, other.val))
	{}

	template<typename U>
	polyValue<T> operator=(const polyValue<U>& other) {
		this->val->free(*this);
		this->val = other.val->copy(*this);
	}
	
	template<typename U>
		polyValue<T> operator=(polyValue<U>&& other) {
		this->val->free(*this);
		this->val = other.val->move(*this, other.val);
	}

	operator T&() { return val->getValue(); }

	T& value() { return val->getValue(); }
	const T& value() const { return val->getValue(); }
	
	template<typename U>
	bool isCastable() {
		return val->poly_cast<U>();
	}

	template<typename U>
	friend U* poly_cast(polyValue<T> poly) {
		return poly.value->poly_cast<U>();
	}

	T* operator->() {
		return val->getValPtr();
	} 

	virtual ~polyValue() {
		this->val->free(*this);
	}
};
#endif