QUIET	:= @

CC	:= gcc
CFLAGS	:= -MMD -Wall -Wextra -Wno-unused-function -std=c99 -pedantic

MODULES	:= common memory \
	bintree rbtree heap list slist hash vector \
	string map

all: $(addprefix build/,$(addprefix libcstl,.so .a) check) doc

b build: $(addprefix build/libcstl,.so .a)

build/libcstl.so: $(addprefix build/,$(MODULES:=.o))
	@echo "  LD  $(@)"
	$(QUIET)$(CC) -fPIC -rdynamic -shared -o $(@) $(^)

build/libcstl.a: $(addprefix build/,$(MODULES:=.o))
	@echo "  AR  $(@)"
	$(QUIET)$(AR) -rc $(@) $(^)

build/%.o: src/%.c
	@echo "  CC  $(@)"
	$(QUIET)$(CC) $(CFLAGS) -O2 -fPIC -DNDEBUG -Iinclude -o $(@) -c $(<)

build/%_test.o: src/%.c
	@echo "  CC  $(@)"
	$(QUIET)$(CC) $(CFLAGS) -g -D__cfg_test__ -Iinclude -o $(@) -c $(<)

build/check: $(addprefix build/,$(addsuffix _test.o,$(MODULES) check))
	@echo "  LD  $(@)"
	$(QUIET)$(CC) $(CFLAGS) -g -o $(@) $(^) -lcheck -lsubunit -lm

t test: build/check
	$(QUIET)CK_VERBOSITY=normal ./$(<)

tv testv: build/check
	$(QUIET)CK_VERBOSITY=verbose ./$(<)

vg valgrind: build/check
	$(QUIET)CK_VERBOSITY=silent ./$(<)
	$(QUIET)CK_VERBOSITY=silent CK_FORK=no valgrind --leak-check=full -s ./$(<)

gdb: build/check
	$(QUIET)CK_FORK=no gdb ./$(<)

doc: build/doc/html/index.html
build/doc/html/index.html: doxygen.conf \
		$(wildcard include/cstl/*.h) $(wildcard src/*.c)
	$(QUIET)doxygen $(<) >/dev/null

docclean:
	$(QUIET)rm -rf build/doc

devclean:
	$(QUIET)find . -type f -name "*~" -exec rm -f {} \;

clean: devclean docclean
	$(QUIET)rm -f $(addprefix build/,check $(addprefix check_test,.o .d))
	$(QUIET)rm -f $(addprefix build/,$(MODULES:=.o) $(MODULES:=_test.o))
	$(QUIET)rm -f $(addprefix build/,$(MODULES:=.d) $(MODULES:=_test.d))
	$(QUIET)rm -f $(addprefix build/libcstl,.a .so)

sinclude build/*.d

.PHONY: doc docclean devclean clean
