#!/usr/bin/env bash

# docker run: This is the command to start a new Docker container from an image.
# --rm: This flag tells Docker to remove the container automatically after it exits. 
#       This helps in cleaning up containers that are no longer needed.
# -it: These are two flags combined:
#     -i stands for interactive, which allows you to interact with the container's shell.
#     -t allocates a pseudo-TTY, or terminal, for the container, allowing you to see the 
#        command-line output and interact with it as if it were a local terminal.
# --platform linux/amd64: This flag specifies the platform architecture for the container. 
#                         In this case, it's set to "linux/amd64," indicating that the container 
#                         should run on a 64-bit x86 architecture.
# --workdir /builder/mnt: This flag sets the working directory inside the container to "/builder/mnt." 
#                         Any commands executed within the container will start from this 
#                         directory unless specified otherwise.
# -v .:/builder/mnt: This flag is used for volume mounting. It maps the current directory on your 
#                    local host (specified as .) to the "/builder/mnt" directory inside the container. 
#                    This allows you to share files and data between your host machine and the container.
# my-builder:latest: This is the name of the Docker image that you want to run. In this case, 
#                    it's "my-builder" with the "latest" tag.
# /bin/bash: This is the command that will be executed inside the container. It starts a Bash shell,
#            which gives you an interactive terminal session within the container.

docker run --rm \
    -it \
    --platform linux/amd64 \
    --workdir /builder/mnt \
    -v .:/builder/mnt \
    my-builder:latest \
    /bin/bash