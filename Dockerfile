FROM alpine:3.17@sha256:36a03c95c2f0c83775d500101869054b927143a8320728f0e135dc151cb8ae61 as build
WORKDIR /build
RUN apk add --no-cache gcc binutils-gold musl-dev libxml2-dev zlib-static make
COPY Makefile *.c *.h ./
RUN make LDFLAGS="--static -Wl,--as-needed,-O1,--sort-common" CFLAGS="-Os" LIBFLAGS="$(xml2-config --libs)" && strip mjr

FROM scratch
COPY --from=build /build/mjr /mjr
