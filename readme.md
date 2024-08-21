# Active Object Pattern

- This repository show a C++ implementation of the Active Objects (Actors) pattern as is described in the book "Practical UML Statecharts in C/C++: Event-Driven Programming for Embedded Systems - Miro Samek".

- In this pattern, Active Objects (Actors) are event-driven, strictly encapsulated software objects running in their own threads of control that communicate with one another asynchronously by exchanging events.

## How to operate the repository
- If you wish to use docker to operate the repository, build the image and launch it using the helper scripts inside of the `docker` folder
- The repository can be operated outside of the docker container if all the dependencies are met
- Once the environment is set (whether inside or outside the container), the following commands can be issued:

- To format the code base with clang-format:
```bash
./bbuild.sh -f
```

- To perform an static analysis in the code base with clang-tidy:
```bash
./bbuild.sh -s
```

- In the following examples, `<target>` is either `app` or `test`

- To build:
```bash
./bbuild.sh -b <target>
```

- To rebuild:
```bash
./bbuild.sh -r <target>
```

- To execute the built binary:
```bash
./bbuild.sh -e <target>
```

- To format, analyze, rebuild and execute with verbose turned ON:
```bash
./bbuild.sh -v -f -s -r -e <target>
```

- Examples:
    - `./bbuild.sh -v -f -s -r -e app`
    - `./bbuild.sh -v -f -s -r -e test`

- To check all options available::
```bash
./bbuild.sh --help
```