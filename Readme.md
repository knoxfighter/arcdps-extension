# Arcdps Extension

This is a repository with common classes and utilities to use with Arcdps and Imgui.

You will find documentation in the header files and doxygen-generated in the [GitHub pages](https://knoxfighter.github.io/arcdps-extension/).

## Usage

### add to your project

Make sure you call proper Setup() and Shutdown() functions in the corresponding classes.
This is especially true with referenced and inherited classes.
If you want to have everything automatically setup and shutdown, call `ArcdpsExtension::Setup()` and `ArcdpsExtension::Shutdown()`.

#### vcpkg

arcdps-extension is available to vcpkg with the custom registry found at https://github.com/Zinn-o-Matics/vcpkg-registry/.
Usage of the registry can be found there.

#### git submodule

Clone the project as git submodule into yours. Then you have to set up your Cmake, so it includes this project.
Alternatively you can also put every used file in your own project (doesn't matter in CMake or visual studio project).

### Use-cases

#### Update your plugin

You can have popups to download new versions like the killproof.me and boon-table addon.
Use [UpdateChecker](UpdateChecker.h) to do so, this will come with an ImGui UI.
If you want to create your own UI, use the baseclass [UpdateCheckerBase](UpdateCheckerBase.h) directly.

#### Loading Icons into DX11

Loading Icons from different Sources is quite some code.
I created the class [IconLoader](IconLoader.h) to do that for you.
It supports loading from different sources.
One of which is the website `gw2dat.com`, so you can load icons from the dat directly from a website.
You are also not limited to loading Icons, every Texture is supported.

Example Usage can be found in the corresponding tests: [IconLoaderTests](IconLoaderTests.cpp).

#### Download Network resources

Previously i used `cpr` as curl wrapper.
That is a huge project which unfortunately had many problems and broke from time to time.
I now have my own curl wrapper to make network calls, it is also able to download files: [SimpleNetworkStack](SimpleNetworkStack.h).
Usage is quite simple, example usage in [SimpleNetworkStackTests](SimpleNetworkStackTests.cpp) and [IconLoader](IconLoader.cpp).

#### arcpds event handling

A common problem is the bad design choices made to create the arcpds API.
I've created classes to get around two of the main issues:
The API events are all sent with the callback and the way to identify each one it quite complicated.
My solution is the [CombatEventHandler](CombatEventHandler.h) that takes the events, and then calls a different function for each identified event.    
Another issue is, that arcdps events are called out of order.
To sort them again i created the class [EventSequencer](EventSequencer.h).
This class is also used by the CombatEventHandler.
It also runs the actual events in a separate thread.

#### Translations

Another common issue in software is are translations
For that I created the class [Localization](Localization.h).
It is not a complete localization unit with pluralisation and more.
It is simply a class to make basic translations easier.
Translations used by other classes in here are already done and can be found in [ExtensionTranslations.h](ExtensionTranslations.h).

#### Handling Keybinds

Windows and Guild Wars 2 have two different ways to handle keybinds, both are quite weird.
I made a [KeyBindHandler](KeyBindHandler.h) where you can subscribe to specific keybinds.
There is also a [KeyInput](KeyInput.h) if you want to have an ImGui popup for settings keybinds.

## Dependencies

### Unofficial Extras

Some classes use the public part of unofficial extras. It is Licensed under MIT and is found on [GitHun](https://github.com/Krappa322/arcdps_unofficial_extras_releases/)

### ImGui

[Project](https://github.com/ocornut/imgui) Licensed under MIT. Based on Version [1.80](https://github.com/ocornut/imgui/tree/v1.80).

### magic-enum

[Project](https://github.com/Neargye/magic_enum) Licensed under MIT.

### nlohmann-json

[Project](https://github.com/nlohmann/json/) Licensed under MIT.

### CURL

[Project](https://curl.se/) Licensed under [MIT-inspired License](https://curl.se/docs/copyright.html).

### googletest

[Project](https://github.com/google/googletest) Licensed under BSD-3-Clause. Only used in Tests.
