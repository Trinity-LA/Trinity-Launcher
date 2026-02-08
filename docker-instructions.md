# 1. Using Compose

## Build image

```
UID=$(id -u) GID=$(id -g) docker compose build
```

## Run image

```
UID=$(id -u) GID=$(id -g) docker compose run --rm trinity
```

# 2. Using dockerfile

## Build image

```
docker build \
  --build-arg UID=$(id -u) \
  --build-arg GID=$(id -g) \
  -t trinity-launcher .
```

## Run image

```
docker run --rm -it \
  --user $(id -u):$(id -g) \
  -e DISPLAY=$DISPLAY \
  -v "$(pwd)":/project \
  -v cargo-cache:/home/trinity/.cargo/registry \
  trinity-launcher \
  /bin/bash
```

# Building Trinity launcher

```
./build.sh --clean --release
```

