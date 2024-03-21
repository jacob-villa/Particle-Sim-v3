# Particle Simulator
This particle simulator provides a visualization for adding 2-dimensional particles onto a 1280 x 720 pixel black canvas. Collisions with walls or the edges of the canvas induce elastic collisions and reflect the particle's angle accordingly. As well as a sprite that users are able to move. 

### There are 2 modes that the user can switch to:
1. Developer Mode
2. Explorer Mode

## Developer Mode
Users are able to specify the position, angle, and velocity of particles added to the canvas. Particles can also be added in batches with varying starting positions, angles, or velocities. A sprite is also seen at the canvas screen indicating the position of the user for the Explorer Mode.

## Explorer Mode
The users will be presented with a zoomed-in perspective representing a 19 x 33 space in the screen. The aforementioned sprite represents the user's "avatar", which can move around the canvas using the "WASD" keys or the arrow keys in the keyboard.

# Running the Application

### Using the Executable File

1. Clone this repository and pull.
2. Access the precompiled executable file in the x64 Debug folder.

### Using Visual Studio
1. Open Visual Studio (at least version 2019) and select "Open a project or solution."
2. Select the "Particle-Sim.sln" file and VS 2022 should open the code in the IDE itself. To view the source code, open the Solution Explorer and navigate to the "Source Files" folder. The "Particle-Sim.cpp" source code can be selected to view the codebase.
5. Click the play button "Local Windows Debugger" and ensure the build configuration is set to Debug and x64 architecture.
