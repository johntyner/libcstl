QUIET	:= @

CC	:= gcc
CFLAGS	:= -MMD -Wall -Wextra -Wno-unused-function -std=c99 -pedantic

MODULES	:= common \
	memory bintree rbtree heap list slist vector hash string

all: $(addprefix libcstl,.so .a) check

b build: $(addprefix libcstl,.so .a)

libcstl.so: $(MODULES:=.o)
	@echo "  LD  $(@)"
	$(QUIET)$(CC) -fPIC -rdynamic -shared -o $(@) $(^)

libcstl.a: $(MODULES:=.o)
	@echo "  AR  $(@)"
	$(QUIET)$(AR) -rc $(@) $(^)

%.o: %.c
	@echo "  CC  $(@)"
	$(QUIET)$(CC) $(CFLAGS) -O2 -fPIC -DNDEBUG -o $(@) -c $(<)

%_test.o: %.c
	@echo "  CC  $(@)"
	$(QUIET)$(CC) $(CFLAGS) -g -D__cfg_test__ -o $(@) -c $(<)

check: $(addsuffix _test.o,$(MODULES) check)
	@echo "  LD  $(@)"
	$(QUIET)$(CC) $(CFLAGS) -g -o $(@) $(^) -lcheck -lsubunit -lm

t test: check
	$(QUIET)CK_VERBOSITY=normal ./$(<)

tv testv: check
	$(QUIET)CK_VERBOSITY=verbose ./$(<)

valgrind: check
	$(QUIET)CK_VERBOSITY=silent ./$(<)
	$(QUIET)CK_VERBOSITY=silent CK_FORK=no $(@) --leak-check=full -s ./$(<)

gdb: check
	$(QUIET)CK_FORK=no gdb ./$(<)

doc: doc/html/index.html
doc/html/index.html: doc/doxygen.conf $(wildcard *.h) $(wildcard *.c)
	$(QUIET)doxygen $(<) >/dev/null

docclean:
	$(QUIET)rm -rf doc/html

devclean:
	$(QUIET)rm -f *~

clean: devclean docclean
	$(QUIET)rm -f check $(addprefix check_test,.o .d)
	$(QUIET)rm -f $(MODULES:=.o) $(MODULES:=_test.o)
	$(QUIET)rm -f $(MODULES:=.d) $(MODULES:=_test.d)
	$(QUIET)rm -f $(addprefix libcstl,.a .so)

sinclude *.d

.PHONY: doc docclean devclean clean
