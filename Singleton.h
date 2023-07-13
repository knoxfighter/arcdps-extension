// copied from MIT-Licensed project `gw2radial`
// https://github.com/Friendly0Fire/GW2Radial

#pragma once

#include <memory>
#include <concepts>
#include <stack>
#include <functional>
#include <stdexcept>

/**
 * To use:
 * 1. Define the `extern` variable `g_singletonManagerInstance` in your main.cpp
 * 2. Call `Shutdown` in `mod_release`
 */
class BaseSingleton
{
public:
	virtual ~BaseSingleton() { }
protected:
	static BaseSingleton* Store(std::unique_ptr<BaseSingleton>&& ptr);
	static void Clear(BaseSingleton* ptr);
};

class SingletonManager
{
public:
	SingletonManager() = default;
	// Call this when shutting down `mod_release`
	void Shutdown() {
		while (!singletons_.empty())
			singletons_.pop();
	}

	bool Empty() {
		return singletons_.empty();
	}

private:
	std::stack<std::unique_ptr<BaseSingleton>> singletons_;

	friend class BaseSingleton;
};
// Construct this in your main.cpp
extern SingletonManager g_singletonManagerInstance;

template<typename T, bool AutoInit = true>
class Singleton : public BaseSingleton
{
public:
	static T& instance()
	{
		if constexpr (AutoInit) {
			if (!init_) {
				init_ = true;
				instance_ = (T*)Store(std::make_unique<T>());
			}
		}
		else {
			if (!instance_)
				throw std::logic_error("Singleton is not Auto Init and was not initialized.");
		}
		return *instance_;
	}

	template<typename T2 = T> requires std::derived_from<T2, T>
	static T& instance(std::unique_ptr<T2>&& i)
	{
		if (!init_) {
			init_ = true;
			instance_ = (T*)Store(std::move(i));
		}
		return *instance_;
	}

	template<typename T2 = T> requires std::derived_from<T2, T>
	static void instance(std::function<void(T&)> action)
	{
		if (instance_)
			action(*instance_);
	}

	static void reset()
	{
		if(init_)
			Clear(instance_);
	}

	virtual ~Singleton() {
		instance_ = nullptr;
		init_ = false;
	}

private:
	inline static bool init_ = false;
	inline static T* instance_ = nullptr;
};
