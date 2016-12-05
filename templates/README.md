# {{project_name}}

{{project_name}} is a JavaScript-based g-speak application.  It's designed to
work with [NPM](https://www.npmjs.com/) (the [Node.js](https://nodejs.org/)
package manager), and [obi](https://github.com/oblong/obi), Oblong's g-speak
command line tool.

To run this project, run `npm run-script obi-go` from the root directory of the project.

## About

*TBD:* This is where you can describe your project for other developers

## How Javascript code gets built

This project

## The package.json file

## The project.yaml file

[project.yaml](project.yaml) contains all the information for building and
running your greenhouse application, if you use `obi` as your project runner
tool.  It lets you specify launch options such as command arguments,
environment vars, etc.

When running on your own machine (`obi go`), obi uses information from the
`localhost` room in [project.yaml](project.yaml).

See `obi --help` for more details.


## Building

{{project_name}} uses [CMake](https://cmake.org/) for it's build system.  Obi
handles building for you, but you can always use CMake directly if you prefer.

```shell
$ mkdir build
$ cd build
$ cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -DMY_CUSTOM_DEFINE .. # If you use Ninja for example
$ ninja -j8
```

Obi and CMake can be used interchangably.  Obi assumes an out-of-tree build in
a directory called `build`.


## Sublime Text Users

This project has a [sublime-project file]({{project_name}}.sublime-project)
that comes equipped with some useful build systems:

- `obi-build`: just build the app
- `obi-go`: build and run the app
- `obi-go:my-room`: build and run the app in your custom room *NOTE:* Requires
  custom setup in your project.yaml


