# Parallel Rubik's Cube Solver

Rubik's cube solver project in C. The current version contains only the flat
facelet engine; search, OpenMP, and MPI integration come later.

## Build and run

```sh
make build
make run
```

## Dev container

### Prerequisites

- [Docker](https://www.docker.com/products/docker-desktop/)
- [VS Code](https://code.visualstudio.com/) with the [Dev Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) extension
- **Windows only:** [WSL2](https://learn.microsoft.com/en-us/windows/wsl/install) (Docker Desktop uses it as its backend)

### Open the container (VS Code)

1. Open this folder in VS Code.
2. Run **Dev Containers: Reopen in Container** from the Command Palette
   (`F1` / `Ctrl+Shift+P`). VS Code builds the image (first time only) and
   drops you into a terminal inside Ubuntu 24.04.
3. Build and run the same way as above.

### Open the container (JetBrains IDE)
**Prerequisites:** Docker (running)
1. From the JetBrains **Welcome** screen, choose **Remote Development → Dev
   Containers → New Dev Container**.
2. Select **From Local Project** (or point it at this repo) and pick the
   `.devcontainer/devcontainer.json` file in this folder.
3. Click **Build Container and Continue**. JetBrains builds the image (first
   time only) and installs its backend inside the container.
4. When the build finishes, select **CLion** (or your IDE) as the backend and
   click **Continue** — a JetBrains Client window opens connected to the
   container.
5. Open the built-in terminal (the shell runs inside Ubuntu 24.04) and build:

   ```sh
   make build
   make run
   ```

Alternatively, if you already have the project open in a JetBrains IDE, click
the dev container badge next to the `devcontainer.json` file in the editor and
choose **Create Dev Container**.
