FROM alpine:3.22.1@sha256:4bcff63911fcb4448bd4fdacec207030997caf25e9bea4045fa6c8c44de311d1 AS build
WORKDIR /build
RUN apk add --no-cache gcc binutils-gold musl-dev libxml2-dev libxml2-static xz-static zlib-static make
COPY Makefile *.c *.h ./
RUN make LDFLAGS="--static -Wl,--as-needed,-O1,--sort-common" CFLAGS="-Os" LIBFLAGS="$(xml2-config --libs)" && strip mjr

FROM scratch
COPY --from=build /build/mjr /mjr
