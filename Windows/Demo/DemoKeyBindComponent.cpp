#include "DemoKeyBindComponent.h"

KeyBinds::Key& ArcdpsExtension::DemoKeyBindComponent::getKeyBind() {
	return mKeyBind;
}

bool ArcdpsExtension::DemoKeyBindComponent::getCloseWithEsc() {
	return true;
}
