# Source files and includes common for all targets
RUUVI_PRJ_SOURCES= \
  $(PROJ_DIR)/ruuvi_library_peak2peak.c \
  $(PROJ_DIR)/ruuvi_library_ringbuffer_test.c \
  $(PROJ_DIR)/ruuvi_library_ringbuffer.c \
  $(PROJ_DIR)/ruuvi_library_rms.c \
  $(PROJ_DIR)/ruuvi_library_test_analysis.c \
  $(PROJ_DIR)/ruuvi_library_test.c \
  $(PROJ_DIR)/ruuvi_library_variance.c \
  $(PROJ_DIR)/ruuvi_library_compress.c \
  $(PROJ_DIR)/ruuvi_library_compress_test.c 

RUUVI_LIB_SOURCES= \
  $(PROJ_DIR)liblzf-3.6/lzf_c.c \
  $(PROJ_DIR)liblzf-3.6/lzf_d.c

COMMON_SOURCES= \
  $(RUUVI_LIB_SOURCES) \
  $(RUUVI_PRJ_SOURCES) \

COMMON_INCLUDES= \
  $(PROJ_DIR) \
