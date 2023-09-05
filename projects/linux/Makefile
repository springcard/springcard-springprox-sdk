#
# SpringCard SpringProx Legacy SDK - Makefile for Win32, with MinGW+GCC as toolchain
# ----------------------------------------------------------------------------------
#
# Requirements:
# - GCC (of course)
# - GNU utilities (make, mkdir...)
# - MinGW to run all the above
#
# If you don't already have a working MinGW+GCC setup on your com:
# - https://code.visualstudio.com/docs/cpp/config-mingw
# - https://earthly.dev/blog/vscode-make/
#
# To build the sample, just open a shell in the directory containing the Makefile,
# and enter 'make'
#
#

# Directory where all the source files are
SOURCE_DIR:=../../src
# Common files
COMMON_DIR:=../../src/common
# Directory for objects
OBJECT_DIR:=./obj
# Directory for the program
OUTPUT_DIR:=./bin

# We use GCC for compiling and linking
CC:=gcc
# All warnings enabled, stop on warning
CFLAGS:=-Wall -Wextra -Werror -Wno-format -DSPROX_API_NO_FTDI -DSPROX_API_ONLY_BIN -DCALYPSO_LEGACY -D__MISSING_STRL -fPIC
# The source will be looking for common files
CINCL:=-I$(COMMON_DIR)

#
# LIBRARIES
# ---------
#
SPRINGPROX_SRCS:=	\
	$(COMMON_DIR)/products/springprox/api/sprox_14443-3.c \
	$(COMMON_DIR)/products/springprox/api/sprox_14443-4.c \
	$(COMMON_DIR)/products/springprox/api/sprox_14443-bi.c \
	$(COMMON_DIR)/products/springprox/api/sprox_card.c \
	$(COMMON_DIR)/products/springprox/api/sprox_15693.c \
	$(COMMON_DIR)/products/springprox/api/sprox_api.c \
	$(COMMON_DIR)/products/springprox/api/sprox_comm_linux.c \
	$(COMMON_DIR)/products/springprox/api/sprox_conf_linux.c \
	$(COMMON_DIR)/products/springprox/api/sprox_crc.c \
	$(COMMON_DIR)/products/springprox/api/sprox_dialog.c \
	$(COMMON_DIR)/products/springprox/api/sprox_dlg_bin.c \
	$(COMMON_DIR)/products/springprox/api/sprox_fct.c \
	$(COMMON_DIR)/products/springprox/api/sprox_find.c \
	$(COMMON_DIR)/products/springprox/api/sprox_hlp.c \
	$(COMMON_DIR)/products/springprox/api/sprox_mifare.c \
	$(COMMON_DIR)/products/springprox/api/sprox_trace.c \
	$(COMMON_DIR)/products/springprox/api/REVISION.c \
	$(COMMON_DIR)/lib-c/utils/strl.c

SPRINGPROX_OBJS:=$(patsubst %.c,%.o,$(SPRINGPROX_SRCS))
SPRINGPROX_OBJS:=$(subst $(COMMON_DIR),$(OBJECT_DIR),$(SPRINGPROX_OBJS))

SPRINGPROX_SO:=$(OUTPUT_DIR)/libspringprox.so

