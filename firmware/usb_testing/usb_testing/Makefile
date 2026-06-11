all:
	stm32_cmake_build.sh
	stm32_project_flash.sh
	sleep 3
	screen /dev/ttyACM1

all_old:
	stm32_cmake_build.sh
	stm32_project_flash.sh
	sleep 3
	python ./Python/reciever_old.py

flash:
	stm32_cmake_build.sh
	stm32_project_flash.sh

