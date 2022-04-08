FROM debian:stable

RUN apt-get update
RUN apt-get install -y dumb-init
RUN apt-get install -y make
RUN apt-get install -y binutils-avr
RUN apt-get install -y gcc-avr
RUN apt-get install -y avr-libc
RUN apt-get install -y git
RUN mkdir -p /home/dev
WORKDIR /home/dev
CMD ["/bin/bash", "-c", "git clone https://github.com/wdl83/bootloader; cd bootloader; ./build.sh"]
ENTRYPOINT ["dumb-init", "--"]
