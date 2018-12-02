#ifndef _LINQ_H_
#define _LINQ_H_

#include <functional>
#include <memory>
#include <type_traits>

#include "util.h"

namespace linq {
	template<typename Iter>
	class id;
	template<typename Container>
	id(Container&)->id<decltype(Container().begin())>;
	template<typename Container>
	id(const Container&)->id<decltype(Container().cbegin())>;
	template<typename Iter>
	id(Iter, Iter)->id<Iter>;

	template<typename Iter>
	class filter;
	template<typename Container, typename Func>
	filter(Container&, Func)->filter<decltype(Container().begin())>;
	template<typename Container, typename Func>
	filter(const Container&, Func)->filter<decltype(Container().cbegin())>;
	template<typename Iter, typename Func>
	filter(Iter, Iter, Func)->filter<Iter>;

	template<typename Iter, typename U, template<typename> typename Ptr>
	class cast;
	template<typename Container, typename U, template <typename> typename Ptr = std::shared_ptr>
	cast(Container&)->cast<decltype(Container().begin()), U, Ptr>;
	template<typename Container, typename U, template <typename> typename Ptr = std::shared_ptr>
	cast(const Container&)->cast<decltype(Container().cbegin()), U, Ptr>;
	template<typename Iter, typename U, template <typename> typename Ptr>
	cast(Iter, Iter)->cast<Iter, U, Ptr>;

	template<typename Iter>
	class append;
	template<typename Container, typename value_type>
	append(Container&, value_type)->append<decltype(Container().begin())>;
	template<typename Container, typename value_type>
	append(const Container&, value_type)->append<decltype(Container().cbegin())>;
	template<typename Iter, typename value_type>
	append(Iter, Iter, value_type)->append<Iter>;

	template<typename Iter, typename Iter2>
	class concat;
	template<typename Iter, typename Iter2>
	class distinct;
	template<typename Iter, typename Iter2>
	class removeAll;
	template<typename Iter, typename T>
	class removeFirst;
	// group
	// join
	template<typename Iter, typename Iter2>
	class intersect;
	template<typename Iter>
	class orderBy;

	template<typename Iter>
	class prepend;
	template<typename Container, typename value_type>
	prepend(Container&, value_type)->prepend<decltype(Container().begin())>;
	template<typename Container, typename value_type>
	prepend(const Container&, value_type)->prepend<decltype(Container().cbegin())>;
	template<typename Iter, typename value_type>
	prepend(Iter, Iter, value_type)->prepend<Iter>;

	template<typename Iter>
	class reverse;
	template<typename Container>
	reverse(Container&)->reverse<decltype(Container().begin())>;
	template<typename Container>
	reverse(const Container&)->reverse<decltype(Container().cbegin())>;
	template<typename Iter>
	reverse(Iter, Iter)->reverse<Iter>;

	template<typename Iter, typename U>
	class select;
	template<typename Container, typename Func>
	select(Container&, Func)->select<decltype(Container().begin()), std::invoke_result_t<Func(const typename std::iterator_traits<decltype(Container().begin())>::value_type&)>>;
	template<typename Container, typename Func>
	select(const Container&, Func)->select<decltype(Container().cbegin()), std::invoke_result_t<Func(const typename std::iterator_traits<decltype(Container().cbegin())>::value_type&)>>;
	template<typename Iter, typename Func>
	select(Iter, Iter, Func)->select<Iter, std::invoke_result_t<Func(const typename std::iterator_traits<Iter>::value_type&)>>;

	template<typename Iter>
	class flatten;
	template<typename Iter, typename Container>
	class selectMany;
	template<typename Iter>
	class skip;
	template<typename Iter>
	class skipWhile;
	template<typename Iter>
	class take;
	template<typename Iter>
	class takeWhile;
	template<typename Iter, typename Iter2>
	class set_union;
	template<typename Iter, typename Iter2>
	class zip;

