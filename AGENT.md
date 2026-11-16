# Agent guidance

- Be concise and critical; point out concrete problems. If work finishes without issues, reply only `Done`.
- Try to change as little as possible.
- Take actions only when explicitly asked.
- Follow `README.md` for module responsibilities and dependency boundaries.
- Naming: types and static functions use PascalCase; static constants use `kUpperCamelCase`; members use `_lowerCamelCase`; other names use lower camelCase.
- Make functions and constructors `constexpr` whenever possible.
- Avoid introducing preprocessor macros.
- Production code must not use exceptions, `noexcept`, or built-in RTTI. Test projects may enable exceptions for GoogleTest.
- Prefer standard-library facilities. Avoid introducing dependencies, backends, or architecture choices without a clear requirement.
- Use local lambdas for small helpers that capture local state.
- Append to buffers with post-increment indexing, such as `buffer[length++] = value`.
- Keep locks scoped to shared-state operations; prepare data before locking and release locks before debugger breaks.
- Omit whitespace-only changes when showing diffs.
- Keep Visual Studio project entries and documentation in sync with structural changes. Add tests for behavior when appropriate.
