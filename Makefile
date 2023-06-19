OUT = build

getfiles = \
	$(foreach d,$(wildcard $1/*),$(call getfiles,$d,$2) $(filter $2,$d))

SRCS = \
	$(call getfiles,src,%.cc)

CXXFLAGS += \
	-std=c++2a -Wall -Wextra -Wno-missing-field-initializers -Wpedantic \
	-Igroufix/include -Isrc

LDFLAGS += -L$(OUT) -Wl,-rpath,'$$ORIGIN'
LDLIBS += -lgroufix

OBJS = $(patsubst %,$(OUT)/%.o,$(SRCS))
DEPS = $(patsubst %,$(OUT)/%.d,$(SRCS))

$(OUT)/fiezta: $(OUT)/libgroufix.so $(OBJS)
	@mkdir -p $(@D)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

$(OUT)/libgroufix.so:
	@mkdir -p $(@D)
	$(MAKE) -C groufix unix
	cp groufix/bin/unix/libgroufix.so $@
	@# TODO: Make this cross-platform
	#install_name_tool -id '@executable_path/libgroufix.so' $@

$(OUT)/%.cc.o: %.cc
	@mkdir -p $(@D)
	$(CXX) -MMD $(CXXFLAGS) -o $@ -c $<
$(OUT)/%.cc.d: $(OUT)/%.cc.o

.PHONY: clean
clean:
	rm -rf $(OUT)

.PHONY: clean-groufix
clean-groufix:
	$(MAKE) -C groufix clean
	rm -f $(OUT)/libgroufix.so

.PHONY: clean-all
clean-all: clean clean-groufix

-include $(DEPS)
