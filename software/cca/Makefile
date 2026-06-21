C_BUILD_DIR        := ./c-inference/build
PYTHON_OUTPUTS_DIR  := ./c-inference/inc
PYTHON_OUTPUT_HEADER := $(PYTHON_OUTPUTS_DIR)/cca_defs.h
COMPILE_COMMANDS    := $(C_BUILD_DIR)/compile_commands.json

all: train inference

train: $(PYTHON_OUTPUT_HEADER)

$(PYTHON_OUTPUT_HEADER): ./python-training/python-training.py
	python3 ./python-training/python-training.py

inference: $(PYTHON_OUTPUT_HEADER)
	cmake -B $(C_BUILD_DIR)
	cmake --build $(C_BUILD_DIR)
	$(C_BUILD_DIR)/cca_inference
	ln -sf $(C_BUILD_DIR)/compile_commands.json .

compdb: $(C_BUILD_DIR)/compile_commands.json
	ln -sf $(C_BUILD_DIR)/compile_commands.json .


clean_inference:
	rm -rf $(C_BUILD_DIR)

clean_train:
	rm -rf $(PYTHON_OUTPUTS_DIR) 

clean: clean_inference clean_train

.PHONY: all train inference compdb clean_inference clean_train clean
