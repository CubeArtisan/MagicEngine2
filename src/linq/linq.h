#ifndef _LINQ_H_
#define _LINQ_H_

#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

#include "util.h"

namespace linq {
	template<typename Container>
	using iterType = decltype(std::declval<Container>().begin());
	template<typename Container>
	using constIterType = decltype(std::declval<Container>().cbegin());

	template<typename Iter>
	class id;
	template<typename Container>
	id(Container&)->id<iterType<Container>>;
	template<typename Container>
	id(const Container&)->id<constIterType<Container>>;
	template<typename Iter>
	id(Iter, Iter)->id<Iter>;

	template<typename Iter>
	class where;
	template<typename Container, typename Func>
	where(Container&, Func)->where<iterType<Container>>;
	template<typename Container, typename Func>
	where(const Container&, Func)->where<constIterType<Container>>;
	template<typename Iter, typename Func>
	where(Iter, Iter, Func)->where<Iter>;

	template<typename Iter>
	class append;
	template<typename Container, typename value_type>
	append(Container&, value_type)->append<iterType<Container>>;
	template<typename Container, typename value_type>
	append(const Container&, value_type)->append<constIterType<Container>>;
	template<typename Iter, typename value_type>
	append(Iter, Iter, value_type)->append<Iter>;
	template<typename Iter>
	class removeFirst;
	template<typename Container, typename value_type>
	removeFirst(Container&, value_type)->removeFirst<iterType<Container>>;
	template<typename Container, typename value_type>
	removeFirst(const Container&, value_type)->removeFirst<constIterType<Container>>;
	template<typename Iter, typename value_type>
	removeFirst(Iter, Iter, value_type)->removeFirst<Iter>;

	template<typename Iter>
	class concat;
	template<typename Container>
	concat(Container&, Container&)->concat<iterType<Container>>;
	template<typename Container>
	concat(const Container&, Container&)->concat<constIterType<Container>>;
	template<typename Iter>
	concat(Iter, Iter, Iter, Iter)->concat<Iter>;

	template<typename Iter>
	class orderBy;
	template<typename Container>
	orderBy(Container&)->orderBy<iterType<Container>>;
	template<typename Container>
	orderBy(const Container&)->orderBy<constIterType<Container>>;
	template<typename Iter>
	orderBy(Iter, Iter)->orderBy<Iter>;
	template<typename Container, typename Func>
	orderBy(Container&, Func)->orderBy<iterType<Container>>;
	template<typename Container, typename Func>
	orderBy(const Container&, Func)->orderBy<constIterType<Container>>;
	template<typename Iter, typename Func>
	orderBy(Iter, Iter, Func)->orderBy<Iter>;

	template<typename Iter>
	class distinct;
	template<typename Container>
	distinct(Container&)->distinct<iterType<Container>>;
	template<typename Container>
	distinct(const Container&)->distinct<constIterType<Container>>;
	template<typename Iter>
	distinct(Iter, Iter)->distinct<Iter>;

	// group
	// join
	template<typename Iter>
	class intersect;
	template<typename Iter, typename Iter2>
	class zip;

	template<typename Iter>
	class prepend;
	template<typename Container, typename value_type>
	prepend(Container&, value_type)->prepend<iterType<Container>>;
	template<typename Container, typename value_type>
	prepend(const Container&, value_type)->prepend<constIterType<Container>>;
	template<typename Iter, typename value_type>
	prepend(Iter, Iter, value_type)->prepend<Iter>;

	template<typename Iter>
	class reverse;
	template<typename Container>
	reverse(Container&)->reverse<iterType<Container>>;
	template<typename Container>
	reverse(const Container&)->reverse<constIterType<Container>>;
	template<typename Iter>
	reverse(Iter, Iter)->reverse<Iter>;

	template<typename Iter, typename U>
	class select;
	template<typename Container, typename Func>
	select(Container&, Func)->select<iterType<Container>, std::invoke_result_t<Func, const typename std::iterator_traits<iterType<Container>>::value_type&>>;
	template<typename Container, typename Func>
	select(const Container&, Func)->select<iterType<Container>, std::invoke_result_t<Func, const typename std::iterator_traits<constIterType<Container>>::value_type&>>;
	template<typename Iter, typename Func>
	select(Iter, Iter, Func)->select<Iter, std::invoke_result_t<Func(const typename std::iterator_traits<Iter>::value_type&)>>;

	// Can't use store because of a bug in Visual Studio, compiles fine in gcc/clang.
	constexpr size_t iteratorStoreSize = 16;
	template<typename Category, typename Value, typename Difference, typename Pointer, typename Reference>
	class iterator_wrapper /* : store<iteratorStoreSize> */ {
	public:
		using iterator_category = Category;
		using value_type = Value;
		using difference_type = Difference;
		using pointer = Pointer;
		using reference = Reference;

	private:
		struct base {
			virtual reference operator*() = 0;
			virtual consted_t<reference> operator*() const = 0;
			virtual base& operator++() = 0;
			virtual bool operator!=(const base& other) const {
				return !(*this == other);
			}
			virtual bool operator==(const base& other) const = 0;
			virtual base& operator--() = 0;

			virtual base* copy() const noexcept = 0;

			/*
			virtual base* copy(store<iteratorStoreSize>& store) const noexcept = 0;
			virtual base* move(store<iteratorStoreSize>& store, base*& other) noexcept = 0;
			virtual void free(store<iteratorStoreSize>& store) const noexcept = 0;
			*/
		} *val;

		template<typename T>
		struct data final : base {
			T val;

			reference operator*() {
				return *val;
			}

			consted_t<reference> operator*() const {
				return *val;
			}

