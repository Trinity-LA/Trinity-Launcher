
# Build image with docker-compose
``` docker-compose build ```

# Run image with docker-compose
``` docker-compose run --rm proyecto  ``` 
# Build Trinity
``` sh build.sh --clean --release ```  

# Build image without docker-compose

``` docker build \
  --build-arg USER_ID=$(id -u) \
  --build-arg GROUP_ID=$(id -g) \
  -t trinity .
``` 

# Run image without docker-compose and build trinity
```
docker run --rm -it \
  -v "$(pwd)":/project \
  trinity \
  ./build.sh --clean --release
``` 