SPROX_DESFIRE_SRCS:=	\
	$(COMMON_DIR)/cardware/desfire/sprox_desfire_aes.c \
	$(COMMON_DIR)/cardware/desfire/sprox_desfire_auth.c \
	$(COMMON_DIR)/cardware/desfire/sprox_desfire_cipher.c \
	$(COMMON_DIR)/cardware/desfire/sprox_desfire_cmac.c \
	$(COMMON_DIR)/cardware/desfire/sprox_desfire_core.c \
	$(COMMON_DIR)/cardware/desfire/sprox_desfire_crc.c \
	$(COMMON_DIR)/cardware/desfire/sprox_desfire_des.c \
	$(COMMON_DIR)/cardware/desfire/sprox_desfire_files.c \
	$(COMMON_DIR)/cardware/desfire/sprox_desfire_iso.c \
	$(COMMON_DIR)/cardware/desfire/sprox_desfire_keys.c \
	$(COMMON_DIR)/cardware/desfire/sprox_desfire_legacy.c \
	$(COMMON_DIR)/cardware/desfire/sprox_desfire_mac.c \
	$(COMMON_DIR)/cardware/desfire/sprox_desfire_mgmt.c \
	$(COMMON_DIR)/cardware/desfire/sprox_desfire_msg.c \
	$(COMMON_DIR)/cardware/desfire/sprox_desfire_old.c \
	$(COMMON_DIR)/cardware/desfire/sprox_desfire_rand.c \
	$(COMMON_DIR)/cardware/desfire/sprox_desfire_read.c \
	$(COMMON_DIR)/cardware/desfire/sprox_desfire_records.c \
	$(COMMON_DIR)/cardware/desfire/sprox_desfire_trans.c \
	$(COMMON_DIR)/cardware/desfire/sprox_desfire_value.c \
	$(COMMON_DIR)/cardware/desfire/sprox_desfire_wrap.c \
	$(COMMON_DIR)/cardware/desfire/sprox_desfire_write.c

SPROX_DESFIRE_OBJS:=$(patsubst %.c,%.o,$(SPROX_DESFIRE_SRCS))
SPROX_DESFIRE_OBJS:=$(subst $(COMMON_DIR),$(OBJECT_DIR),$(SPROX_DESFIRE_OBJS))

SPROX_DESFIRE_SO:=$(OUTPUT_DIR)/libsprox_desfire.so

SPROX_MIFULC_SRCS:=	\
	$(COMMON_DIR)/cardware/mifulc/sprox_mifulc_des.c \
	$(COMMON_DIR)/cardware/mifulc/sprox_mifulc_func.c \
	$(COMMON_DIR)/cardware/mifulc/sprox_mifulc_legacy.c \
	$(COMMON_DIR)/cardware/mifulc/sprox_mifulc_rand.c

SPROX_MIFULC_OBJS:=$(patsubst %.c,%.o,$(SPROX_MIFULC_SRCS))
SPROX_MIFULC_OBJS:=$(subst $(COMMON_DIR),$(OBJECT_DIR),$(SPROX_MIFULC_OBJS))

SPROX_MIFULC_SO:=$(OUTPUT_DIR)/libsprox_mifulc.so

SPROX_MIFPLUS_SRCS:=	\
	$(COMMON_DIR)/cardware/mifplus/sprox_mifplus_aes.c \
	$(COMMON_DIR)/cardware/mifplus/sprox_mifplus_auth.c \
	$(COMMON_DIR)/cardware/mifplus/sprox_mifplus_cipher.c \
	$(COMMON_DIR)/cardware/mifplus/sprox_mifplus_cmac.c \
	$(COMMON_DIR)/cardware/mifplus/sprox_mifplus_level0.c \
	$(COMMON_DIR)/cardware/mifplus/sprox_mifplus_level3.c \
	$(COMMON_DIR)/cardware/mifplus/sprox_mifplus_legacy.c \
	$(COMMON_DIR)/cardware/mifplus/sprox_mifplus_rand.c \
	$(COMMON_DIR)/cardware/mifplus/sprox_mifplus_reentrant.c \
	$(COMMON_DIR)/cardware/mifplus/sprox_mifplus_utils.c \
	$(COMMON_DIR)/cardware/mifplus/sprox_mifplus_vc.c

SPROX_MIFPLUS_OBJS:=$(patsubst %.c,%.o,$(SPROX_MIFPLUS_SRCS))
SPROX_MIFPLUS_OBJS:=$(subst $(COMMON_DIR),$(OBJECT_DIR),$(SPROX_MIFPLUS_OBJS))

SPROX_MIFPLUS_SO:=$(OUTPUT_DIR)/libsprox_mifplus.so