			data<T>& operator++() {
				++this->val;
				return *this;
			}

			data<T>& operator--() {
				--this->val;
				return *this;
			}

			bool operator==(const base& other) const {
				const data<T>* dataVersion = dynamic_cast<const data<T>*>(&other);
				return dataVersion && *this == *dataVersion;
			}

			data<T>* copy() const noexcept override {
				return new data<T>(this->val);
			}

			/*
			data<T>* copy(store<iteratorStoreSize>& store) const noexcept override {
				return store.template copy<data<T>>(get());
			}

			base* move(store<iteratorStoreSize>& store, base*& other) noexcept override {
				return store.template move<data<T>>(std::move(get()), other);
			}

			void free(store<iteratorStoreSize>& store) const noexcept override {
				store.free(this);
			}
			*/

			template<typename U>
			data(U&& val) noexcept
				: val(std::forward<U>(val))
			{}
		};

	public:
		template<typename U, typename Enable = std::enable_if_t<std::conjunction_v<std::is_same<typename std::iterator_traits<std::decay_t<U>>::iterator_category, iterator_category>, std::is_same<typename std::iterator_traits<std::decay_t<U>>::difference_type, difference_type>, std::is_same<typename std::iterator_traits<std::decay_t<U>>::pointer, pointer>, std::is_same<typename std::iterator_traits<std::decay_t<U>>::reference, reference>>>>
		iterator_wrapper(U&& val)
			: val(new data<std::decay_t<U>>(std::forward<U>(val)))
		{}

		iterator_wrapper(const iterator_wrapper& other)
			: val(other.val->copy())
		{}

		iterator_wrapper(iterator_wrapper&& other)
			: val(other.val)
		{
			other.val = nullptr;
		}

		iterator_wrapper& operator=(const iterator_wrapper& other) {
			if (this->val) delete this->val;
			if (other.val) this->val = other.val->copy();
			else this->val = nullptr;
			return *this;
		}

		iterator_wrapper& operator=(iterator_wrapper&& other) {
			if (this->val) delete this->val;
			this->val = other.val;
			other.val = nullptr;
			return *this;
		}

		reference operator*() {
			return **val;
		}

		consted_t<reference> operator*() const {
			return **val;
		}

		iterator_wrapper& operator++() {
			++(this->val);
			return *this;
		}

		iterator_wrapper& operator--() {
			--(this->val);
			return *this;
		}

		bool operator==(const iterator_wrapper& other) const {
			if (!this->val || !other.val) return this->val == other.val;
			return *(this->val) == *(other.val);
		}

		bool operator!=(const iterator_wrapper& other) const {
			return !(*this == other);
		}

		~iterator_wrapper() {
			if (this->val) delete this->val;
		}
	};

	template<typename Iter>
	iterator_wrapper(Iter&&)->iterator_wrapper<typename std::iterator_traits<Iter>::iterator_category,
		typename std::iterator_traits<Iter>::value_type,
		typename std::iterator_traits<Iter>::difference_type,
		typename std::iterator_traits<Iter>::pointer,
		typename std::iterator_traits<Iter>::reference>;

	template<typename Iter, typename ConstIter, typename BackingIter, typename... Args>
	class abstract_linq {
	protected:
		using iterator_category = typename std::iterator_traits<Iter>::iterator_category;
		using value_type = typename std::iterator_traits<Iter>::value_type;
		using difference_type = typename std::iterator_traits<Iter>::difference_type;
		using pointer = typename std::iterator_traits<Iter>::pointer;
		using reference = typename std::iterator_traits<Iter>::reference;

		using const_iterator_category = typename std::iterator_traits<ConstIter>::iterator_category;
		using const_value_type = typename std::iterator_traits<ConstIter>::value_type;
		using const_difference_type = typename std::iterator_traits<ConstIter>::difference_type;
		using const_pointer = typename std::iterator_traits<ConstIter>::pointer;
		using const_reference = typename std::iterator_traits<ConstIter>::reference;
	public:
		using iterator = iterator_wrapper<iterator_category, value_type, difference_type, pointer, reference>;
		using const_iterator = iterator_wrapper<const_iterator_category, const_value_type, const_difference_type, const_pointer, const_reference>;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	protected:
		BackingIter beginning;
		BackingIter ending;
		std::tuple<Args...> args;
		const decltype(std::index_sequence_for<Args...>()) indices = std::index_sequence_for<Args...>();