	template<typename Iter, typename ConstIter, typename BackingIter, typename... Args>
	class abstract_linq {
	public:
		using iterator = Iter;
		using const_iterator = ConstIter;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	protected:
		BackingIter beginning;
		BackingIter ending;
		std::tuple<Args...> args;
		const decltype(std::index_sequence_for<Args...>()) indices = std::index_sequence_for<Args...>();

		template<size_t... Is>
		iterator begin(const std::index_sequence<Is...>&) {
			return iterator(this->beginning, std::get<Is>(this->args)...);
		}
		template<size_t... Is>
		const_iterator begin(const std::index_sequence<Is...>&) const {
			return const_iterator(this->beginning, std::get<Is>(this->args)...);
		}
		template<size_t... Is>
		iterator end(const std::index_sequence<Is...>&) {
			return iterator(this->ending, std::get<Is>(this->args)...);
		}
		template<size_t... Is>
		const_iterator end(const std::index_sequence<Is...>&) const {
			return const_iterator(this->ending, std::get<Is>(this->args)...);
		}

	public:
		abstract_linq(BackingIter beginning, BackingIter ending, Args... args)
			: beginning(beginning), ending(ending), args(args...)
		{}

		virtual iterator begin() {
			return this->begin(indices);
		}
		virtual const_iterator begin() const {
			return this->begin(indices);
		}
		virtual const_iterator cbegin() const {
			return this->begin();
		}
		virtual reverse_iterator rbegin() {
			return std::reverse_iterator(this->end());
		}
		virtual const_reverse_iterator rbegin() const {
			return std::reverse_iterator(this->end());
		}
		virtual iterator end() {
			return this->end(indices);
		}
		virtual const_iterator end() const {
			return this->end(indices);
		}
		virtual const_iterator cend() const {
			return this->end();
		}
		virtual reverse_iterator rend() {
			return std::reverse_iterator(this->begin());
		}
		virtual const_reverse_iterator rend() const {
			return std::reverse_iterator(this->begin());
		}

		using value_type = typename Iter::value_type;
		using difference_type = typename Iter::difference_type;

		// EqualityComparison

		difference_type count() const {
			return this->end() - this->begin();
		}

		bool contains(const value_type& value) const {
			for (const value_type& contained : *this) {
				if (value == contained) return true;
			}
			return false;
		}

		void forEach(std::function<void(value_type&)> func) {
			for (value_type& value : *this) {
				func(value);
			}
		}

		void forEach(std::function<void(const value_type&)> func) const {
			for (const value_type& value : *this) {
				func(value);
			}
		}

		template<typename U>
		U aggregate(std::function<U(U, const value_type&)> aggregator, U start = {}) {
			this->forEach([aggregator, start](U u, const value_type& value) { return aggregator(u, value); });
			return start;
		}

		value_type& at(size_t index) {
			Iter begin = this->begin();
			std::advance(begin, index);
			return *begin;
		}

		const value_type& at(size_t index) const {
			const_iterator begin = this->begin();
			std::advance(begin, index);
			return *begin;
		}

		value_type& operator[](size_t index) {
			return this->at(index);
		}

		const value_type& operator[](size_t index) const {
			return this->at(index);
		}

		value_type atOrDefault(size_t index, const value_type& def) const {
			if (index > this->count()) return def;
			return this->at(index);
		}

		bool empty() const {
			return this->begin() == this->end();
		}

		bool all(std::function<bool(const value_type&)> pred) const {
			for (const value_type& contained : *this) {
				if (!pred(contained)) return false;
			}
			return true;
		}

		bool any(std::function<bool(const value_type&)> pred) const {
			for (const value_type& contained : *this) {
				if (pred(contained)) return true;
			}
			return false;
		}

		typename Iter::reference first() {
			return *this->begin();
		}

		const value_type& first() const {
			return *this->begin();
		}

		value_type firstOrDefault(const value_type& def) const {
			if (this->empty()) return def;
			return this->first();
		}

		value_type& last() {
			return *(this->begin() + (this->count() - 1));
		}

		const value_type& last() const {
			return *(this->begin() + (this->count() - 1));
		}

		value_type lastOrDefault(const value_type& def) const {
			if (this->empty()) return def;
			return this->last();
		}