SPROX_CALYPSO_SRCS:=	\
	$(COMMON_DIR)/cardware/calypso/calypso_strings.c \
	$(COMMON_DIR)/cardware/calypso/readers/calypso_reader_abstract.c \
	$(COMMON_DIR)/cardware/calypso/readers/calypso_reader_legacy.c \
	$(COMMON_DIR)/cardware/calypso/entries/calypso_entries_pc.c \
	$(COMMON_DIR)/cardware/calypso/entries/calypso_explorer.c \
	$(COMMON_DIR)/cardware/calypso/entries/calypso_get_info.c \
	$(COMMON_DIR)/cardware/calypso/functions/calypso_card_commands.c \
	$(COMMON_DIR)/cardware/calypso/functions/calypso_card_manage.c \
	$(COMMON_DIR)/cardware/calypso/functions/calypso_card_pin.c \
	$(COMMON_DIR)/cardware/calypso/functions/calypso_card_read.c \
	$(COMMON_DIR)/cardware/calypso/functions/calypso_card_session.c \
	$(COMMON_DIR)/cardware/calypso/functions/calypso_card_write.c \
	$(COMMON_DIR)/cardware/calypso/functions/calypso_sam_commands.c \
	$(COMMON_DIR)/cardware/calypso/functions/calypso_storedvalue.c \
	$(COMMON_DIR)/cardware/calypso/functions/calypso_transaction.c \
	$(COMMON_DIR)/cardware/calypso/intercode/calypso_intercode_datetime.c \
	$(COMMON_DIR)/cardware/calypso/intercode/calypso_intercode_from_struct.c \
	$(COMMON_DIR)/cardware/calypso/intercode/calypso_intercode_to_struct.c \
	$(COMMON_DIR)/cardware/calypso/intercode/calypso_intercode_to_xml_core.c \
	$(COMMON_DIR)/cardware/calypso/intercode/calypso_intercode_to_xml_pc.c \
	$(COMMON_DIR)/cardware/calypso/others/calypso_bench_pc.c \
	$(COMMON_DIR)/cardware/calypso/others/calypso_info_to_xml.c \
	$(COMMON_DIR)/cardware/calypso/others/calypso_atr_to_xml.c \
	$(COMMON_DIR)/cardware/calypso/others/calypso_pc_trace.c \
	$(COMMON_DIR)/cardware/calypso/others/calypso_pc_datetime.c \
	$(COMMON_DIR)/cardware/calypso/parsers/calypso_parser_asn1.c \
	$(COMMON_DIR)/cardware/calypso/parsers/calypso_parser_atr.c \
	$(COMMON_DIR)/cardware/calypso/parsers/calypso_parser_fci.c \
	$(COMMON_DIR)/cardware/calypso/parsers/calypso_parser_finfo.c \
	$(COMMON_DIR)/lib-c/utils/binconvert.c \
	$(COMMON_DIR)/lib-c/utils/strl.c

SPROX_CALYPSO_OBJS:=$(patsubst %.c,%.o,$(SPROX_CALYPSO_SRCS))
SPROX_CALYPSO_OBJS:=$(subst $(COMMON_DIR),$(OBJECT_DIR),$(SPROX_CALYPSO_OBJS))

SPROX_CALYPSO_SO:=$(OUTPUT_DIR)/libsprox_calypso.so

LIBRARIES:=\
	$(SPRINGPROX_SO) \
	$(SPROX_DESFIRE_SO) \
	$(SPROX_MIFULC_SO) \
	$(SPROX_MIFPLUS_SO) \
	$(SPROX_CALYPSO_SO)

#
# SAMPLES
# -------
#