		template<size_t... Is>
		iterator begin(const std::index_sequence<Is...>&) {
			return iterator(Iter(this->beginning, std::get<Is>(this->args)...));
		}
		template<size_t... Is>
		const_iterator begin(const std::index_sequence<Is...>&) const {
			return const_iterator(ConstIter(this->beginning, std::get<Is>(this->args)...));
		}
		template<size_t... Is>
		iterator end(const std::index_sequence<Is...>&) {
			return iterator(Iter(this->ending, std::get<Is>(this->args)...));
		}
		template<size_t... Is>
		const_iterator end(const std::index_sequence<Is...>&) const {
			return const_iterator(ConstIter(this->ending, std::get<Is>(this->args)...));
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

		auto where(std::function<bool(const value_type&)> prop) {
			return linq::where{ this->begin(), this->end(), prop };
		}

		template<typename U, template <typename> typename Ptr = std::shared_ptr, typename Enable = std::enable_if_t<std::is_same_v<std::shared_ptr<U>, Ptr<U>>>>
		auto ofType() {
			return this->select([](const value_type& v) { return std::dynamic_pointer_cast<U>(v); }).where([](const Ptr<U>& u) { return (bool)u; });
		}

		/*
		template<typename U, template <typename> typename Ptr, typename Enable = std::enable_if_t<is_pointer<Ptr<U>::value>>>
		decltype(auto) ofType() {
			return this->select([](const value_type& v) { return dynamic_cast<Ptr<U>>(v); }).where([](const Ptr<U> u) { return (bool)u; });
		}

		template<typename U, template <typename> typename Ptr, typename Enable = std::enable_if_t<is_pointer<Ptr<U>::value>>>
		decltype(auto) ofType() {
			return this->where([](const value_type& v) { return (bool)dynamic_cast<const U*>(&v); }).select([](const value_type& v) { return dynamic_cast<Ptr<U>>(v); });
		}
		*/

		auto append(value_type appended) {
			return linq::append(*this, appended);
		}

		auto prepend(value_type prepended) {
			return linq::prepend{ *this, prepended };
		}

		auto reverse() {
			return linq::reverse{ *this };
		}

		template<typename Func>
		auto select(Func func) {
			return linq::select<iterator, std::invoke_result_t<Func, const value_type&>>(*this, func);
		}

		// set_union (concat then distinct)
		// selectMany (select then flatten)

		auto removeFirst(value_type toRemove) {
			return linq::removeFirst(*this, toRemove);
		}

		template<typename Container, typename Enable = std::enable_if_t<std::is_same_v<typename std::iterator_traits<iterType<Container>>::value_type, value_type>>>
		auto removeAll(const Container& container) {
			auto linqed = linq::id(container);
			return linq::where([&linqed](const value_type& val) { return !linqed.contains(val); });
		}

		auto removeAll(Iter begin, Iter end) {
			auto linqed = linq::id(begin, end);
			return linq::where([&begin, &end](const value_type& val) {return !linqed.contains(val); });
		}

		template<typename Container, typename Enable = std::enable_if_t<std::is_same_v<typename std::iterator_traits<iterType<Container>>::value_type, value_type>>>
		auto concat(Container& container) {
			return linq::concat(*this, container);
		}

		template<typename Container, typename Enable = std::enable_if_t<std::is_same_v<typename std::iterator_traits<iterType<Container>>::value_type, value_type>>>
		auto concat(const Container& container) {
			return linq::concat(*this, container);
		}

		auto concat(iterator iter1, iterator iter2) {
			return linq::concat(this->begin(), this->end(), iter1, iter2);
		}

		auto orderBy() {
			return linq::orderBy(*this);
		}

		auto orderBy(std::function<bool(const value_type&, const value_type&)> comparison) {
			return linq::orderBy(*this, comparison);
		}

		// CodeReview: This potentially does computation at call site, evaluating the backing container, might need to be adjusted
		auto take(size_t n) {
			iterator copy = this->begin();
			for (size_t i = 0; i < n; i++) ++copy;
			return linq::id(this->begin(), copy);
		}

		// CodeReview: This potentially does computation at call site, evaluating the backing container, might need to be adjusted
		auto skip(size_t n) {
			iterator copy = this->begin();
			for (size_t i = 0; i < n; i++) ++copy;
			return linq::id(copy, this->end());
		}

		// CodeReview: This potentially does computation at call site, evaluating the backing container, might need to be adjusted
		auto takeWhile(std::function<bool(const value_type&)> prop) {
			iterator copy = this->begin();
			while (prop(*copy)) { ++copy; }
			return linq::id(this->begin(), copy);
		}

		// CodeReview: This potentially does computation at call site, evaluating the backing container, might need to be adjusted
		auto skipWhile(std::function<bool(const value_type&)> prop) {
			iterator copy = this->begin();
			while (prop(*copy)) { ++copy; }
			return linq::id(copy, this->end());
		}

		// CodeReview: This potentially does computation at call site, evaluating the backing container, might need to be adjusted
		template<typename U = value_type, typename Enable = decltype(concat(std::declval(U), std::declval(U)))>
		auto flatten() {
			if (this->count() >= 2) {
				value_type first = this->first();
				value_type second = this->at(1);
				auto result = linq::concat(first, second);
				linq::id(first).skip(2).forEach([&result](const value_type& x) -> void { result.concat(id(x)); });
				return result;
			}
			else if (this->count() == 1) {
				return id(this->first());
			}
			else {
				return id((value_type*)nullptr, (value_type*)nullptr);
			}
		}
	};

	template<typename Iter, bool cons = is_const_iterator<Iter>::value,
		typename Category = typename std::iterator_traits<Iter>::iterator_category,
		typename Value = typename std::iterator_traits<Iter>::value_type,
		typename Difference = typename std::iterator_traits<Iter>::difference_type,
		typename Pointer = typename std::iterator_traits<Iter>::pointer,
		typename Reference = typename std::iterator_traits<Iter>::reference>
		class base_iterator {
		public:
			using iterator_category = Category;
			using value_type = Value;
			using difference_type = Difference;
			using pointer = Pointer;
			using reference = std::conditional_t<cons, consted_t<Reference>, Reference>;

			virtual base_iterator& operator++() {
				++this->current;
				return *this;
			}

			virtual bool operator!=(const base_iterator& other) const {
				return !(*this == other);
			}

			virtual bool operator==(const base_iterator& other) const {
				return this->current == other.current;
			}

			virtual base_iterator& operator--() {
				--this->current;
				return *this;
			}

			// operator++(int)
			// operator--(int)
			// operator+
			// operator-(size_t)
			// operator[]
			// operator-(id_iterator&)

			base_iterator(Iter current)
				: current(current)
			{}

		protected:
			Iter current;
	};