		std::vector<value_type> toVector() const {
			return { this->begin(), this->end() };
		}

		filter<iterator> where(std::function<bool(const value_type&)> prop) {
			return { *this, prop };
		}

		template<typename U, template <typename> typename Ptr = std::shared_ptr>
		cast<iterator, U, Ptr> ofType() {
			return { *this };
		}

		append<iterator> append(value_type appended) {
			return { *this, appended };
		}

		prepend<iterator> prepend(value_type prepended) {
			return { *this, prepended };
		}

		template<typename U>
		select<iterator, U> select(std::function<U(const value_type&)> func) {
			return { *this, func };
		}
	};

	template<typename Iter, bool cons = is_const_iterator<Iter>::value,
		typename Category = typename std::iterator_traits<Iter>::iterator_category,
		typename Value = typename std::iterator_traits<Iter>::value_type,
		typename Difference = typename std::iterator_traits<Iter>::difference_type,
		typename Pointer = typename std::iterator_traits<Iter>::pointer,
		typename Reference = typename std::iterator_traits<Iter>::reference>
		class id_iterator {
		public:
			using iterator_category = Category;
			using value_type = Value;
			using difference_type = Difference;
			using pointer = Pointer;
			using reference = Reference;

			virtual id_iterator& operator++() {
				++this->current;
				return *this;
			}

			virtual bool operator!=(const id_iterator& other) const {
				return !(*this == other);
			}

			virtual bool operator==(const id_iterator& other) const {
				return this->current == other.current;
			}

			virtual id_iterator& operator--() {
				--this->current;
				return *this;
			}

			// operator++(int)
			// operator--(int)
			// operator+
			// operator-(size_t)
			// operator[]
			// operator-(id_iterator&)

			id_iterator(Iter current)
				: current(current)
			{}

		protected:
			Iter current;
	};

	template<typename Iter>
	class id : public abstract_linq<id_iterator<Iter>, id_iterator<Iter, true>, Iter> {
	public:
		template<typename Container, typename Enable = std::enable_if_t<std::is_same_v<decltype(Container().begin()), Iter>>>
		id(Container& backing)
			: abstract_linq<id_iterator<Iter>, id_iterator<Iter, true>, Iter>(backing.begin(), backing.end())
		{}

		template<typename Container, typename Enable = std::enable_if_t<std::is_same_v<decltype(Container().cbegin()), Iter>>>
		id(const Container& backing)
			: abstract_linq<id_iterator<Iter>, id_iterator<Iter, true>, Iter>(backing.cbegin(), backing.cend())
		{}
	};

	template<typename Iter>
	class filter_iterator : public id_iterator<Iter, true, std::input_iterator_tag, typename std::iterator_traits<Iter>::value_type,
		typename std::iterator_traits<Iter>::difference_type, consted_t<typename std::iterator_traits<Iter>::pointer>,
		typename std::iterator_traits<Iter>::value_type> {
	public:
		using value_type = typename id_iterator < Iter, true, std::input_iterator_tag, typename std::iterator_traits<Iter>::value_type,
			typename std::iterator_traits<Iter>::difference_type, consted_t<typename std::iterator_traits<Iter>::pointer>, typename std::iterator_traits<Iter>::value_type>::value_type;

		filter_iterator& operator++() {
			do {
				++this->current;
			} while (!filter(*this->current) && this->current != this->end);
			return *this;
		}

		virtual consted_t<typename std::iterator_traits<Iter>::reference> operator*() {
			return *this->current;
		}

		virtual consted_t<typename std::iterator_traits<Iter>::reference> operator*() const {
			return *this->current;
		}

		filter_iterator(Iter current, Iter end, std::function<bool(const value_type&)> filter)
			: id_iterator<Iter, true, std::input_iterator_tag, typename std::iterator_traits<Iter>::value_type, typename std::iterator_traits<Iter>::difference_type, consted_t<typename std::iterator_traits<Iter>::pointer>, typename std::iterator_traits<Iter>::value_type>(current),
			end(end), filter(filter)
		{}

	private:
		Iter end;
		std::function<bool(const value_type&)> filter;
	};

