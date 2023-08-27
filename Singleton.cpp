#include "Singleton.h"

ArcdpsExtension::SingletonManager ArcdpsExtension::g_singletonManagerInstance;

ArcdpsExtension::BaseSingleton* ArcdpsExtension::BaseSingleton::Store(std::unique_ptr<BaseSingleton>&& ptr) {
	g_singletonManagerInstance.singletons_.push(std::move(ptr));
	return g_singletonManagerInstance.singletons_.top().get();
}

void ArcdpsExtension::BaseSingleton::Clear(BaseSingleton* clearPtr) {
	std::stack<std::unique_ptr<BaseSingleton>> singletons;

	while (!g_singletonManagerInstance.singletons_.empty()) {
		auto ptr = std::move(g_singletonManagerInstance.singletons_.top());
		g_singletonManagerInstance.singletons_.pop();
		if (ptr.get() != clearPtr)
			singletons.push(std::move(ptr));
	}

	std::swap(singletons, g_singletonManagerInstance.singletons_);
}