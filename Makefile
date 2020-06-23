# Tag on this commit
TAG := $(shell git describe --tags --exact-match)
# Commit hash from git
COMMIT := $(shell git rev-parse --short HEAD)
VERSION := $(if $(TAG),\\\"$(TAG)\\\",\\\"$(COMMIT)\\\")

doxygen:
	export PROJECT_VERSION=$(VERSION) 
	doxygen

astyle:
	astyle --project=".astylerc" \
	      "src/*.c" \
			  "src/*.h"