	template<typename Iter, bool cons = is_const_iterator<Iter>::value>
	class id_iterator : public base_iterator<Iter, cons> {
	public:
		using reference = typename std::iterator_traits<Iter>::reference;

		virtual reference operator*() {
			return *current;
		}

		virtual consted_t<reference> operator*() const {
			return *current;
		}

		id_iterator(Iter current)
			: base_iterator<Iter, cons>(current)
		{}

	protected:
		Iter current;
	};

	template<typename Iter>
	class id : public abstract_linq<id_iterator<Iter>, id_iterator<Iter, true>, Iter> {
	public:
		template<typename Container, typename Enable = std::enable_if_t<std::is_same_v<iterType<Container>, Iter>>>
		id(Container& backing)
			: abstract_linq<id_iterator<Iter>, id_iterator<Iter, true>, Iter>(backing.begin(), backing.end())
		{}

		template<typename Container, typename Enable = std::enable_if_t<std::is_same_v<constIterType<Container>, Iter>>>
		id(const Container& backing)
			: abstract_linq<id_iterator<Iter>, id_iterator<Iter, true>, Iter>(backing.cbegin(), backing.cend())
		{}
	};

	template<typename Iter>
	class filter_iterator : public base_iterator<Iter, true, std::input_iterator_tag, typename std::iterator_traits<Iter>::value_type,
		typename std::iterator_traits<Iter>::difference_type, consted_t<typename std::iterator_traits<Iter>::pointer>,
		consted_t<typename std::iterator_traits<Iter>::reference>> {
	public:
		using value_type = typename std::iterator_traits<Iter>::value_type;

		filter_iterator& operator++() {
			do {
				++this->current;
			} while (!filter(*this->current) && this->current != this->end);
			return *this;
		}

		// CodeReview: Implement operator--

		virtual consted_t<typename std::iterator_traits<Iter>::reference> operator*() {
			return *this->current;
		}

		virtual consted_t<typename std::iterator_traits<Iter>::reference> operator*() const {
			return *this->current;
		}

		filter_iterator(Iter current, Iter end, std::function<bool(const value_type&)> filter)
			: base_iterator<Iter, true, std::input_iterator_tag, typename std::iterator_traits<Iter>::value_type,
			typename std::iterator_traits<Iter>::difference_type, consted_t<typename std::iterator_traits<Iter>::pointer>,
			consted_t<typename std::iterator_traits<Iter>::reference>>(current),
			end(end), filter(filter)
		{}

	private:
		Iter end;
		std::function<bool(const value_type&)> filter;
	};

	template<typename Iter>
	class where : public abstract_linq<filter_iterator<Iter>, filter_iterator<Iter>, Iter, Iter, std::function<bool(const typename std::iterator_traits<Iter>::value_type&)>> {
	public:
		using value_type = typename std::iterator_traits<Iter>::value_type;

		// Visual Studio refuses to compile these constructors
		/*
		template<typename Container, typename Enable = std::enable_if_t<std::is_same_v<iterType<Container>, Iter>>>
		where(Container& backing, std::function<bool(const value_type&)> filter)
			: abstract_linq<filter_iterator<Iter>, filter_iterator<Iter>, Iter, Iter, std::function<bool(const typename std::iterator_traits<Iter>::value_type&)>>(backing.begin(), backing.end(), backing.end(), filter)
		{}

		template<typename Container, typename Enable = std::enable_if_t<std::is_same_v<constIterType<Container>, Iter>>>
		where(const Container& backing, std::function<bool(const value_type&)> filter)
			: abstract_linq<filter_iterator<Iter>, filter_iterator<Iter>, Iter, Iter, std::function<bool(const typename std::iterator_traits<Iter>::value_type&)>>(backing.cbegin(), backing.cend(), backing.cend(), filter)
		{}
		*/

		where(Iter beginning, Iter ending, std::function<bool(const value_type&)> filter)
			: abstract_linq<filter_iterator<Iter>, filter_iterator<Iter>, Iter, Iter, std::function<bool(const typename std::iterator_traits<Iter>::value_type&)>>(beginning, ending, ending, filter)
		{}

	private:
		std::function<bool(const value_type&)> function;
	};

	template<typename Iter>
	class reverse : public abstract_linq<std::reverse_iterator<Iter>, std::reverse_iterator<Iter>, Iter> {
	public:
		template<typename Container, typename Enable = std::enable_if_t<std::is_same_v<iterType<Container>, Iter>>>
		reverse(Container& backing)
			: abstract_linq<std::reverse_iterator<Iter>, std::reverse_iterator<Iter>, Iter>(backing.end(), backing.begin())
		{}

		template<typename Container, typename Enable = std::enable_if_t<std::is_same_v<constIterType<Container>, Iter>>>
		reverse(const Container& backing)
			: abstract_linq<std::reverse_iterator<Iter>, std::reverse_iterator<Iter>, Iter>(backing.cend(), backing.cbegin())
		{}

		reverse(Iter beginning, Iter ending)
			: abstract_linq<std::reverse_iterator<Iter>, std::reverse_iterator<Iter>, Iter>(ending, beginning)
		{}
	};

	template<typename Iter, bool cons>
	class append_iterator : public base_iterator<Iter, cons> {
	public:
		using value_type = typename std::iterator_traits<Iter>::value_type;
		using reference = typename std::iterator_traits<Iter>::reference;

		std::conditional_t<cons, consted_t<reference>, reference> operator*() {
			if (this->current == this->ending && !this->at_end) return this->appended;
			return *this->current;
		}

		consted_t<reference> operator*() const {
			if (this->current == this->ending && !this->at_end) return this->appended;
			return *this->current;
		}

