/ ?= ./
BUILD := $/.build

CC := gcc
CXX := g++
LD := $(CC)
AR ?= ar
RM ?= rm --force

CFLAGS := -std=c2x -iquote $/server

CXXFLAGS := -std=c++20
CXXFLAGS += -iquote $/libs -iquote $/gui

CFLAGS += $(shell grep -vP '^(#|$$)' ./warning_flags.c.conf)
CXXFLAGS += $(shell grep -vP '^(#|$$)' ./warning_flags.cpp.conf)
# Do not listen to bad hightlighter  ^

CFLAGS_release := -O2 -fomit-frame-pointer
CXXFLAGS_release := -O2 -fomit-frame-pointer

CFLAGS_debug := -g3 -fsanitize=address,leak,undefined -DDEBUG_MODE=1
CXXFLAGS_debug := -g3 -fsanitize=address,leak,undefined -DDEBUG_MODE=1

CFLAGS_cov := --coverage -g3
CXXFLAGS_cov := --coverage -g3
CFLAGS_tests := --coverage -g3
CXXFLAGS_tests := --coverage -g3

LDLIBS_server := -lm
LDFLAGS_server :=

LDFLAGS_gui != pkg-config --libs-only-L sdl2
LDLIBS_gui != pkg-config --libs-only-l sdl2 SDL2_image glew gl glu
CXXFLAGS_gui += $(shell pkg-config --cflags sdl2)

