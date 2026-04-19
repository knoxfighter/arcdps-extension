ImGui upgrade steps:
- Copy TreeNodeBehavior() from upstream ImGui and re-add the code related to icons.
- Copy TableHeader() from upstream ImGui and re-add the code related to show label and texture.
- Copy ProgressBar() from upstream ImGui into AlignedProgressBar() and re-add the code related to alignment.
- Copy BeginMenu() from upstream ImGui and re-add the code related to.
- Copy the bottom part of TableDrawContextMenu() from upstream ImGui to MenuItemTableColumnVisibility().
