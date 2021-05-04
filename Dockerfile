FROM alpine:3.13 as build
WORKDIR /build
RUN apk add --no-cache gcc binutils-gold musl-dev libxml2-dev zlib-static make
COPY Makefile *.c *.h ./
RUN make LDFLAGS="--static -Wl,--as-needed,-O1,--sort-common" CFLAGS="-Os" LIBFLAGS="$(xml2-config --libs)" && strip mjr

FROM scratch
COPY --from=build /build/mjr /mjr