	template<typename Iter>
	class filter : public abstract_linq<filter_iterator<Iter>, filter_iterator<Iter>, Iter, Iter, std::function<bool(const typename std::iterator_traits<Iter>::value_type&)>> {
	public:
		using value_type = typename std::iterator_traits<Iter>::value_type;

		template<typename Container, typename Enable = std::enable_if_t<std::is_same_v<decltype(Container().begin()), Iter>>>
		filter(Container& backing, std::function<bool(const value_type&)> filter)
			: abstract_linq<filter_iterator<Iter>, filter_iterator<Iter>, Iter, Iter, std::function<bool(const typename std::iterator_traits<Iter>::value_type&)>>(backing.begin(), backing.end(), backing.end(), filter)
		{}

		template<typename Container, typename Enable = std::enable_if_t<std::is_same_v<decltype(Container().cbegin()), Iter>>>
		filter(const Container& backing, std::function<bool(const value_type&)> filter)
			: abstract_linq<filter_iterator<Iter>, filter_iterator<Iter>, Iter, Iter, std::function<bool(const typename std::iterator_traits<Iter>::value_type&)>>(backing.cbegin(), backing.cend(), backing.cend(), filter)
		{}

		filter(Iter beginning, Iter ending, std::function<bool(const value_type&)> filter)
			: abstract_linq<filter_iterator<Iter>, filter_iterator<Iter>, Iter, Iter, std::function<bool(const typename std::iterator_traits<Iter>::value_type&)>>(beginning, ending, ending, filter)
		{}

	private:
		std::function<bool(const value_type&)> function;
	};

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

	template<typename Iter, typename U, template<typename> typename Ptr = std::shared_ptr, typename Enable = void>
	class cast_iterator;

	template<typename Iter, typename U, template<typename> typename Ptr>
	class cast_iterator<Iter, U, Ptr, std::enable_if_t<is_pointer_or_reference<Ptr<U>>::value>> : public  id_iterator<Iter, true, std::input_iterator_tag, Ptr<U>, typename std::iterator_traits<Iter>::difference_type, const Ptr<U>*, Ptr<U>> {
	public:
		const Ptr<U> operator*() {
			return dynamic_cast<Ptr<U>>(*this->current);
		}
		consted_t<Ptr<U>> operator*() const {
			return dynamic_cast<consted<Ptr<U>>>(*this->current);
		}

		template<typename R = U, typename Enable = std::enable_if_t<is_pointer<Ptr<R>>::value>>
		cast_iterator(std::enable_if_t<is_pointer<Ptr<R>>::value, Iter> current)
			: id_iterator<Iter, true, std::input_iterator_tag, Ptr<U>, typename std::iterator_traits<Iter>::difference_type, const Ptr<U>*, const Ptr<U>&>(filter<Iter>(current, [](const Ptr<typename std::iterator_traits<Iter>::value_type>& t) { return (bool)dynamic_cast<Ptr<const U>>(t); }).begin())
		{}

		template<typename R = U, typename Enable = std::enable_if_t<is_reference<Ptr<R>>::value>>
		cast_iterator(std::enable_if_t<is_reference<Ptr<R>>::value, Iter> current)
			: id_iterator<Iter, true, std::input_iterator_tag, Ptr<U>, typename std::iterator_traits<Iter>::difference_type, const Ptr<U>*, const Ptr<U>&>(filter<Iter>(current, [](const Ptr<typename std::iterator_traits<Iter>::value_type>& t) { return (bool)dynamic_cast<const U*>(&t); }).begin())
		{}

		template<typename R = U, typename Enable = std::enable_if_t<std::is_same_v<std::shared_ptr<R>, Ptr<R>>>>
		cast_iterator(std::enable_if_t<std::is_same_v<std::shared_ptr<R>, Ptr<R>>, Iter> current)
			: id_iterator<Iter, true, std::input_iterator_tag, Ptr<U>, typename std::iterator_traits<Iter>::difference_type, const Ptr<U>*, const Ptr<U>&>(filter<Iter>(current, [](const Ptr<typename std::iterator_traits<Iter>::value_type>& t) { return (bool)std::dynamic_pointer_cast<U>(t); }).begin())
		{}
	};

