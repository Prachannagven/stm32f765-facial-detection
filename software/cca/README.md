Camera -> Image -> Crop to just face -> Multiply by w_l -> compare to features -> if confidence above threshold say yes, otherwise no

Works using CMake. Required as a dependency.

Read the makefile to actually do things. Run them all from root:
1. `make` to perform training and inference
2. `make inference` to just perform infernece after updating c code
3. `make train` to perform just training and update the header.