		append_iterator& operator++() override {
			if (this->current == this->ending) this->at_end = true;
			else ++this->current;
			return *this;
		}

		// CodeReview: Implement operator--

		bool operator==(const base_iterator<Iter, cons>& other) {
			return this->current == other.current && this->at_end == other.at_end;
		}

		append_iterator(Iter current, Iter ending, value_type appended, bool at_end = false)
			: base_iterator<Iter, cons>(current), ending(ending), appended(appended), at_end(at_end)
		{}

	private:
		Iter ending;
		value_type appended;
		bool at_end;
	};

	template<typename Iter>
	class append : public abstract_linq<append_iterator<Iter, is_const_iterator<Iter>::value>, append_iterator<Iter, true>, Iter, Iter, typename std::iterator_traits<Iter>::value_type> {
	public:
		typename abstract_linq<append_iterator<Iter, is_const_iterator<Iter>::value>, append_iterator<Iter, true>, Iter, Iter, typename std::iterator_traits<Iter>::value_type>::iterator end() override {
			return iterator_wrapper(append_iterator<Iter, is_const_iterator<Iter>::value>{ this->ending, std::get<0>(this->args), std::get<1>(this->args), true });
		}

		typename abstract_linq<append_iterator<Iter, is_const_iterator<Iter>::value>, append_iterator<Iter, true>, Iter, Iter, typename std::iterator_traits<Iter>::value_type>::const_iterator end() const override {
			return iterator_wrapper(append_iterator<Iter, true>{ this->ending, std::get<0>(this->args), std::get<1>(this->args), true });
		}

		using value_type = typename std::iterator_traits<Iter>::value_type;

		template<typename Container, typename Enable = std::enable_if_t<std::is_same_v<iterType<Container>, Iter>>>
		append(Container& backing, value_type appended)
			: abstract_linq<append_iterator<Iter, is_const_iterator<Iter>::value>, append_iterator<Iter, true>, Iter, Iter, value_type>(backing.begin(), backing.end(), backing.cend(), appended)
		{}

		template<typename Container, typename Enable = std::enable_if_t<std::is_same_v<constIterType<Container>, Iter>>>
		append(const Container& backing, value_type appended)
			: abstract_linq<append_iterator<Iter, is_const_iterator<Iter>::value>, append_iterator<Iter, true>, Iter, Iter, value_type>(backing.cbegin(), backing.cend(), backing.cend(), appended)
		{}

		append(Iter beginning, Iter ending, value_type appended)
			: abstract_linq<std::reverse_iterator<Iter>, std::reverse_iterator<Iter>, Iter>(beginning, ending, ending, appended)
		{}
	};

	template<typename Iter, bool cons>
	class prepend_iterator : public base_iterator<Iter, cons> {
	public:
		using value_type = typename base_iterator<Iter, cons>::value_type;
		using reference = typename base_iterator<Iter, cons>::reference;

		std::conditional_t<cons, consted_t<reference>, reference> operator*() {
			if (this->at_beginning) return this->prepended;
			return *this->current;
		}

		consted_t<reference> operator*() const {
			if (this->at_beginning) return this->prepended;
			return *this->current;
		}

		prepend_iterator& operator++() override {
			if (this->at_beginning) this->at_beginning = false;
			else ++this->current;
			return *this;
		}

		// CodeReview: Implement operator--

		bool operator==(const base_iterator<Iter, cons>& other) {
			return this->current == other.current && this->at_beginning == other.at_beginning;
		}

		prepend_iterator(Iter current, value_type prepended, bool at_beginning = false)
			: base_iterator<Iter, cons>(current), prepended(prepended), at_beginning(at_beginning)
		{}

	private:
		value_type prepended;
		bool at_beginning;
	};

	template<typename Iter>
	class prepend : public abstract_linq<prepend_iterator<Iter, is_const_iterator<Iter>::value>, prepend_iterator<Iter, true>, Iter, typename std::iterator_traits<Iter>::value_type> {
	public:
		typename abstract_linq<prepend_iterator<Iter, is_const_iterator<Iter>::value>, prepend_iterator<Iter, true>, Iter, typename std::iterator_traits<Iter>::value_type>::iterator begin() override {
			return iterator_wrapper(prepend_iterator<Iter, is_const_iterator<Iter>::value>(this->beginning, std::get<0>(this->args), true));
		}

		typename abstract_linq<append_iterator<Iter, is_const_iterator<Iter>::value>, append_iterator<Iter, true>, Iter, typename std::iterator_traits<Iter>::value_type>::const_iterator begin() const override {
			return iterator_wrapper(prepend_iterator<Iter, true>(this->beginning, std::get<0>(this->args), true));
		}

		using value_type = typename std::iterator_traits<Iter>::value_type;

		template<typename Container, typename Enable = std::enable_if_t<std::is_same_v<iterType<Container>, Iter>>>
		prepend(Container& backing, value_type prepended)
			: abstract_linq<prepend_iterator<Iter, is_const_iterator<Iter>::value>, prepend_iterator<Iter, true>, Iter, value_type>(backing.begin(), backing.end(), prepended)
		{}

		template<typename Container, typename Enable = std::enable_if_t<std::is_same_v<constIterType<Container>, Iter>>>
		prepend(const Container& backing, value_type prepended)
			: abstract_linq<prepend_iterator<Iter, is_const_iterator<Iter>::value>, prepend_iterator<Iter, true>, Iter, value_type>(backing.cbegin(), backing.cend(), prepended)
		{}

		prepend(Iter beginning, Iter ending, value_type prepended)
			: abstract_linq<prepend_iterator<Iter, is_const_iterator<Iter>::value>, prepend_iterator<Iter, true>, Iter, value_type>(beginning, ending, prepended)
		{}
	};