	template<typename Iter, typename U, template<typename> typename Ptr>
	class cast_iterator<Iter, U, Ptr, std::enable_if_t<std::is_same_v<Ptr<U>, std::shared_ptr<U>>>> : public id_iterator<typename filter<Iter>::iterator, true, std::input_iterator_tag, Ptr<U>, typename std::iterator_traits<Iter>::difference_type, const Ptr<U>*, Ptr<U>> {
	public:
		const Ptr<U> operator*() {
			return std::dynamic_pointer_cast<U>(*this->current);
		}
		consted_t<Ptr<U>> operator*() const {
			return std::dynamic_pointer_cast<U>(*this->current);
		}

		template<typename R = U, typename Enable = std::enable_if_t<is_pointer<Ptr<R>>::value>>
		cast_iterator(std::enable_if_t<is_pointer<Ptr<R>>::value, Iter> current, Iter end)
			: id_iterator<typename filter<Iter>::iterator, true, std::input_iterator_tag, Ptr<U>, typename std::iterator_traits<Iter>::difference_type, const Ptr<U>*, Ptr<U>>(filter<Iter>(current, end, [](const typename std::iterator_traits<Iter>::value_type& t) { return (bool)dynamic_cast<Ptr<const U>>(t); }).begin())
		{}

		template<typename R = U, typename Enable = std::enable_if_t<is_reference<Ptr<R>>::value>>
		cast_iterator(std::enable_if_t<is_reference<Ptr<R>>::value, Iter> current, Iter end)
			: id_iterator<typename filter<Iter>::iterator, true, std::input_iterator_tag, Ptr<U>, typename std::iterator_traits<Iter>::difference_type, const Ptr<U>*, Ptr<U>>(filter<Iter>(current, end, [](const typename std::iterator_traits<Iter>::value_type& t) { return (bool)dynamic_cast<const U*>(&t); }).begin())
		{}

		template<typename R = U, typename Enable = std::enable_if_t<std::is_same_v<std::shared_ptr<R>, Ptr<R>>>>
		cast_iterator(std::enable_if_t<std::is_same_v<std::shared_ptr<R>, Ptr<R>>, Iter> current, Iter end)
			: id_iterator<typename filter<Iter>::iterator, true, std::input_iterator_tag, Ptr<U>, typename std::iterator_traits<Iter>::difference_type, const Ptr<U>*, Ptr<U>>(filter<Iter>(current, end, [](const typename std::iterator_traits<Iter>::value_type& t) { return (bool)std::dynamic_pointer_cast<U>(t); }).begin())
		{}
	};

	template<typename Iter, typename U, template <typename> typename Ptr = std::shared_ptr>
	class cast : public abstract_linq<cast_iterator<Iter, U, Ptr>, cast_iterator<Iter, U, Ptr>, Iter, Iter> {
	public:
		template<typename Container, typename Enable = std::enable_if_t<std::is_same_v<decltype(Container().begin()), Iter>>>
		cast(Container& backing)
			: abstract_linq<cast_iterator<Iter, U, Ptr>, cast_iterator<Iter, U, Ptr>, Iter, Iter>(backing.begin(), backing.end(), backing.end())
		{}

		template<typename Container, typename Enable = std::enable_if_t<std::is_same_v<decltype(Container().cbegin()), Iter>>>
		cast(const Container& backing)
			: abstract_linq<cast_iterator<Iter, U, Ptr>, cast_iterator<Iter, U, Ptr>, Iter, Iter>(backing.cbegin(), backing.cend(), backing.cend())
		{}

		cast(Iter beginning, Iter ending)
			: abstract_linq<cast_iterator<Iter, U, Ptr>, cast_iterator<Iter, U, Ptr>, Iter, Iter>(beginning, ending)
		{}
	};

