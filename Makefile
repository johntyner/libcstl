QUIET	:= @

CC	:= gcc

CFLAGS	:= -MMD \
	-Wall -Wextra -Werror=vla -Werror=declaration-after-statement \
	-std=c99 -pedantic \
	-D_POSIX_C_SOURCE=199309L

CDBGFLAGS	:= -O0 -g
CTSTFLAGS	:= -fprofile-arcs -ftest-coverage -D__cfg_test__
CRELFLAGS	:= -O2 -Wno-unused-function -fPIC -DNDEBUG

LDFLAGS	:= -fPIC -rdynamic

INCLUDE	:= $(addprefix -I,include)

define sources
	$(filter-out $(addprefix $(1)/,_%.c $(2)),$(wildcard $(1)/*.c))
endef
define objects
	$(patsubst $(strip $(1))/%.c,$(2)/%.o,$(3))
endef

LIBSRCS	:= $(call sources, src, check.c)
LIBOBJS	:= $(call objects, src, build, $(LIBSRCS))

CHKSRCS	:= $(call sources, src)
CHKOBJS	:= $(call objects, src, build/test, $(CHKSRCS))

BCHSRCS	:= $(call sources, benches)
BCHOBJS	:= $(call objects, benches, build/benches, $(BCHSRCS))

.PHONY: b build
build b: $(addprefix build/libcstl,.so .a)

.PHONY: all
all: $(addprefix build/,\
	$(addprefix libcstl,.so .a) test/check benches/run) doc

build/libcstl.so: $(LIBOBJS)
	@echo "  LD\t$(@)"
	$(QUIET)$(CC) $(LDFLAGS) -shared -o $(@) $(^) -lm

build/libcstl.a: $(LIBOBJS)
	@echo "  AR\t$(@)"
	$(QUIET)$(AR) -rc $(@) $(^)

build/%.o: src/%.c Makefile
	@echo "  CC\t$(@)"
	$(QUIET)$(CC) $(CFLAGS) $(CRELFLAGS) $(INCLUDE) -o $(@) -c $(<)

build/benches/%.o: benches/%.c Makefile
	@echo "  CC\t$(@)"
	$(QUIET)$(CC) $(CFLAGS) $(CRELFLAGS) $(INCLUDE) -o $(@) -c $(<)

build/test/%.o: src/%.c Makefile
	@echo "  CC\t$(@)"
	$(QUIET)$(CC) $(CFLAGS) $(CDBGFLAGS) $(CTSTFLAGS) $(INCLUDE) \
		-o $(@) -c $(<)
	$(QUIET)rm -f $(@:.o=.gcda)

build/test/check: $(CHKOBJS)
	@echo "  LD\t$(@)"
	$(QUIET)$(CC) $(CFLAGS) $(CDBGFLAGS) $(CTSTFLAGS) \
		-o $(@) $(^) -lcheck -lsubunit -lm -lgcov

build/benches/run: $(BCHOBJS) build/libcstl.a
	@echo "  LD\t$(@)"
	$(QUIET)$(CC) $(CFLAGS) $(CRELFLAGS) -o $(@) $(^) -lm

.PHONY: bench
bench: build/benches/run
	$(QUIET)$(<)

.PHONY: t test
t test: build/test/check
	$(QUIET)CK_VERBOSITY=normal $(<)

.PHONY: gcov
gcov: build/test/check
	$(QUIET)CK_VERBOSITY=silent $(<)
	$(QUIET)gcov $(CHKOBJS)
	$(QUIET)mv *.gcov $(dir $(<))

.PHONY: gcovr
gcovr: gcov
	$(QUIET)gcovr build/test

.PHONY: tv testv
tv testv: build/test/check
	$(QUIET)CK_VERBOSITY=verbose $(<)

.PHONY: vg valgrind
vg valgrind: build/test/check
	$(QUIET)CK_FORK=no \
		valgrind --leak-check=full -s --error-exitcode=1 $(<)

.PHONY: gdb
gdb: build/test/check
	$(QUIET)CK_FORK=no gdb $(<)

.PHONY: fmt
fmt: $(wildcard src/*.c benches/*.c include/internal/*.h include/cstl/*.h)
	$(QUIET)astyle --options=.astylerc --suffix=none -Q $(^)

.PHONY: doc
doc: build/doc/html/index.html
build/doc/html/index.html: doxygen.conf \
		$(wildcard include/cstl/*.h) $(wildcard src/*.c)
	$(QUIET)echo "  DOC\t$(@D)"
	$(QUIET)doxygen $(<) >/dev/null

.PHONY: docclean
docclean:
	$(QUIET)rm -rf build/doc

.PHONY: devclean
devclean:
	$(QUIET)find . -type f -name "*~" -exec rm -f {} \;

.PHONY: clean
clean: devclean docclean
	$(QUIET)rm -f $(addprefix build/libcstl,.a .so)
	$(QUIET)rm -f $(LIBOBJS) $(LIBOBJS:.o=.d)
	$(QUIET)rm -f  build/test/check
	$(QUIET)rm -f $(CHKOBJS) $(CHKOBJS:.o=.d)
	$(QUIET)rm -f $(CHKOBJS:.o=.gcno) $(CHKOBJS:.o=.gcda)
	$(QUIET)rm -f build/test/*.gcov
	$(QUIET)rm -f build/benches/run
	$(QUIET)rm -f $(BCHOBJS) $(BCHOBJS:.o=.d)

sinclude build/*.d
sinclude build/test/*.d
sinclude build/benches/*.d