	template<typename Iter, typename U>
	class select_iterator : public base_iterator<Iter, true, std::input_iterator_tag, U, typename std::iterator_traits<Iter>::difference_type, U*, U> {
	public:
		U operator*() {
			return func(*this->current);
		}

		U operator*() const {
			return func(*this->current);
		}

		select_iterator(Iter current, std::function<U(const typename std::iterator_traits<Iter>::value_type&)> func)
			: base_iterator<Iter, true, std::input_iterator_tag, U, typename std::iterator_traits<Iter>::difference_type, U*, U>(current), func(func)
		{}

	private:
		std::function<U(const typename std::iterator_traits<Iter>::value_type&)> func;
	};

	template<typename Iter, typename U>
	class select : public abstract_linq<select_iterator<Iter, U>, select_iterator<Iter, U>, Iter, std::function<U(const typename std::iterator_traits<Iter>::value_type&)>> {
	public:
		using func_type = std::function<U(const typename std::iterator_traits<Iter>::value_type&)>;

		template<typename Container, typename Func, typename Enable = std::enable_if_t<std::is_same_v<iterType<Container>, Iter>>>
		select(Container& backing, Func func)
			: abstract_linq<select_iterator<Iter, U>, select_iterator<Iter, U>, Iter, func_type>(backing.begin(), backing.end(), func)
		{}

		template<typename Container, typename Func, typename Enable = std::enable_if_t<std::is_same_v<constIterType<Container>, Iter>>>
		select(const Container& backing, Func func)
			: abstract_linq<select_iterator<Iter, U>, select_iterator<Iter, U>, Iter, func_type>(backing.cbegin(), backing.cend(), func)
		{}

		template<typename Func>
		select(Iter beginning, Iter ending, Func func)
			: abstract_linq<select_iterator<Iter, U>, select_iterator<Iter, U>, Iter, func_type>(beginning, ending, func)
		{}
	};

	template<typename Iter>
	class removeFirst_iterator : public base_iterator<Iter, true, std::input_iterator_tag, typename std::iterator_traits<Iter>::value_type,
		typename std::iterator_traits<Iter>::difference_type, consted_t<typename std::iterator_traits<Iter>::pointer>,
		consted_t<typename std::iterator_traits<Iter>::reference>> {
	public:
		using value_type = typename std::iterator_traits<Iter>::value_type;
		using reference = typename std::iterator_traits<Iter>::reference;

		consted_t<reference> operator*() {
			return *this->current;
		}

		consted_t<reference> operator*() const {
			return *this->current;
		}

		removeFirst_iterator& operator++() {
			++this->current;
			if (!removed && *this->current == toRemove)++this->current;
			return *this;
		}

		// CodeReview: Implement operator--

		removeFirst_iterator(Iter current, value_type toRemove)
			: base_iterator<Iter, true, std::input_iterator_tag, typename std::iterator_traits<Iter>::value_type,
			typename std::iterator_traits<Iter>::difference_type, consted_t<typename std::iterator_traits<Iter>::pointer>,
			consted_t<typename std::iterator_traits<Iter>::reference>>(current),
			toRemove(toRemove)
		{}

	private:
		value_type toRemove;
		bool removed{ false };
	};

	template<typename Iter>
	class removeFirst : public abstract_linq<removeFirst_iterator<Iter>, removeFirst_iterator<Iter>, Iter, typename std::iterator_traits<Iter>::value_type> {
	public:
		using value_type = typename std::iterator_traits<Iter>::value_type;

		template<typename Container, typename Enable = std::enable_if_t<std::is_same_v<iterType<Container>, Iter>>>
		removeFirst(Container& backing, value_type toRemove)
			: abstract_linq<removeFirst_iterator<Iter>, removeFirst_iterator<Iter>, Iter, value_type>(backing.begin(), backing.end(), toRemove)
		{}

		template<typename Container, typename Enable = std::enable_if_t<std::is_same_v<constIterType<Container>, Iter>>>
		removeFirst(const Container& backing, value_type toRemove)
			: abstract_linq<removeFirst_iterator<Iter>, removeFirst_iterator<Iter>, Iter, value_type>(backing.cbegin(), backing.cend(), toRemove)
		{}

		removeFirst(Iter beginning, Iter ending, value_type toRemove)
			: abstract_linq<removeFirst_iterator<Iter>, removeFirst_iterator<Iter>, Iter, value_type>(beginning, ending, toRemove)
		{}
	};

	template<typename Iter, bool cons>
	class concat_iterator : public base_iterator<Iter, cons> {
	public:
		using reference = typename std::iterator_traits<Iter>::reference;

		std::conditional_t<cons, consted_t<reference>, reference> operator*() {
			return *this->current;
		}

		consted_t<reference> operator*() const {
			return *this->current;
		}

		concat_iterator& operator++() {
			if (++this->current == this->firstEnding) this->current = this->secondBegin;
			return *this;
		}

		concat_iterator& operator--() {
			if (this->current == this->secondBegin) this->current = this->firstEnding;
			--this->current;
			return *this;
		}

		concat_iterator(Iter current, Iter firstEnding, Iter secondBegin)
			: base_iterator<Iter, cons>(current), firstEnding(firstEnding), secondBegin(secondBegin)
		{}

	private:
		Iter firstEnding;
		Iter secondBegin;
	};

