FROM alpine:3.22.0@sha256:8a1f59ffb675680d47db6337b49d22281a139e9d709335b492be023728e11715 AS build
WORKDIR /build
RUN apk add --no-cache gcc binutils-gold musl-dev libxml2-dev libxml2-static xz-static zlib-static make
COPY Makefile *.c *.h ./
RUN make LDFLAGS="--static -Wl,--as-needed,-O1,--sort-common" CFLAGS="-Os" LIBFLAGS="$(xml2-config --libs)" && strip mjr

FROM scratch
COPY --from=build /build/mjr /mjr
