.POSIX:

.SUFFIXES: .c .o

CC=clang
AR=ar
CFLAGS=-O0 -g
COBJS=lib/ttc-discord-embed.o lib/ttc-discord-ui.o lib/ttc-discord-utils.o lib/ttc-discord-api.o lib/ttc-discord-api-helpers.o lib/ttc-discord-interaction.o lib/ttc-discord-moderation.o lib/ttc-discord-messages.o lib/ttc-discord.o lib/ttc-discord-gateway.o lib/ttc-discord-commands.o lib/ttc-discord-modals.o
TARGET=ttc-discord.so
TARGET_STATIC=ttc-discord.a
EXAMPLE=ttc-bot
EXAMPLESRC=src/main.c src/command.c src/modals.c src/components.c
INCLUDES=-I includes
LIBS=-lttc-http -lttc-ws -lttc-log -lssl -lcrypto -ljson-c

all: $(TARGET) $(TARGET_STATIC) $(EXAMPLE)

.c.o:
	@echo $(CC) $@
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(TARGET): $(COBJS)
	@echo $(CC) $@
	@$(CC) $(CFLAGS) $(INCLUDES) -shared -o $@ $(COBJS)

$(TARGET_STATIC): $(COBJS)
	@echo $(AR) $@
	@$(AR) -rcs  $@ $(COBJS)

$(EXAMPLE): $(TARGET_STATIC) $(EXAMPLESRC)
	@echo $(CC) $@
	@$(CC) $(CFLAGS) $(INCLUDES) -o $@ $(EXAMPLESRC) $(TARGET_STATIC) $(LIBS) #Could this be dynamically linked yes but if this is run before the SO is installed it will fail

uninstall:
	@echo uninstalling

install:
	@echo installing

clean:
	@echo cleaning $(TARGET) $(TARGET_STATIC) $(COBJS)
	@rm ttc-bot log.txt $(TARGET) $(TARGET_STATIC) $(COBJS)