	template<typename Iter>
	class concat : public abstract_linq<concat_iterator<Iter, is_const_iterator<Iter>::value>, concat_iterator<Iter, true>, Iter, Iter, Iter> {
	public:
		template<typename Container, typename Enable = std::enable_if_t<std::is_same_v<iterType<Container>, Iter>>>
		concat(Container& backing1, Container& backing2)
			: abstract_linq<concat_iterator<Iter, is_const_iterator<Iter>::value>, concat_iterator<Iter, true>, Iter, Iter, Iter>(backing1.begin(), backing2.end(), backing1.end(), backing2.begin())
		{}

		template<typename Container, typename Enable = std::enable_if_t<std::is_same_v<constIterType<Container>, Iter>>>
		concat(const Container& backing1, const Container& backing2)
			: abstract_linq<concat_iterator<Iter, is_const_iterator<Iter>::value>, concat_iterator<Iter, true>, Iter, Iter, Iter>(backing1.cbegin(), backing2.cend(), backing1.cend(), backing2.cbegin())
		{}

		concat(Iter beginning1, Iter ending1, Iter beginning2, Iter ending2)
			: abstract_linq<concat_iterator<Iter, is_const_iterator<Iter>::value>, concat_iterator<Iter, true>, Iter, Iter, Iter>(beginning1, ending2, ending1, beginning2)
		{}
	};

	template<typename T>
	class PairingHeap {
	public:
		bool empty() {
			return !this->elem;
		}

		// Doesn't change the container
		T findMind() const {
			if (this->empty()) throw "Finding in empty heap";
			else return this->elem;
		}

		// Assume that in the original container all elements in *this were left of all elements in other
		// and that for all PairingHeaps recursively elem is their minimal leftmost element
		// and each subheaps array is in left to right order meaning all elements of the leftmost heap come
		// leftmost of all those in the rightmost
		// Then there are three cases
		// Case 1 this->elem < other.elem: Then we know the new elem should be this->elem and since we know other
		// is rightmost of *this we can append it to our list
		// Case 2 this->elem == other.elem: Then since we know *this is leftmost of other we should use this->elem
		// and append other to our list
		// Case 3 this->elem > other.elem: Then we know that the new elem should be other.elem and since we are leftmost
		// of other we can prepend ourselves to their list of subheaps
		// Therefore merge maintains the given assumptions
		PairingHeap<T> merge(const PairingHeap<T>& other) const {
			if (this->empty()) return other;
			else if (other.empty()) return *this;
			else if (comparison(this->elem, other.elem)) {
				std::vector newSubHeaps = linq::append(this->subheaps, other).toVector();
				return PairingHeap<T>{ this->elem, newSubHeaps };
			}
			else {
				std::vector newSubHeaps = linq::prepend(other.subheaps, *this).toVector();
				return PairingHeap<T>{ other.elem, newSubHeaps };
			}
		}

		// Assume that in the original container all elements in *this were left of toInsert
		// and that for all PairingHeaps recursively elem is their minimal leftmost element
		// and each subheaps array is in left to right order meaning all elements of the leftmost heap come
		// leftmost of all those in the rightmost
		// If we create a new heap with just the element toInsert as elem we satisfy the above assumptions for merge
		// Therefore merging those two will maintain our heap assumptions
		PairingHeap<T> insert(T toInsert) const {
			return this->merge(PairingHeap<T>{toInsert, {}});
		}

		// Assume that for all PairingHeaps recursively elem is their minimal leftmost element
		// and each subheaps array is in left to right order meaning all elements of the leftmost heap come
		// leftmost of all those in the rightmost
		// Then we know that elem is the leftmost minimal element so removing it is the correct thing to do
		// That leaves us with just the subtrees which follow the above property. If mergePairs on them
		// creates a structure maintaining the property we're good(see mergePairs)
		PairingHeap<T> deleteMin() const {
			if (this->empty()) throw "Finding in empty heap";
			else return this->mergePairs(this->subheaps);
		}

		template<typename U = T, typename Enable = decltype(std::declval(U) <= std::declval(U))>
		PairingHeap()
			: comparison([](const U& a, const U& b) { return a <= b; })
		{}
		PairingHeap(std::function<bool(const T&, const T&)> comparison)
			: comparison(comparison)
		{}

	private:
		std::optional<T> elem;
		std::vector<PairingHeap<T>> subheaps;
		std::function<bool(const T&, const T&)> comparison;

		// Assume that for all PairingHeaps recursively elem is their minimal leftmost element
		// and each subheaps array is in left to right order meaning all elements of the leftmost heap come
		// leftmost of all those in the rightmost and that the list is in left to right order
		// If the list is empty we create an empty structure which maintains the property
		// If the list is singleton we can return that element
		// If the list has at least two elements we merge the first two elements which maintains the property
		// Then we recursively mergePairs on the rest of the list which maintains the property by this proof
		// Finally we merge those two structures which maintains the desired property
		PairingHeap<T> mergePairs(const std::vector<PairingHeap<T>>& list) const {
			if (list.size() == 0) return { std::nullopt, {} };
			else if (list.size() == 1) return list[0];
			else return merge(merge(list[0], list[1]), mergePairs(std::vector<PairingHeap<T>>(list.begin() + 2, list.end())));
		}
	};

	template<typename Iter>
	class orderBy_iterator : public base_iterator<Iter, true, std::input_iterator_tag> {
	public:
		using value_type = typename std::iterator_traits<Iter>::value_type;
		using reference = consted_t<typename std::iterator_traits<Iter>::reference>;

		reference operator*() {
			if (!this->initialized) this->initialize();
			return this->heap.findMin();
		}

		reference operator*() const {
			if (!this->initialized) this->initialize();
			return this->heap.findMin();
		}

		orderBy_iterator& operator++() override {
			if (!this->initialized) this->initialize();
			this->heap = this->heap.deleteMin();
		}

