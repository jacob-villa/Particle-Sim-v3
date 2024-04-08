# Distributed Particle Simulator
This particle simulator provides a visualization for adding 2-dimensional particles onto a 1280 x 720 pixel black canvas. Collisions with walls or the edges of the canvas induce elastic collisions and reflect the particle's angle accordingly, including an explorer sprite that users can control as they navigate the canvas. The simulator also includes capabilities for a server that hosts all connected explorer clients and particles while also allowing the addition of particles. Clients are able to navigate the canvas through a zoomed-in explorer-view periphery that allows them to see particles and other clients.

## Developer Server
The server is capable of adding particles by individually specifying their parameters (x, y, angle, velocity), adding a random particle, and adding particles by batch. The server also hosts all created particles and explorers representing connected clients.

## Explorer Client
The explorer client is able to navigate the canvas and see existing particles and other existing clients that come within its zoomed-in periphery.

# Usage Instructions (Visual Studio)
- Download the libraries used through this [link](https://drive.google.com/file/d/1WYh7ZnrMbhtwgByEr5ZQEsxh3n9TsQEk/view?usp=drive_link).
- Extract the folders of the libraries in the project directory.
- Configure Visual Studio settings to include the respective Boost libraries. This involves including the 'vendor include' and 'Boost' folders, including the Boost libraries folder and the vendor library directory, and configuring the Linker to include the additional dependencies (OpenGL32.lib, glfw3.lib, and the core library dependencies). 

# Running the Application

### Running the Server
1. Open Visual Studio (at least version 2019) and open the solution file of the project.
2. Select the Configuration Manager and create a new configuration. Name the configuration for the server accordingly.
3. Select the project folder in the solutions explorer and configure the properties to include the necessary libraries and dependencies as detailed previously.
4. Exclude the 'Explorer-Client.cpp' file from the build to prevent it from compiling and to only compile the 'Dev-Server.cpp' file.
5. Build the 'Dev-Server.cpp' file with the corresponding configuration.

### Running the Client:
1. Follow steps 1-3 for running the server, but name the configuration accordingly for the client.
2. Follow step 4 for running the server, but instead exclude the server from building to only build the client file.
3. Build the 'Explorer-Client.cpp' file with a running server.

