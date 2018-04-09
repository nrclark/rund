.SUFFIXES:
CC := gcc

all: librund.a

#------------------------------------------------------------------------------#

lint_flags.mk:
	$(CC) -Wall -Wextra -pedantic -ansi -std=c11 -Q --help=warning | \
	grep -P "(disabled|[=] )" | grep -Po "[-]W[^ \t=]+" | \
	sort | uniq > $@.temp.init
	echo "int main(void) { return 0;}" > $@.temp.c
	$(CC) $$(cat $@.temp.init) $@.temp.c -o /dev/null 2>&1 | \
	grep "error: " | grep -oP "[-]W[a-zA-Z0-9_-]+" | \
	sort | uniq > $@.temp.blacklist
	cat $@.temp.init | grep -vFf $@.temp.blacklist > $@.temp.works
	$(CC) $$(cat $@.temp.works) $@.temp.c -o /dev/null 2>&1 | \
	    grep -P "is valid for [^ ]+ but not for C" | \
	    grep -oP "[-]W[a-zA-Z0-9_-]+" > $@.temp.blacklist
	cat $@.temp.works | grep -vFf $@.temp.blacklist > $@.temp.ok
	echo 'LINT_CFLAGS := \' > $@
	cat $@.temp.ok | sed 's/$$/ \\/g' >> $@
	echo "" >> $@
	rm -f $@.*

-include lint_flags.mk

LINT_BLACKLIST := \
    -Wtraditional -Wformat-nonliteral -Wtraditional-conversion -Wpadded \
    -Wunused-macros

LINT_CFLAGS := $(strip \
    -Wall -Wextra -pedantic -ansi -std=c11 \
    $(filter-out $(LINT_BLACKLIST),$(LINT_CFLAGS)) \
    -Wno-system-headers \
)

lint-%.h: %.h
	@echo Checking $< against lots of compiler warnings...
	@$(CC) -c $(LINT_CFLAGS) -Wno-unused-macros $< -o /dev/null

lint-%.c: %.c
	@echo Checking $< against lots of compiler warnings...
	@$(CC) -c $(LINT_CFLAGS) $< -o /dev/null

$(foreach x,$(wildcard *.c),$(eval lint-$x:))
$(foreach x,$(wildcard *.h),$(eval lint-$x:))
lint: $(foreach x,$(wildcard *.c),lint-$x)
lint: $(foreach x,$(filter-out config.h,$(wildcard *.h)),lint-$x)

clean::
	rm -f lint_flags.mk

#------------------------------------------------------------------------------#

format-%: % uncrustify.cfg
	@echo Formatting $*...
	@cp -a $* $*.temp.c
	@uncrustify -c uncrustify.cfg --no-backup $*.temp.c -lc -q
	@sed -i 's/^    "/"/g' $*.temp.c
	@if ! diff -q $*.temp.c $* 1>/dev/null; then cp $*.temp.c $*; fi
	@rm $*.temp.c

$(foreach x,$(wildcard *.c),$(eval format-$x:))
$(foreach x,$(wildcard *.h),$(eval format-$x:))
format: $(foreach x,$(wildcard *.c),format-$x)
format: $(foreach x,$(wildcard *.h),format-$x)

#------------------------------------------------------------------------------#

tidy-%: %
	@echo Analyzing $* with clang-tidy/clang-check...
	@clang-tidy \
	    "-checks=*,-llvm-header-guard,-android-cloexec-open,-android-cloexec-fopen,-hicpp-no-assembler" \
	    "-header-filter=.*" $* -- 2>/dev/null | \
	    (grep -iP "(warning|error)[:]" -A2 --color || true)
	@clang-check -analyze $* --

$(foreach x,$(wildcard *.c),$(eval tidy-$x:))
$(foreach x,$(wildcard *.h),$(eval tidy-$x:))
tidy: $(foreach x,$(wildcard *.c),tidy-$x)
tidy: $(foreach x,$(filter-out config.h,$(wildcard *.h)),tidy-$x)

