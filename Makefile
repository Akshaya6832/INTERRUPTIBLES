PLATFORM ?= aarch64le
BUILD_PROFILE ?= debug
PROJECTS = TRINETRA_SENSOR_INGEST TRINETRA_VALIDATOR TRINETRA_HAZARD_FUSION TRINETRA_WARNING_GOVERNOR TRINETRA_ALERT TRINETRA_SENSOR_SIM

all:
	@for p in $(PROJECTS); do echo "=== $$p ==="; $(MAKE) -C $$p PLATFORM=$(PLATFORM) BUILD_PROFILE=$(BUILD_PROFILE) all || exit $$?; done

clean:
	@for p in $(PROJECTS); do $(MAKE) -C $$p clean; done

rebuild: clean all

.PHONY: all clean rebuild
