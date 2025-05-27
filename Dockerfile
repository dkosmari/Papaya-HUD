# FROM devkitpro/devkitppc
# FROM ghcr.io/wiiu-env/devkitppc:20241128
FROM dkosmari/devkitppc-wiiu-alpine

COPY --from=ghcr.io/wiiu-env/libbuttoncombo:20250127 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libmappedmemory:20250204 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libnotifications:20250204 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/wiiupluginsystem:20250208 /artifacts $DEVKITPRO

COPY --chown=user . .
