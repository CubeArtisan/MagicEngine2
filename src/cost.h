#ifndef _COST_H_
#define _COST_H_

#include "changeset.h"
#include "mana.h"
#include "util.h"

struct Environment;
struct Player;
struct CardToken;
struct Token;
struct Card;
struct Emblem;

using SourceType = std::variant<std::shared_ptr<const Card>, std::shared_ptr<const Token>, std::shared_ptr<const Emblem>>;

struct CostValue;

struct Cost {
	virtual bool canPay(const Player& player, const Environment& env, const SourceType& source) const = 0;
	virtual Changeset payCost(const Player& player, const Environment& env, const SourceType& source) const = 0;

	virtual Cost& operator+=(const Cost& other) = 0;
	
	virtual Cost& operator-=(const Cost& other) = 0;

	virtual CostValue createCostValue() const = 0;

	virtual const std::type_info& getType() const {
		return typeid(*this);
	}

	virtual ~Cost() {}
};

struct CostValue : public polyValue<Cost>, public Cost {
	using polyValue<Cost>::polyValue;

	bool canPay(const Player& player, const Environment& env, const SourceType& source) const {
		return this->value().canPay(player, env, source);
	}
	Changeset payCost(const Player& player, const Environment& env, const SourceType& source) const {
		return this->value().payCost(player, env, source);
	}
	Cost& operator+=(const Cost& other);
	Cost& operator-=(const Cost& other);
	CostValue createCostValue() const { return *this; }

	const std::type_info& getType() const {
		return typeid(this->value());
	}

	bool operator==(const CostValue& other) {
		return this->getType() == other.getType();
	}
};

struct EmptyCost : public Cost {
	bool canPay(const Player&, const Environment&, const SourceType&) const override { return true; }
	Changeset payCost(const Player&, const Environment&, const SourceType&) const override { return Changeset(); }

	EmptyCost& operator+=(const Cost&) override { return *this; }

	EmptyCost& operator-=(const Cost&) override { return *this; }

	CostValue createCostValue() const override { return CostValue(*this); }
};

template<typename Child>
struct CostImpl : public virtual Cost {
};

struct ManaCost : public virtual CostImpl<ManaCost> {
    bool canPay(const Player& player, const Environment& env, const SourceType& source) const;
    Changeset payCost(const Player& player, const Environment& env, const SourceType& source) const;

	ManaCost& operator+=(const Cost& other) override;
	ManaCost& operator-=(const Cost& other) override;

	CostValue createCostValue() const { return CostValue(*this); }

    ManaCost(Mana mana);

protected:
    Mana mana;
};

struct LandPlayCost : public virtual CostImpl<LandPlayCost> {
    bool canPay(const Player& player, const Environment& env, const SourceType& source) const;
    Changeset payCost(const Player& player, const Environment& env, const SourceType& source) const;

	LandPlayCost& operator+=(const Cost&) { return *this; }
	LandPlayCost& operator-=(const Cost&) { return *this; }

	CostValue createCostValue() const { return CostValue(*this); }
};

struct TapCost : public virtual CostImpl<TapCost> {
    bool canPay(const Player& player, const Environment& env, const SourceType& source) const;
    Changeset payCost(const Player& player, const Environment& env, const SourceType& source) const;

	TapCost& operator+=(const Cost& other);
	TapCost& operator-=(const Cost& other);

	CostValue createCostValue() const { return CostValue(*this); }

protected:
	bool applies;
};

struct CombineCost : public virtual Cost {
	bool canPay(const Player& player, const Environment& env, const SourceType& source) const {
		bool result = true;
		for (const auto& child : this->children) {
			result = result && child.canPay(player, env, source);
		}
		return result;
	}
	Changeset payCost(const Player& player, const Environment& env, const SourceType& source) const {
		Changeset result;
		for (const auto& child : this->children) {
			result += child.payCost(player, env, source);
		}
		return result;
	}

	CombineCost& operator+=(const Cost& other) {
		if (typeid(other) == typeid(CostValue)) {
			return *this += dynamic_cast<const CostValue&>(other).value();
		}
		if (typeid(other) == typeid(CombineCost)) {
			const CombineCost& o = dynamic_cast<const CombineCost&>(other);
			for (const auto& child : o.children) {
				*this += child;
			}
		}
		else {
			bool found = false;
			for (auto& child : this->children) {
				if (child.getType() == typeid(other)) {
					child += other;
					found = true;
					break;
				}
			}
			if (!found) {
				this->children.push_back(other.createCostValue());
			}
		}
		return *this;
	}

	CombineCost& operator-=(const Cost& other) {
		if (typeid(other) == typeid(CostValue)) {
			return *this -= dynamic_cast<const CostValue&>(other).value();
		}
		if (typeid(other) == typeid(CombineCost)) {
			const CombineCost& o = dynamic_cast<const CombineCost&>(other);
			for (const auto& child : o.children) {
				*this -= child;
			}
		}
		else {
			bool found = false;
			for (auto& child : this->children) {
				if (typeid(child.value()) == typeid(other)) {
					child -= other;
					found = true;
					break;
				}
			}
		}
		return *this;
	}

	CostValue createCostValue() const { return CostValue(*this); }

	template<typename... Costs>
	CombineCost(const Costs&... costs)
	{
		addAll(costs...);
	}

	template<typename T, typename... Costs>
	void addAll(const T& cost, const Costs&... costs) {
		*this += cost;
		if constexpr (sizeof...(costs) > 0) this->addAll(costs...);
	}

protected:
	std::vector<CostValue> children;
};
#endif
