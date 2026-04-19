ImGui upgrade steps:
- Copy TreeNodeBehavior() from upstream ImGui and re-add the code related to icons. Add ImGui:: prefix to every ImGui method.
- Copy TableHeader() from upstream ImGui and re-add the code related to show label and texture. Add ImGui:: prefix to every ImGui method.
- Copy ProgressBar() from upstream ImGui into AlignedProgressBar() and re-add the code related to alignment. Add ImGui:: prefix to every ImGui method.
