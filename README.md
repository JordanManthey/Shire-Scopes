# Shire-Scopes
Shire Scopes is a multiplayer FPS game made with Unreal Engine 5.0 for PC. It's fully integrated with the Steam Online Subsystem and features matchmaking, custom lobbies, and leaderboards. Supports both controller and mouse/keyboard. Available for free on Steam and Itch.io! 

This repository contains all of the C++ and blueprint files for this project. There is no development history here since I used Perforce for version control instead. Most blueprints are uploaded to a visualization webtool [here](https://blueprintue.com/profile/jordanmanthey/), which allows you to see the top-level node layout of each blueprint.

You can also find a high-level UML diagram [here](https://drive.google.com/file/d/18IXymCuSL17X7kTAPDWl6hR5QvQzk5jk/view?usp=sharing) to represent the system of most C++ and blueprint classes. This exludes lower-level classes such as widgets and behavior tree tasks as to not saturate the diagram.

------------------

Prefixes used in blueprints:
BP = blueprint,
ABP = animation blueprint,
WBP = widget blueprint,
BTT = behavior tree task,
BTS = behavior tree service

*Note: BlueprintUE does not support Behavior Trees nor Animation Graphs. This is an unofficial tool and does not visualize everything perfectly.*
