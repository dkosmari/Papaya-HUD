#!/bin/bash

cleanup()
{
    echo "Cleaning up Docker..."
    if test x$CONTAINER != x
    then
        docker container rm --force $CONTAINER
    fi
    if test x$IMAGE != x
    then
        docker image rm --force $IMAGE
    fi
    echo "You may also want to run 'docker system prune --force' to delete Docker's caches."
    exit $1
}
trap cleanup INT TERM

IMAGE=plugin-builder-image
docker build --tag $IMAGE . || cleanup 1

CONTAINER=plugin-builder-container
ARGS="--tty --interactive --name $CONTAINER $IMAGE"
docker run $ARGS sh -c "./bootstrap && ./configure --host=powerpc-eabi CXXFLAGS='-O2 -ffunction-sections -fipa-pta' && make" || cleanup 2
echo "Compilation finished."

# Copy the wps file out.
docker cp "$CONTAINER:/project/src/papaya-hud.wps" .

cleanup 0
