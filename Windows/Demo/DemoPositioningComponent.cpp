#include "DemoPositioningComponent.h"

Position& ArcdpsExtension::DemoPositioningComponent::getPositionMode() {
	return mPositionMode;
}

CornerPosition& ArcdpsExtension::DemoPositioningComponent::getCornerPosition() {
	return mCornerPosition;
}

ImVec2& ArcdpsExtension::DemoPositioningComponent::getCornerVector() {
	return mCornerVector;
}

CornerPosition& ArcdpsExtension::DemoPositioningComponent::getAnchorPanelCorner() {
	return mAnchorPanelCornerPosition;
}

CornerPosition& ArcdpsExtension::DemoPositioningComponent::getSelfPanelCorner() {
	return mSelfPanelCornerPosition;
}

ImGuiID& ArcdpsExtension::DemoPositioningComponent::getFromWindowId() {
	return mFromWindowID;
}