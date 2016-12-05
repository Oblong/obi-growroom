# {{project_name}}

{{project_name}} is a JavaScript-based g-speak application.  It's designed to
work with [NPM](https://www.npmjs.com/) (the [Node.js](https://nodejs.org/)
package manager), and [obi](https://github.com/oblong/obi), Oblong's g-speak
command line tool.

To run this project, run `npm run-script obi-go` from the root directory of the project.

## About

*TBD:* This is where you can describe your project for other developers

## Entry Point

For adding new C++ code the entry point is [`src/main.cpp`](src/main.cpp) in
the function `Setup ()`.

For adding new JavaScript code the entry point is
[`scripts/index.js`](scripts/index.js).  This file is compiled to
`scripts/index-bundled.js`, which is where the app looks for JavaScript, but
you shouldn't need to edit that file.

It's recommended that you keep all C and C++ sources in `src/` and all
JavaScript sources in `scripts/`.  It's not required to keep Node.js and
g-speak JavaScript sources separate, but for large projects that might help
keep the project organized.

## How JavaScript code gets built

This project uses [browserify](http://browserify.org/) to build a single JS
source file that includes all `required` dependencies.  The browserify build
needs to be run before code is run on the target system (in both normal
deployment *and* live-coding), and requires both Node.js and NPM to run.
Based on these requirements it's ideal (but not required) to have the JS build
completed on a development machine instead of the deployment system.  To
orchestrate the steps for deployment there are NPM scripts wrapping each `obi`
command.

- `npm run-script obi-build`
- `npm run-script obi-go`
- `npm run-script obi-stop`
- `npm run-script obi-clean`

These should be used instead of the bare `obi` commands, however they do use
`obi` under the hood.

**NB:** `npm run` is an alias for `npm run-script`, so you can use `npm run
obi-go` if you prefer.

## The package.json file

[package.json](package.json) has the JavaScript specific dependencies and
configuration for this project.  The main sections of interest are:

### `devDependencies`

These are the dependencies required for building the project, but not for
running the project.  The project is configured to use
[browserify](http://browserify.org/) and
[watchify](https://github.com/substack/watchify) for AMD/CommonJS modules, but
these can be changed if a different build system is desired.

### `dependencies`

These are the run-time JS dependencies that can be `require`d into your app.
By default we include:

- [javlin-pure](https://github.com/aeoril/javlin-pure), a vector math library

- [MoOx](https://github.com/MoOx/color), a color library

- [lodash](https://lodash.com/), utility library for collections and
  functional programming

- [pocket-physics](https://github.com/kirbysayshi/pocket-physics), a physics
  library

- [perceive](https://github.com/AndreasMadsen/perceive), a code inspection
  library

- [sprintf-js](https://github.com/alexei/sprintf.js), sprintf for JavaScript

- [squat](https://github.com/Oblong/squat), our quaternion library
    
### `scripts`

This is where the `obi` commands are wrapped and allows for custom invocations
of the building and running of this project.

## The project.yaml file

[project.yaml](project.yaml) contains all the information for building and
running your greenhouse application, if you use `obi` as your project runner
tool.  It lets you specify launch options such as command arguments,
environment vars, etc.

When running on your own machine (`obi go` or `npm run-script obi-go`), obi
uses information from the `localhost` room in [project.yaml](project.yaml).

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

Obi and CMake can be used interchangeably.  Obi assumes an out-of-tree build in
a directory called `build`.


## Sublime Text Users

This project has a [sublime-project file]({{project_name}}.sublime-project)
that comes equipped with some useful build systems (these invoke the `npm`
wrappers of the underlying `obi` commands):

- `obi-build`: just build the app
- `obi-go`: build and run the app
- `obi-go:my-room`: build and run the app in your custom room *NOTE:* Requires
  custom setup in your project.yaml

## Live Coding

