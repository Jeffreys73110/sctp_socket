b64 := true	# comment this line as 32 bit
# -m32 bits or -m64 bits
ifdef b64
	BITS := -m64
	OS_BIT := 64
else
	BITS := -m32
	OS_BIT := 32
endif

# if make debug=1 than debug mode
ifndef debug
	DEBUG := -D NDEBUG
endif

CC	= g++ -std=c++11

# variables
DIR_BUILD := build
DIR_SRC := src
DIR_MOD := $(DIR_SRC)/module
DIR_DEV := $(DIR_SRC)/dev
DEV_LINK_TARGET := sctp_socket
SYS_LIBS := -lpthread -lsctp

## modules
MOD_OBJS := 
MOD_OBJS += ${DIR_BUILD}/sctp_socket.o

## dev
DEV_OBJS := $(MOD_OBJS)
DEV_OBJS += $(DIR_BUILD)/sctp_dev.o


dev : clean init $(DEV_OBJS)
	$(CC) $(BITS) -Wl,--gc-sections -o $(DEV_LINK_TARGET) $(DEV_OBJS) $(SYS_LIBS)
	@echo All done

init :
	mkdir -p build

## dev
$(DIR_BUILD)/%.o : $(DIR_DEV)/%.cpp $(DIR_DEV)/%.h
	$(CC) $(BITS) -c $< -o $@

## modules
$(DIR_BUILD)/%.o : $(DIR_MOD)/sctp_client/%.cpp $(DIR_MOD)/sctp_client/%.h
	$(CC) $(BITS) -c $< -o $@
$(DIR_BUILD)/%.o : $(DIR_MOD)/sctp_socket/%.cpp $(DIR_MOD)/sctp_socket/%.h
	$(CC) $(BITS) -c $< -o $@


.phony: clean
clean:
	-rm $(DIR_BUILD)/*.o $(DEV_LINK_TARGET)