	template<typename Iter>
	class reverse : public abstract_linq<std::reverse_iterator<Iter>, std::reverse_iterator<Iter>, Iter> {
	public:
		template<typename Container, typename Enable = std::enable_if_t<std::is_same_v<decltype(Container().begin()), Iter>>>
		reverse(Container& backing)
			: abstract_linq<std::reverse_iterator<Iter>, std::reverse_iterator<Iter>, Iter>(backing.end(), backing.begin())
		{}

		template<typename Container, typename Enable = std::enable_if_t<std::is_same_v<decltype(Container().cbegin()), Iter>>>
		reverse(const Container& backing)
			: abstract_linq<std::reverse_iterator<Iter>, std::reverse_iterator<Iter>, Iter>(backing.cend(), backing.cbegin())
		{}

		reverse(Iter beginning, Iter ending)
			: abstract_linq<std::reverse_iterator<Iter>, std::reverse_iterator<Iter>, Iter>(ending, beginning)
		{}
	};

	template<typename Iter, bool cons>
	class append_iterator : public id_iterator<Iter, cons> {
	public:
		using value_type = typename std::iterator_traits<Iter>::value_type;
		using reference = typename std::iterator_traits<Iter>::reference;

		std::conditional_t<cons, consted_t<reference>, reference> operator*() override {
			if (this->current == this->ending && !this->at_end) return this->appended;
			return *this->current;
		}

		consted_t<reference> operator*() const override {
			if (this->current == this->ending && !this->at_end) return this->appended;
			return *this->current;
		}

		append_iterator& operator++() override {
			if (this->current == this->ending) this->at_end = true;
			else ++this->current;
			return *this;
		}

		bool operator==(const id_iterator<Iter, cons>& other) override {
			return this->current == other.current && this->at_end == other.at_end;
		}

		append_iterator(Iter current, Iter ending, value_type appended, bool at_end = false)
			: id_iterator<Iter, cons>(current), ending(ending), appended(appended), at_end(at_end)
		{}

	private:
		Iter ending;
		value_type appended;
		bool at_end;
	};

	template<typename Iter>
	class append : public abstract_linq<append_iterator<Iter, is_const_iterator<Iter>::value>, append_iterator<Iter, true>, Iter, Iter, typename std::iterator_traits<Iter>::value_type> {
	public:
		append_iterator<Iter, is_const_iterator<Iter>::value> end() override {
			return { this->ending, std::get<0>(this->args), std::get<1>(this->args), true };
		}

		append_iterator<Iter, true> end() const override {
			return { this->ending, std::get<0>(this->args), std::get<1>(this->args), true };
		}

		using value_type = typename std::iterator_traits<Iter>::value_type;

		template<typename Container, typename Enable = std::enable_if_t<std::is_same_v<decltype(Container().begin()), Iter>>>
		append(Container& backing, value_type appended)
			: abstract_linq<append_iterator<Iter, is_const_iterator<Iter>::value>, append_iterator<Iter, true>, Iter, Iter, value_type>(backing.begin(), backing.end(), backing.cend(), appended)
		{}

		template<typename Container, typename Enable = std::enable_if_t<std::is_same_v<decltype(Container().cbegin()), Iter>>>
		append(const Container& backing, value_type appended)
			: abstract_linq<append_iterator<Iter, is_const_iterator<Iter>::value>, append_iterator<Iter, true>, Iter, Iter, value_type>(backing.cbegin(), backing.cend(), backing.cend(), appended)
		{}

		append(Iter beginning, Iter ending, value_type appended)
			: abstract_linq<std::reverse_iterator<Iter>, std::reverse_iterator<Iter>, Iter>(beginning, ending, ending, appended)
		{}
	};

	template<typename Iter, bool cons>
	class prepend_iterator : public id_iterator<Iter, cons> {
	public:
		using value_type = typename std::iterator_traits<Iter>::value_type;
		using reference = typename std::iterator_traits<Iter>::reference;

		std::conditional_t<cons, consted_t<reference>, reference> operator*() override {
			if (this->at_beginning) return this->prepended;
			return *this->current;
		}

		consted_t<reference> operator*() const override {
			if (this->at_beginning) return this->prepended;
			return *this->current;
		}

