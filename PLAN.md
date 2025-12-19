# Plan: Port src from C99 to C23 (and keep build system working)

## Scope
- Source code: `src/em1.c`, `src/em2.c`.
- Build system: `configure.ac`, `configure`, `Makefile.am`, `Makefile.in`, `src/Makefile.am`, `src/Makefile.in`, `doc/Makefile.in`.

## Steps
1. Code audit for C23 incompatibilities in `src/`.
   - Find and fix non-standard `main` signatures (e.g., `void main` -> `int main` with `return 0`).
   - Replace K&R-style/old-style function declarations (e.g., `int (*f)()` and empty parameter lists) with full prototypes.
   - Remove or rename macros that collide with standard headers (e.g., `EOF`, `SIGHUP`, `SIGQUIT`), and rely on the standard definitions when appropriate.
   - Verify all functions are declared before use (no implicit int / implicit declarations).
   - Confirm C23 type requirements (`size_t`, `ssize_t`, `sigjmp_buf`, etc.) have correct headers and consistent usage.

2. Update source to C23 style and safety.
   - Make function pointer signatures explicit (e.g., `int (*f)(void)` or `int (*f)(int)` as needed).
   - Normalize function prototypes in both files so they match definitions.
   - Adjust any legacy constructs that are obsoleted or removed in C23 (e.g., K&R function definitions if present).
   - Keep behavior unchanged; limit refactors to compatibility and warning fixes.

3. Update Autoconf/Automake to request C23.
   - Replace deprecated `AC_PROG_CC_STDC` with `AC_PROG_CC`.
   - Add a compile check to prefer `-std=c23`, then fall back to `-std=gnu23` (or `-std=c2x` if needed), and set `CFLAGS`/`AM_CFLAGS` accordingly.
   - Regenerate `configure` and `Makefile.in` files with `autoreconf -fi` so the distributed scripts reflect the C23 change.

4. Verify build and runtime basics.
   - Run `./configure` and `make` with the new flags.
   - Smoke-test `./src/em` with a small sample file to ensure no regressions in basic commands.

## Deliverables
- Updated `src/*.c` C23-compatible source.
- Updated Autotools inputs and regenerated outputs (configure/Makefile.in).
- A short note in `README` or `NEWS` (optional) indicating the C23 port and any build flag requirements.

## Feature branches
- `feat/o-editor-visual-backspace`: add visual feedback when deleting a character in `o` raw line editor.
