.PHONY: all arm host driver clean help

all: arm host

arm:
	$(MAKE) -C app arm

host:
	$(MAKE) -C app host

driver:
	$(MAKE) -C driver

clean:
	$(MAKE) -C app clean

help:
	@echo "Embedded Linux Assignment 02"
	@echo ""
	@echo "Available targets:"
	@echo "  make arm     - Build static ARM application"
	@echo "  make host    - Build x86 host application"
	@echo "  make driver  - Build ARM kernel module"
	@echo "  make all     - Build ARM and host applications"
	@echo "  make clean   - Remove application binaries"
