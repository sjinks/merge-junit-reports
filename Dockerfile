FROM alpine:3.24.4@sha256:f59abe656b692d901e184be3445e084786602108599d2a51d229c0f5a48c71bf AS build
WORKDIR /build
RUN apk add --no-cache gcc binutils-gold musl-dev libxml2-dev libxml2-static xz-static zlib-static make
COPY Makefile *.c *.h ./
RUN make LDFLAGS="--static -Wl,--as-needed,-O1,--sort-common" CFLAGS="-Os" LIBFLAGS="$(xml2-config --libs)" && strip mjr

FROM scratch
COPY --from=build /build/mjr /mjr
