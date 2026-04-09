FROM devkitpro/devkitppc

RUN apt-get install -y automake libtool
RUN dkp-pacman -Syu --noconfirm

WORKDIR /project
COPY . /project