		orderBy_iterator& operator--() override {
			throw "Unsupported operation on orderBy_iterator";
		}

		orderBy_iterator(Iter current, Iter ending)
			: base_iterator<Iter, true, std::input_iterator_tag>(current), ending(ending)
		{}

		orderBy_iterator(Iter current, Iter ending, std::function<bool(const value_type&, const value_type&)> comparison)
			: base_iterator<Iter, true, std::input_iterator_tag>(current), heap([](const std::reference_wrapper<const value_type>& a, const std::reference_wrapper<const value_type>& b) { return comparison(a, b); }), ending(ending)
		{}

	private:
		mutable PairingHeap<std::reference_wrapper<const value_type>> heap;
		mutable bool initialized{ false };
		Iter ending;

		void initialize() const {
			for (const value_type& value : id(this->current, this->ending)) {
				this->heap = this->heap.insert(std::reference_wrapper(value));
			}
			this->initialized = true;
		}
	};

	template<typename Iter>
	class orderBy : public abstract_linq<orderBy_iterator<Iter>, orderBy_iterator<Iter>, Iter, Iter, std::function<bool(const typename std::iterator_traits<Iter>::value_type&, const typename std::iterator_traits<Iter>::value_type&)>> {
	public:
		using value_type = typename std::iterator_traits<Iter>::value_type;

		template<typename Container, typename Enable = std::enable_if_t<std::is_same_v<iterType<Container>, Iter>>>
		orderBy(Container& backing)
			: abstract_linq<orderBy_iterator<Iter>, orderBy_iterator<Iter>, Iter, Iter, std::function<bool(const value_type&, const value_type&)>>(backing.begin(), backing.end(), backing.end(), defaultComparison)
		{}

		template<typename Container, typename Enable = std::enable_if_t<std::is_same_v<constIterType<Container>, Iter>>>
		orderBy(const Container& backing)
			: abstract_linq<orderBy_iterator<Iter>, orderBy_iterator<Iter>, Iter, Iter, std::function<bool(const value_type&, const value_type&)>>(backing.cbegin(), backing.cend(), backing.cend(), defaultComparison)
		{}

		orderBy(Iter beginning, Iter ending)
			: abstract_linq<orderBy_iterator<Iter>, orderBy_iterator<Iter>, Iter, Iter, std::function<bool(const value_type&, const value_type&)>>(beginning, ending, ending, defaultComparison)
		{}

		template<typename Container, typename Enable = std::enable_if_t<std::is_same_v<iterType<Container>, Iter>>>
		orderBy(Container& backing, std::function<bool(const value_type&, const value_type&)> comparison)
			: abstract_linq<orderBy_iterator<Iter>, orderBy_iterator<Iter>, Iter, Iter, std::function<bool(const value_type&, const value_type&)>>(backing.begin(), backing.end(), backing.end(), comparison)
		{}

		template<typename Container, typename Enable = std::enable_if_t<std::is_same_v<constIterType<Container>, Iter>>>
		orderBy(const Container& backing, std::function<bool(const value_type&, const value_type&)> comparison)
			: abstract_linq<orderBy_iterator<Iter>, orderBy_iterator<Iter>, Iter, Iter, std::function<bool(const value_type&, const value_type&)>>(backing.cbegin(), backing.cend(), backing.cend(), comparison)
		{}

		orderBy(Iter beginning, Iter ending, std::function<bool(const value_type&, const value_type&)> comparison)
			: abstract_linq<removeFirst_iterator<Iter>, removeFirst_iterator<Iter>, Iter, value_type>(beginning, ending, ending, comparison)
		{}

	private:
		inline static std::function<bool(const value_type&, const value_type&)> defaultComparison{ [](const value_type& a, const value_type& b) { return a < b; } };
	};

	template<typename Iter>
	class distinct_iterator : public base_iterator<Iter, true, std::input_iterator_tag> {
	public:
		using value_type = typename std::iterator_traits<Iter>::value_type;
		using reference = typename base_iterator<Iter, true, std::input_iterator_tag>::reference;

		reference operator*() {
			return *this->current;
		}

		reference operator*() const {
			return *this->current;
		}

		distinct_iterator<Iter>& operator++() {
			while (this->seen.find(*(++this->current)) != this->seen.end()) {}
			this->seen.insert(*this->current);
			return *this;
		}

		distinct_iterator<Iter>& operator--() {
			throw "Unsupported operation on distinct_iterator";
		}

		distinct_iterator(Iter current)
			: base_iterator<Iter, true, std::input_iterator_tag>(current)
		{ }

	private:
		std::set<value_type> seen;
	};

	template<typename Iter>
	class distinct : public abstract_linq<distinct_iterator<Iter>, distinct_iterator<Iter>, Iter> {
	public:
		using value_type = typename std::iterator_traits<Iter>::value_type;

		template<typename Container, typename Enable = std::enable_if_t<std::is_same_v<iterType<Container>, Iter>>>
		distinct(Container& backing)
			: abstract_linq<distinct_iterator<Iter>, distinct_iterator<Iter>, Iter>(backing.begin(), backing.end())
		{}

		template<typename Container, typename Enable = std::enable_if_t<std::is_same_v<constIterType<Container>, Iter>>>
		distinct(const Container& backing)
			: abstract_linq<distinct_iterator<Iter>, distinct_iterator<Iter>, Iter>(backing.cbegin(), backing.cend())
		{}

		distinct(Iter beginning, Iter ending)
			: abstract_linq<distinct_iterator<Iter>, distinct_iterator<Iter>, Iter>(beginning, ending)
		{}
	};
}
#endif
