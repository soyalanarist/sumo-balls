# Dev Workflow

1. Bootstrap deps and build:
   - `./scripts/bootstrap.sh` (macOS or Debian/Ubuntu) or use CMake presets: `cmake --preset debug` then `cmake --build --preset debug`.
2. Run services:
   - Coordinator: `./scripts/run-coordinator.sh`
   - Server: `./scripts/run-server.sh 7777`
   - Client: `./scripts/run-client.sh`
3. Formatting:
   - `./scripts/format.sh` (clang-format C++, gofmt Go). Use `pre-commit install` to auto-run on commit.
4. Tests:
   - C++: `cmake --build build/debug --target test` (or `ctest` inside build dir).
   - Go: `cd coordinator && go test ./...`.
5. Docker (optional): `docker compose up coordinator` (+ `--profile server` for server) and `docker compose up seed` for sample users.
