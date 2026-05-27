# Introduction
The goal of this project is to build a 50x50mm drone, capable of autonomous flight, and recognizing humans. The final product should be able to fly towards the target.

For current protyping, I'm looking at using an OV7670 camera, which supplies up to a 320x260 frame, based on the qvga output. However, I can go as low as the qcif resolution (176x144), although I'm not sure recognition will work well on that.

# Links to Look at
[OV7670 Interfacing][https://www.youtube.com/watch?v=aanNau1RDTA]

# Hardware Design
I use an STM32F765VIT6 due to its on board DCMI block, as well as its ART accelerator to speed up computations. It pairs with a TOF sensor, a BMP280 barometer, IMU and magnetic field sensor for full attitude determination.

Four brushed DC motors are used for this system, allowing for lightweight control. The only limitation here is how heavy the entire system is going to be. It can carry approximately 100-200 grams, which is well within our weight budget.
