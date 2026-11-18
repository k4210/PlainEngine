# PlainEngine

PlainEngine is a C++20 engine project organized into separate Visual Studio projects.
The standard library is preferred; no third-party dependencies or rendering
backend are selected yet.

Production projects compile with C++ exceptions and built-in RTTI disabled.
Test projects enable exceptions for GoogleTest compatibility. Engine code uses
non-exception error handling and does not declare functions `noexcept`;
runtime type identification will use a project-defined mechanism when
introduced.
All projects use MSVC warning level 4 and treat warnings as errors.

## Modules

| Module | Responsibility | May depend on |
| --- | --- | --- |
| Core | Shared utilities, math, containers, logging, threading, and coroutines | Standard library |
| Renderer | Rendering system and rendering-related resources | Core |
| Input | Input system | Core |
| Engine | Main loop, scheduling, resource coordination, configuration, and worlds | Core, Renderer, Input |
| Game | Game-specific framework and sample game | Engine |
| Editor | General editor application and content/world tools | Engine |
| GameEditor | Game-specific editor integration | Game, Editor |
| GameLaunch | Executable entry point for running the game | Game |

Game and Editor are independent: neither may depend on the other. Dependencies
are expressed as Visual Studio project references; include paths expose only
the module's permitted dependencies. Keep public headers under `include/` and
implementation files under `src/` within each module.

GameLaunch and GameEditor build as executables; all other projects build as
static libraries. Executable projects reference the static-library dependency
closure so the final link includes each required library. GameLaunch does not
reference Editor or GameEditor.

Each library module has a separate GoogleTest project under `Tests/`:
`CoreTests`, `RendererTests`, `InputTests`, `EngineTests`, `GameTests`, and
`EditorTests`. These test projects build as executables so Visual Studio Test
Explorer can discover and run them; they are the exception to the production
project output rule above. Tests depend on their module and its required
library dependencies, while production projects never depend on tests.

GoogleTest is provided by Visual Studio's Google Test component and referenced
through the shared `Tests/GoogleTest.props` NuGet package reference. Test
executables can be built individually or discovered and run from Test Explorer.

Core logging provides `Log`, `Info`, `Warning`, `Error`, and `Break` with
`std::format`-style arguments and optional `StringId` channels, written as
`"channel"_id`. Messages are formatted into a fixed buffer; oversized messages are truncated.
Conditional variants (`CLog`, `CInfo`, `CWarning`, `CError`, and `CBreak`)
take a leading `bool` and do nothing when it is false.
Warnings use yellow console text; errors and break messages use red. `Break`
flushes its message before triggering a debugger break. Logging is enabled by
default; defining `LOG_ENABLED=0` compiles logging calls and their arguments out.

Core provides a C++20 `Expected<T, E>` result type, with `Unexpected<E>` for
errors. Check `has_value()` or the boolean conversion before accessing
`value()` or `error()`; invalid access is asserted because production code
does not use exceptions. File utilities in `PlainEngine`
use this type for extension-filtered directory iteration and text or binary
reads, carrying failures as `std::error_code`. Their public declarations
contain no platform-specific headers.

`PlainEngine::LinearMap<Key, Value>` stores sorted keys and corresponding
values in parallel vectors, providing ordered insertion and binary-search lookup
for maps with a small number of entries.

`PlainEngine::Config::Read` reads `.ini` files from a relative
folder path and returns a `Config` value. Its `find` method retrieves integer
or floating-point values by filename without its `.ini` extension, section,
and key.

Console variables in `PlainEngine` register with
`ConsoleVariableManager::Get()` at construction and can be initialized with a
`Config` value. Define them with global storage duration and keep them at a
stable address; C++ cannot enforce global storage duration, so the variable
types are non-copyable and non-movable. The manager stores non-owning pointers
and unregisters variables on destruction; lookup pointers are valid only while
their variables are alive. Their values are atomic.

## Naming conventions

- Classes and static functions start uppercase: `GameWorld`, `Initialize`.
- Static `const` and `constexpr` values start with `k` and uppercase: `kMaxPlayers`.
- Member variables start with `_` and lowercase: `_frameCount`.
- Other names start lowercase: `updateWorld`, `playerCount`.

## Build

Open `PlainEngine.slnx` in Visual Studio and build `GameLaunch` to run the game
or `GameEditor` to run the game-specific editor. The projects use C++20 and
the MSVC `v145` toolset configured in the existing workspace.
