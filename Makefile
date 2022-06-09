OUT = build

SRCS = \
	$(wildcard src/*.cc)

CXXFLAGS += \
	-std=c++20 -Wall -Wextra \
	-isystem groufix/include -Isrc

LDFLAGS += -L$(OUT)
LDLIBS += -lgroufix

OBJS = $(patsubst %,$(OUT)/%.o,$(SRCS))
DEPS = $(patsubst %,$(OUT)/%.d,$(SRCS))

$(OUT)/fiezta: $(OUT)/libgroufix.so $(OBJS)
	@mkdir -p $(@D)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

groufix/.shaderc-deps.stamp:
	cd groufix/deps/shaderc && ./utils/git-sync-deps
	touch $@

$(OUT)/libgroufix.so: groufix/.shaderc-deps.stamp
	@mkdir -p $(@D)
	$(MAKE) -C groufix unix
	cp groufix/bin/unix/libgroufix.so $@
	@# TODO: Make this cross-platform
	install_name_tool -id '@executable_path/libgroufix.so' $@

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
clean-all: clean
	$(MAKE) -C groufix clean-all

-include $(DEPS)