# Select all the sources
SAMPLES_C:=\
	$(wildcard $(SOURCE_DIR)/samples/*.c)

# Make objects from sources
SAMPLES_O:=$(patsubst %.c,%.o,$(SAMPLES_C))
SAMPLES_O:=$(subst $(SOURCE_DIR),$(OBJECT_DIR),$(SAMPLES_O))

# Make programs from sources
SAMPLES_EXE:=$(patsubst %.c,%,$(SAMPLES_C))
SAMPLES_EXE:=$(subst $(SOURCE_DIR)/samples,$(OUTPUT_DIR),$(SAMPLES_EXE))

# Build the programs
all: $(LIBRARIES) $(SAMPLES_EXE)

# Rule to link a program
$(OUTPUT_DIR)/%: $(OBJECT_DIR)/samples/%.o | $(OUTPUT_DIR)
	$(CC) -o $@ $^ -L$(OUTPUT_DIR) -l$(subst lib,,$(subst .so,,$(notdir $(SPRINGPROX_SO)))) -l$(subst lib,,$(subst .so,,$(notdir $(SPROX_DESFIRE_SO)))) -l$(subst lib,,$(subst .so,,$(notdir $(SPROX_MIFULC_SO)))) -l$(subst lib,,$(subst .so,,$(notdir $(SPROX_MIFPLUS_SO)))) -l$(subst lib,,$(subst .so,,$(notdir $(SPROX_CALYPSO_SO))))

# Rule to link every library
$(SPRINGPROX_SO): $(SPRINGPROX_OBJS) | $(OUTPUT_DIR)
	$(CC) -o $@ $(SPRINGPROX_OBJS) -shared

$(SPROX_DESFIRE_SO): $(SPROX_DESFIRE_OBJS) | $(OUTPUT_DIR)
	$(CC) -o $@ $(SPROX_DESFIRE_OBJS) -shared -L$(OUTPUT_DIR) -l$(subst lib,,$(subst .so,,$(notdir $(SPRINGPROX_SO))))

$(SPROX_MIFULC_SO): $(SPROX_MIFULC_OBJS) | $(OUTPUT_DIR)
	$(CC) -o $@ $(SPROX_MIFULC_OBJS) -shared -L$(OUTPUT_DIR) -l$(subst lib,,$(subst .so,,$(notdir $(SPRINGPROX_SO))))

$(SPROX_MIFPLUS_SO): $(SPROX_MIFPLUS_OBJS) | $(OUTPUT_DIR)
	$(CC) -o $@ $(SPROX_MIFPLUS_OBJS) -shared -L$(OUTPUT_DIR) -l$(subst lib,,$(subst .so,,$(notdir $(SPRINGPROX_SO))))

$(SPROX_CALYPSO_SO): $(SPROX_CALYPSO_OBJS) | $(OUTPUT_DIR)
	$(CC) -o $@ $(SPROX_CALYPSO_OBJS) -shared -L$(OUTPUT_DIR) -l$(subst lib,,$(subst .so,,$(notdir $(SPRINGPROX_SO))))

# Rule to compile an object from a source file
$(OBJECT_DIR)/%.o: $(SOURCE_DIR)/%.c | $(OBJECT_DIR)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CINCL) -c -o $@ $<

$(OBJECT_DIR)/%.o: $(COMMON_DIR)/%.c | $(OBJECT_DIR)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CINCL) -c -o $@ $<

# Make sure we have the output directories
$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)

$(LIBRARIES_DIR):
	mkdir -p $(LIBRARIES_DIR)

$(OBJECT_DIR):
	mkdir -p $(OBJECT_DIR)

# Clean the objects and the program
.PHONY: clean
clean:
	- rm $(SPRINGPROX_SO)
	- rm $(SPRINGPROX_OBJS)
	- rm $(SPROX_DESFIRE_SO)
	- rm $(SPROX_DESFIRE_OBJS)
	- rm $(SPROX_MIFULC_SO)
	- rm $(SPROX_MIFULC_OBJS)
	- rm $(SPROX_MIFPLUS_SO)
	- rm $(SPROX_MIFPLUS_OBJS)
	- rm $(SPROX_CALYPSO_SO)
	- rm $(SPROX_CALYPSO_OBJS)	
	- rm $(LIBRARIES)
	- rm $(SAMPLES_O)
	- rm $(SAMPLES_EXE)