clean::
	rm -f *.plist

#------------------------------------------------------------------------------#

sanitize ?= 1

SAN_CFLAGS := \
    -O0 -g \
    -fstack-protector-strong \
    -fstack-protector-all \
    -fsanitize=shift \
    -fsanitize=undefined \
    -fsanitize=address \
    -fsanitize=alignment \
    -fsanitize=bool \
    -fsanitize=bounds \
    -fsanitize=bounds-strict \
    -fsanitize=enum \
    -fsanitize=float-cast-overflow \
    -fsanitize=float-divide-by-zero \
    -fsanitize=integer-divide-by-zero \
    -fsanitize=null \
    -fsanitize=object-size \
    -fsanitize=leak \
    -fno-sanitize-recover=all \
    -fsanitize=return \
    -fsanitize=vla-bound \
    -fsanitize=unreachable \
    -fsanitize=returns-nonnull-attribute \
    -fsanitize=signed-integer-overflow \
    -fstack-check

#------------------------------------------------------------------------------#

cppcheck-%:
	@(cppcheck $* --force --enable=warning,style,performance,portability \
	-I `pwd` -I /usr/include -I /usr/include/linux \
	-I /usr/lib/gcc/x86_64-redhat-linux/7/include \
	--std=c99 1>/dev/null) 2>&1 | (grep -vP "^[(]information" 1>&2 || true)

$(foreach x,$(wildcard *.c),$(eval cppcheck-$x:))
$(foreach x,$(wildcard *.h),$(eval cppcheck-$x:))
cppcheck: $(foreach x,$(wildcard *.c),cppcheck-$x)
cppcheck: $(foreach x,$(wildcard *.h),cppcheck-$x)

include-%:
	@(include-what-you-use \
	    -I/usr/lib/gcc/x86_64-redhat-linux/7/include $* || true) 2>&1 | \
	    grep -P "(should remove these lines|has correct [#]includes)" || true
	@(include-what-you-use \
	    -I/usr/lib/gcc/x86_64-redhat-linux/7/include $* || true) 2>&1 | \
	    grep -P "^[-] " || true

fullinclude-%:
	-include-what-you-use -I/usr/lib/gcc/x86_64-redhat-linux/7/include $*

include: $(foreach x,$(wildcard *.c),include-$x)
include: $(foreach x,$(wildcard *.h),include-$x)
$(foreach x,$(wildcard *.c),$(eval include-$x:))
$(foreach x,$(wildcard *.h),$(eval include-$x:))

#------------------------------------------------------------------------------#

CFLAGS := -Wall -Wextra -pedantic
ifneq ($(sanitize),)
CFLAGS += $(SAN_CFLAGS)
else
CFLAGS += -O2
CFLAGS += -ffunction-sections -fdata-sections -flto -Wl,-flto,--gc-sections
endif

CFLAGS := $(strip $(CFLAGS))

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean::
	rm -f *.o

#------------------------------------------------------------------------------#

LIBRUND_SRC := $(wildcard *.c)
LIBRUND_SRC += $(wildcard *.h)

LIBRUND_SRC := $(filter-out test-%,$(LIBRUND_SRC))

librund.a: $(filter %.o,$(patsubst %.c,%.o,$(LIBRUND_SRC)))
	rm -f $@
	gcc-ar rcs $@ $^

lint-librund: $(foreach x,$(LIBRUND_SRC),lint-$x)
format-librund: $(foreach x,$(LIBRUND_SRC),format-$x)
tidy-librund: $(foreach x,$(LIBRUND_SRC),tidy-$x)

clean::
	rm -f librund.a

#------------------------------------------------------------------------------#

test-%: test-%.c librund.a
	$(CC) $(CFLAGS) $^ -o $@

clean::
	rm -f $(patsubst %.c,%,$(wildcard test-*.c))