		prepend_iterator& operator++() override {
			if (this->at_beginning) this->at_end = true;
			else ++this->current;
			return *this;
		}

		bool operator==(const id_iterator<Iter, cons>& other) override {
			return this->current == other.current && this->at_beginning == other.at_beginning;
		}

		prepend_iterator(Iter current, value_type prepended, bool at_beginning = false)
			: id_iterator<Iter, cons>(current), prepended(prepended), at_beginning(at_beginning)
		{}

	private:
		value_type prepended;
		bool at_end;
	};

	template<typename Iter>
	class prepend : public abstract_linq<append_iterator<Iter, is_const_iterator<Iter>::value>, append_iterator<Iter, true>, Iter, typename std::iterator_traits<Iter>::value_type> {
	public:
		prepend_iterator<Iter, is_const_iterator<Iter>::value> begin() override {
			return { this->beginning, std::get<0>(this->args), true };
		}

		prepend_iterator<Iter, true> begin() const override {
			return { this->beginning, std::get<0>(this->args), true };
		}

		using value_type = typename std::iterator_traits<Iter>::value_type;

		template<typename Container, typename Enable = std::enable_if_t<std::is_same_v<decltype(Container().begin()), Iter>>>
		prepend(Container& backing, value_type prepended)
			: abstract_linq<append_iterator<Iter, is_const_iterator<Iter>::value>, append_iterator<Iter, true>, Iter, value_type>(backing.begin(), backing.end(), prepended)
		{}

		template<typename Container, typename Enable = std::enable_if_t<std::is_same_v<decltype(Container().cbegin()), Iter>>>
		prepend(const Container& backing, value_type prepended)
			: abstract_linq<append_iterator<Iter, is_const_iterator<Iter>::value>, append_iterator<Iter, true>, Iter, Iter, value_type>(backing.cbegin(), backing.cend(), prepended)
		{}

		prepend(Iter beginning, Iter ending, value_type prepended)
			: abstract_linq<std::reverse_iterator<Iter>, std::reverse_iterator<Iter>, Iter>(beginning, ending, ending, prepended)
		{}
	};

	template<typename Iter, typename U>
	class select_iterator : public id_iterator<Iter, true, std::input_iterator_tag, U, typename std::iterator_traits<Iter>::difference_type, U*, U> {
	public:
		U operator*() override {
			return func(*this->current);
		}
		U operator*() const override {
			return func(*this->current);
		}

		select_iterator(Iter current, std::function<U(const typename std::iterator_traits<Iter>::value_type&)> func)
			: id_iterator<Iter, std::input_iterator_tag, U, typename std::iterator_traits<Iter>::difference_type, U*, U>(current), func(func)
		{}

	private:
		std::function<U(const typename std::iterator_traits<Iter>::value_type&)> func;
	};

	template<typename Iter, typename U>
	class select : abstract_linq<select_iterator<Iter, U>, select_iterator<Iter, U>, Iter, std::function<U(const typename std::iterator_traits<Iter>::value_type&)>> {
	public:
		using func_type = std::function<U(const typename std::iterator_traits<Iter>::value_type&)>;

		template<typename Container, typename Enable = std::enable_if_t<std::is_same_v<decltype(Container().begin()), Iter>>>
		select(Container& backing, func_type func)
			: abstract_linq<select_iterator<Iter, U>, select_iterator<Iter, U>, Iter, func_type>(backing.begin(), backing.end(), func)
		{}

		template<typename Container, typename Enable = std::enable_if_t<std::is_same_v<decltype(Container().cbegin()), Iter>>>
		select(const Container& backing, func_type func)
			: abstract_linq<select_iterator<Iter, U>, select_iterator<Iter, U>, Iter, func_type>(backing.cbegin(), backing.cend(), func)
		{}

		select(Iter beginning, Iter ending, func_type func)
			: abstract_linq<select_iterator<Iter, U>, select_iterator<Iter, U>, Iter, func_type>(beginning, ending, ending, func)
		{}
	};
}
#endif
