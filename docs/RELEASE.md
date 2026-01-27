# Release Checklist

1. Ensure CI green on main (Linux/macOS build + tests).
2. Format code: `./scripts/format.sh` and `pre-commit run --all-files` if installed.
3. Build release preset: `cmake --preset release` then `cmake --build --preset release`.
4. Run tests:
   - C++: `ctest` in `build/release` (or `build/debug`).
   - Go: `cd coordinator && go test ./...`.
5. Docker images:
   - Coordinator: `docker build -f Dockerfile.coordinator -t <org>/sumo-coordinator:latest .`
   - Server: `docker build -f Dockerfile.server -t <org>/sumo-server:latest .`
6. Tag and push: `git tag vX.Y.Z && git push origin vX.Y.Z`.
7. Publish artifacts (optional): upload server/coordinator binaries or images to registry.
