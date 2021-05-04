TARGET   = mjr
C_SRC    = mjr.c
C_DEPS   = $(patsubst %.c,%.d,$(C_SRC))
OBJS     = $(patsubst %.c,%.o,$(C_SRC))
LIBFLAGS = $(shell xml2-config --libs --static)
LDFLAGS  =
override CPPFLAGS += $(shell xml2-config --cflags)

all: $(TARGET)

ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif

mjr: $(OBJS)
	$(CC) $^ $(LIBFLAGS) $(LDFLAGS) -o $@

%.o: %.c
	$(CC) $(CPPFLAGS) -fvisibility=hidden -Wall -Wextra -Werror -std=gnu99 $(CFLAGS) -c "$<" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@"

clean: objclean depclean
	-rm -f $(TARGET)

objclean:
	-rm -f $(OBJS)

depclean:
	-rm -f $(C_DEPS)

dockerbuild:
	DOCKER_BUILDKIT=1 docker build --progress=plain --output type=local,dest=./ .

static-analysis: clean
	scan-build $(MAKE) $(MAKEFLAGS) $(TARGET)

.PHONY: clean objclean depclean dockerbuild static-analysis
