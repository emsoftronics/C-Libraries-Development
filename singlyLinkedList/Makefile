
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
CFLAGS  :=  -Wall
#-ansi -pedantic -Wall -Werror -W -g3
LDFLAGS :=  -L$(SLDIR)
LDLIBS  :=  -lpthread
INC		:=  -I$(current_dir)/../ -I.

CC		:= gcc

.PHONY: all clean fclean rebuild c_lib_static c_lib_dynamic c_app

all: c_lib_static c_lib_dynamic c_app

c_lib_static: $(SNAME)

c_lib_dynamic: $(DNAME)

c_app : $(BIN)

$(SNAME): $(SOBJ) | $(SLDIR)
	$(AR) $(ARFLAGS) $@ $^

$(DNAME): CFLAGS += -fPIC
$(DNAME): LDFLAGS += -shared
$(DNAME): $(DOBJ) | $(DLDIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(SOBJ): $(SRC) | $(SDIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c $<

$(DOBJ): $(SRC) | $(DDIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c $<

$(SDIR) $(DDIR) $(SLDIR) $(DLDIR):
	@mkdir -p $@

$(BIN): $(AOBJ) | $(SNAME)
	$(CC) $(INC) $(CPPFLAGS) $(CFLAGS) $(LDLIBS) $(LDFLAGS) -o $@ $< -l$(NAME)

$(AOBJ): $(APP_SRC)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c $<

clean:
	$(RM) -rf $(SDIR) $(DDIR)
	$(RM) -f $(BIN)
	$(RM) -f $(AOBJ)

print:
	@echo "SLDIR : $(SLDIR)"
	@echo "DLDIR : $(DLDIR)"
	@echo "APP_SRC : $(APP_SRC)"
	@echo "ALL_SRC : $(ALL_SRC)"
	@echo "SRC : $(SRC)"
	@echo "SOBJ : $(SOBJ)"
	

fclean: clean
	$(RM) $(SNAME) $(DNAME)

rebuild: fclean all