IMGUI_DIR ?= gui/imgui
IMGUI_SRC := $(wildcard $(IMGUI_DIR)/*.cpp)
IMGUI_SRC += $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
IMGUI_SRC += $(IMGUI_DIR)/backends/imgui_impl_sdl2.cpp

IMGUI_INC += $(IMGUI_DIR) $(IMGUI_DIR)/backends

# Used to simplify the nix build
GUI_ASSET_DIR ?= assets

CXXFLAGS_gui += -DASSET_DIR='"$(GUI_ASSET_DIR)"'
CXXFLAGS_gui += $(foreach d,$(IMGUI_INC),-iquote $d)

MAKEFLAGS += --no-builtin-rules
MAKEFLAGS += --no-print-directory

.PHONY: _start all debug
_start: all

GENERIC_SUFFIX_C := c
GENERIC_SUFFIX_CPP := cpp
GENERIC_FLAGS_C := C
GENERIC_FLAGS_CPP := CXX

NAME_server_release := zappy_server
NAME_server_debug := debug_server
NAME_server_tests := tests_server

NAME_gui_release := zappy_gui
NAME_gui_debug := debug_gui
NAME_gui_tests := tests_gui

EXTRA_SRC_gui_tests != find tests/gui -name "*.cpp"
EXTRA_SRC_server_tests != find tests/server -type f -name "*.c"

SRC_gui != find gui -type f -name "*.cpp" -not -path "gui/imgui/*"
SRC_gui += $(IMGUI_SRC)

SRC_server != find server -type f -name "*.c"


# call mk-bin, bin-name, profile, lang
# DOES THIS MAKE COFFEE NOW ??
define mk-bin

$(BUILD)/$(strip $2)/%.o: %.$(GENERIC_SUFFIX_$(strip $3))
	@ mkdir -p $$(dir $$@)
	$$Q $$(COMPILE.$(GENERIC_SUFFIX_$(strip $3)))                             \
		-MMD -MP -MF $$(@:.o=.d)                                              \
		-o $$@ -c $$<                                                         \
			$$($(GENERIC_FLAGS_$(strip $3))FLAGS)                             \
			$$($(GENERIC_FLAGS_$(strip $3))FLAGS_$(strip $1))                 \
			$$($(GENERIC_FLAGS_$(strip $3))FLAGS_$(strip $2))
	@ $$(LOG_TIME) "CC $$(C_YELLOW)$(strip $2)$$(C_RESET)                     \
		$$(C_PURPLE)$$(notdir $$@) $$(C_RESET)"

out_$(strip $1)_$(strip $2) := $(NAME_$(strip $1)_$(strip $2))

src_$(strip $1)_$(strip $2) := $(SRC_$(strip $1))
src_$(strip $1)_$(strip $2) += $(EXTRA_SRC_$(strip $1)_$(strip $2))

objs_$(strip $1)_$(strip $2) :=                                               \
	$$(src_$(strip $1)_$(strip $2):%.$(GENERIC_SUFFIX_$(strip                 \
	$3))=$(BUILD)/$(strip $2)/%.o)

$$(out_$(strip $1)_$(strip $2)): LDFLAGS += $(LDLIBS_$(strip $1))
$$(out_$(strip $1)_$(strip $2)): LDLIBS += $(LDLIBS_$(strip $1))
$$(out_$(strip $1)_$(strip $2)): $$(objs_$(strip $1)_$(strip $2))
	$$Q $$(LINK.$(GENERIC_SUFFIX_$(strip $3)))                                \
			$$($(GENERIC_FLAGS_$(strip $3))FLAGS)                             \
			$$($(GENERIC_FLAGS_$(strip $3))FLAGS_$(strip $1))                 \
			$$($(GENERIC_FLAGS_$(strip $3))FLAGS_$(strip $2))                 \
-o $$@ $$^ $$(LDLIBS) $$(LDFLAGS)
	@ $$(LOG_TIME) "LD $$(C_GREEN)$$@ $$(C_RESET)"

every_out += $$(out_$(strip $1)_$(strip $2))
every_obj += $$(objs_$(strip $1)_$(strip $2))

endef

LANG_server := C
LANG_gui := CPP

$(foreach target, server gui,                                                 \
$(foreach build-mode, release debug tests,                                    \
	$(eval $(call mk-bin, $(target), $(build-mode), $(LANG_$(target))))       \
))

ifeq ($(V),2)
$(foreach target, server gui,                                                 \
$(foreach build-mode, release debug cov tests,                                \
	$(info $(call mk-bin, $(target), $(build-mode), $(LANG_$(target))))       \
))
endif

-include $(every_obj:.o=.d)

all: zappy_server zappy_gui zappy_ai

ifneq ($(auto-complete),)
# auto complete for dumb terminals

debug_gui:
debug_server:
zappy_ai:
zappy_gui:
zappy_server:
tests_gui:
tests_server:
tests_run_server:
tests_run_gui:
endif

IN_DOCKER != stat /.dockerenv >/dev/null 2>/dev/null && echo 1 || echo 0

PIP_FLAGS := --quiet
# unline
PIP_FLAGS += --no-build-isolation
PIP_FLAGS += --disable-pip-version-check

ifneq ($(IN_DOCKER),1)
venv:
	$Q bash configure-venv.sh
	@ $(LOG_TIME) "PY $(C_YELLOW)$@ $(C_RESET)"

venv/bin/zappy_ai: venv $(shell find ai -type f -name "*.py")
	$Q venv/bin/pip install hatchling editables $(PIP_FLAGS)
	$Q venv/bin/pip install -e . $(PIP_FLAGS)
	@ $(LOG_TIME) "DL $(C_YELLOW)$@ $(C_RESET)"

zappy_ai: venv/bin/zappy_ai
	@ ln -fs venv/bin/zappy_ai .
	@ $(LOG_TIME) "LN $(C_GREEN)$@ $(C_RESET)"
else

# We have to break system packages
# ¯\_(ツ)_/¯

export PIP_ROOT_USER_ACTION=ignore

zappy_ai:
	pip install hatchling --break-system-packages
	pip install . -q --break-system-packages $(PIP_FLAGS)
	@ ln -fs $(shell which zappy_ai) .
	@ $(LOG_TIME) "PY $(C_YELLOW)$@ $(C_RESET)"

endif

html-doc: #? html-doc: Build the static html documentation
	doxygen Doxyfile
	$(MAKE) -C docs html
	@ $(LOG_TIME) "DO $(C_YELLOW)$@ $(C_RESET)"

.PHONY: help
help: #? help: Show this help message
	@ grep -P "#[?] " $(MAKEFILE_LIST)                                        \
	  | sed -E 's/.*#\? ([^:]+): (.*)/\1 "\2"/'                               \
	  | xargs printf "%-12s: %s\n"

.PHONY: clean fclean re
clean:
	$(RM) $(every_obj)

fclean: clean
	$(RM) $(every_out) zappy_ai

mrproper: fclean
	$(RM) -rf venv $(BUILD)
	$(RM) -rf $(BUILD)
	$(RM) -rf docs/source/api docs/doxygen
	$(RM) -rf compile_commands.json
	$(RM) -rf logs

.NOTPARALLEL: re
re: fclean all

tests_run_%: tests_%
	./$^

tests_run_ai: venv
	pytest . --cov=ai --no-summary

.PHONY: cov cov_ai cov_server cov_gui

cov_ai: tests_run_ai
	coverage report -m

cov_gui: tests_run_gui
	gcovr $(BUILD)/tests/gui --exclude=tests

cov_server: tests_run_server
	gcovr $(BUILD)/tests/server --exclude=tests

.NOTPARALLEL: cov
cov: cov_ai cov_gui cov_server

bleach: bleach.c
	$(CC) -o $@ -O2 $<

V ?= 0
ifneq ($(V),0)
  Q :=
else
  Q := @
endif

ifneq ($(shell command -v tput),)
  ifneq ($(shell tput colors),0)

mk-color = \e[$(strip $1)m

C_BEGIN := \033[A
C_RESET := $(call mk-color, 00)

C_RED := $(call mk-color, 31)
C_GREEN := $(call mk-color, 32)
C_YELLOW := $(call mk-color, 33)
C_BLUE := $(call mk-color, 34)
C_PURPLE := $(call mk-color, 35)
C_CYAN := $(call mk-color, 36)

  endif
endif

NOW = $(shell date +%s%3N)

STIME := $(shell date +%s%3N)
export STIME

define TIME_MS
$$( expr \( $$(date +%s%3N) - $(STIME) \))
endef

BOXIFY = "[$(C_BLUE)$(1)$(C_RESET)] $(2)"

ifneq ($(shell command -v printf),)
  LOG_TIME = printf $(call BOXIFY, %6s , %b\n) "$(call TIME_MS)"
else
  LOG_TIME = echo -e $(call BOXIFY, $(call TIME_MS) ,)
endif

-include ./hook.mk
