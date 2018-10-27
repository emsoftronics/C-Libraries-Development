

DEPLIBS  := 

LDLIBS  :=  -pthread

CC		:= gcc
CFLAGS  :=  -Wall 

mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
current_dir := $(dir $(mkfile_path))
NAME    :=  $(notdir $(patsubst %/,%,$(dir $(mkfile_path))))

# $(shell echo $(current_dir) | rev | cut -d'/' -f1 | rev )
BIN		:=  $(current_dir)/$(NAME)_app
APP_SRC :=	$(shell grep -rs "int main" $(current_dir)/*.c | cut -d':' -f1 )
AOBJ    :=  $(APP_SRC:$(current_dir)/%.c=$(current_dir)/%.o)
SLDIR	:=  $(current_dir)/../static-lib
SNAME   :=  $(SLDIR)/lib$(NAME).a
DLDIR	:=  $(current_dir)/../shared-lib
DNAME   :=  $(DLDIR)/lib$(NAME).so
ALL_SRC :=  $(wildcard $(current_dir)/*.c)
SRC		:=	$(filter-out $(APP_SRC), $(ALL_SRC))
SDIR    :=  $(current_dir)/sobj
SOBJ    :=  $(SRC:$(current_dir)/%.c=$(SDIR)/%.o)
DDIR    :=  $(current_dir)/dobj
DOBJ    :=  $(SRC:$(current_dir)/%.c=$(DDIR)/%.o)
#-ansi -pedantic -Wall -Werror -W -g3
SLDFLAGS :=  -L$(SLDIR)
DLDFLAGS :=  -L$(DLDIR)
INC		:=  -I$(current_dir)/../ -I.

DDEPLIBS := $(DEPLIBS:-l%=$(DLDIR)/lib%.so)
SDEPLIBS := $(DEPLIBS:-l%=$(SLDIR)/lib%.a)
LIBDIRS := $(DEPLIBS:-l%=$(current_dir)/../%)

.PHONY: all clean fclean rebuild c_lib_static c_lib_dynamic c_app

all: c_lib_static c_app

c_lib_static: $(SNAME)

c_lib_dynamic: $(DNAME)

c_app: $(BIN)

$(SNAME): $(SOBJ) | $(SLDIR)
	$(AR) $(ARFLAGS) $@ $^

$(DNAME): CFLAGS += -fPIC
$(DNAME): DLDFLAGS += -shared
$(DNAME): $(DOBJ) | $(DLDIR) $(DDEPLIBS)
	$(CC) $(DLDFLAGS) $^ $(LDLIBS) $(DEPLIBS) -o $@

$(SOBJ): $(SRC) | $(SDIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c $<

$(DOBJ): $(SRC) | $(DDIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c $<

$(SDIR) $(DDIR) $(SLDIR) $(DLDIR):
	@mkdir -p $@

$(DDEPLIBS): $(LIBDIRS)
	@make -C $< c_lib_dynamic
	

$(BIN): $(AOBJ) | $(SNAME) $(SDEPLIBS)
	$(CC) $(INC) $(CPPFLAGS) $(CFLAGS) $(LDLIBS) $(DEPLIBS) $(SLDFLAGS) -o $@ $< -l$(NAME)

$(AOBJ): $(APP_SRC)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c $<

$(SDEPLIBS): $(LIBDIRS)
	@make -C $< c_lib_static

$(LIBDIRS):
	@echo "$@ : No such file or directory."

clean:
	$(RM) -r $(SDIR) $(DDIR)
	$(RM) $(BIN)
	$(RM) $(AOBJ)

print:
	@echo "SLDIR : $(SLDIR)"
	@echo "DLDIR : $(DLDIR)"
	@echo "APP_SRC : $(APP_SRC)"
	@echo "ALL_SRC : $(ALL_SRC)"
	@echo "SRC : $(SRC)"
	@echo "SOBJ : $(SOBJ)"
	@echo "DEPLIBS : $(DEPLIBS)"
	@echo "DDEPLIBS : $(DDEPLIBS)"
	@echo "SDEPLIBS : $(SDEPLIBS)"
	@echo "LIBDIRS : $(LIBDIRS)"
	

fclean: clean
	$(RM) $(SNAME) $(DNAME)

rebuild: fclean all

