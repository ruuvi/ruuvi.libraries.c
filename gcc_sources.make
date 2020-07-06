# Source files and includes common for all targets
RUUVI_PRJ_SOURCES= \
  $(PROJ_DIR)/ruuvi_library_peak2peak.c \
  $(PROJ_DIR)/ruuvi_library_ringbuffer_test.c \
  $(PROJ_DIR)/ruuvi_library_ringbuffer.c \
  $(PROJ_DIR)/ruuvi_library_rms.c \
  $(PROJ_DIR)/ruuvi_library_test_analysis.c \
  $(PROJ_DIR)/ruuvi_library_test.c \
  $(PROJ_DIR)/ruuvi_library_variance.c

COMMON_SOURCES= \
  $(RUUVI_PRJ_SOURCES) \

COMMON_INCLUDES= \
  $(PROJ_DIR) \
