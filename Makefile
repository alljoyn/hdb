CC 	:= g++
LD 	:= g++

CFLAGS		:= -Wall -g -Os -DQCC_OS_LINUX -DQCC_OS_GROUP_POSIX -D_DEBUG -Wno-unused-variable -Wno-deprecated-declarations -Wno-unused-function

TARGET:= hdb

MODULES		:= core core/common plugins plugins/zigbee plugins/wifi  utils
SRC_DIR		:= $(addprefix src/,$(MODULES))
BUILD_ROOT	:= build/

SRC			:= $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.cc))
OBJ			:= $(patsubst src/%.cc,build/%.o,$(SRC))

BUILD_DIR	:= $(addprefix $(BUILD_ROOT),$(MODULES))

INCLUDES	:= $(addprefix -I,$(SRC_DIR)) -Isrc/inc

vpath %.cc $(SRC_DIR)

define make-goal
$1/%.o: %.cc
	$(CC) $(CFLAGS) $(INCLUDES) -c $$< -o $$@
endef

LIBS 		:= -lalljoyn_config -lalljoyn_about -lalljoyn_notification -lalljoyn_onboarding -lalljoyn_services_common -lalljoyn -lpthread -lrt -s -Wl,--gc-sections

ifdef ALLJOYN_DISTDIR
	INCLUDES += -I$(ALLJOYN_DISTDIR)/cpp/inc -I$(ALLJOYN_DISTDIR)/about/inc -I$(ALLJOYN_DISTDIR)/notification/inc -I$(ALLJOYN_DISTDIR)/services_common/inc -I$(ALLJOYN_DISTDIR)/config/inc -I$(ALLJOYN_DISTDIR)/onboarding/inc
	LDFLAGS += -L$(ALLJOYN_DISTDIR)/cpp/lib -L$(ALLJOYN_DISTDIR)/about/lib -L$(ALLJOYN_DISTDIR)/notification/lib -L$(ALLJOYN_DISTDIR)/services_common/lib -L$(ALLJOYN_DISTDIR)/config/lib -L$(ALLJOYN_DISTDIR)/onboarding/lib
endif

.PHONY: all checkdirs clean

all: checkdirs $(BUILD_ROOT) $(BUILD_ROOT)$(TARGET)

$(BUILD_ROOT)$(TARGET): $(OBJ)
	$(LD) $^ $(LDFLAGS) -L$(BUILD_ROOT) $(LIBS) -o $@

checkdirs: $(BUILD_DIR)

$(BUILD_DIR):
	@mkdir -p $@

clean:
	@rm -rf $(BUILD_ROOT)
	
$(foreach bdir,$(BUILD_DIR),$(eval $(call make-goal,$(bdir))))

