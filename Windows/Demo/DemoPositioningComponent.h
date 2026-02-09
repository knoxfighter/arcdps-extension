#pragma once

#include "../MainWindow.h"
#include "../PositioningComponent.h"
#include "arcdps_structs.h"

#include <imgui/imgui.h>

namespace ArcdpsExtension {
	class DemoPositioningComponent final : public PositioningComponent {
	public:
		explicit DemoPositioningComponent(MainWindow* pMainWindow)
			: PositioningComponent(pMainWindow) {}

	protected:
		Position& getPositionMode() override;
		CornerPosition& getCornerPosition() override;
		ImVec2& getCornerVector() override;
		CornerPosition& getAnchorPanelCorner() override;
		CornerPosition& getSelfPanelCorner() override;
		ImGuiID& getFromWindowId() override;

	private:
		Position mPositionMode = Position::Manual;
		CornerPosition mCornerPosition = CornerPosition::TopLeft;
		ImVec2 mCornerVector;
		CornerPosition mAnchorPanelCornerPosition = CornerPosition::TopLeft;
		CornerPosition mSelfPanelCornerPosition = CornerPosition::TopLeft;
		ImGuiID mFromWindowID = 0;
	};
} // namespace ArcdpsExtension
