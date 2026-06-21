import os
import shutil

import numpy as np
from PIL import Image
from scipy.linalg import eigh

name = "Pramit"

# Getting all the image names
pramit_lr_lst = sorted(
    os.listdir(
        "/home/pranav/PersonalProjects/kamikaze-drone/software/cca/python-training/original-split/s01_pramit/LR/"
    )
)
pramit_hr_lst = sorted(
    os.listdir(
        "/home/pranav/PersonalProjects/kamikaze-drone/software/cca/python-training/original-split/s01_pramit/HR/"
    )
)

# Intializing Empty arrays for the HR and LR sections
HR = []
LR = []

# Iterating over the tuples in both the lists to generate X(HR) and Y(LR) as per the paper
for lr_img, hr_img in zip(pramit_lr_lst, pramit_hr_lst):
    hr = np.array(
        Image.open(
            os.path.join(
                "/home/pranav/PersonalProjects/kamikaze-drone/software/cca/python-training/original-split/s01_pramit/HR/",
                hr_img,
            )
        ).convert("L")
    )
    lr = np.array(
        Image.open(
            os.path.join(
                "/home/pranav/PersonalProjects/kamikaze-drone/software/cca/python-training/original-split/s01_pramit/LR/",
                lr_img,
            )
        ).convert("L")
    )
    HR.append(hr.flatten())
    LR.append(lr.flatten())
# Pushing them into the array format for future working
HR = np.array(HR, dtype="i")
LR = np.array(LR, dtype="i")


# Calculating the Covariance Matrices
hr_mean = np.mean(HR, axis=0)
lr_mean = np.mean(LR, axis=0)
HR_centered = HR - hr_mean
LR_centered = LR - lr_mean
N = HR.shape[0]

S_XY = (HR_centered.T @ LR_centered) / N
S_YX = (LR_centered.T @ HR_centered) / N
S_XX = (HR_centered.T @ HR_centered) / N
S_YY = (LR_centered.T @ LR_centered) / N

# Getting Projection Vectors by calculating Eigenvectors and Eigenvalues
M_XY = np.linalg.inv(S_XX) @ S_XY @ np.linalg.inv(S_YY) @ S_YX
M_YX = np.linalg.inv(S_YY) @ S_YX @ np.linalg.inv(S_XX) @ S_XY

_, Wh = eigh(M_XY)
_, Wl = eigh(M_YX)

# Ignoring the lower eigenvectors and just taking the biggest one
k = 50
Wh = Wh[:, -k:]
Wl = Wl[:, -k:]

# Generating the actually important stuff
features = LR_centered @ Wl
centroid = np.mean(features, axis=0)
threshold = np.mean(np.linalg.norm(features - centroid, axis=1)) + 3 * np.std(
    np.linalg.norm(features - centroid, axis=1)
)

# Exporting the stuff to be used for C inference
if os.path.isdir("./c-inference/inc"):
    shutil.rmtree("./c-inference/inc/", ignore_errors=False)
    os.makedirs("./c-inference/inc/")
else:
    os.makedirs("./c-inference/inc/")

np.save("./c-inference/inc/lr_mean.npy", lr_mean)
np.save("./c-inference/inc/Wl.npy", Wl)
np.save("./c-inference/inc/features.npy", features)
np.save("./c-inference/inc/threshold.npy", threshold)

# Making a C Header File That Can be called in the C code
f = open("./c-inference/inc/cca_defs.h", "w")
f.write("#ifndef CCA_DEFS_H \n #define CCA_DEFS_H \n\n")

f.write(f"const static float threshold = {threshold};\n")

f.write(f"static const float lr_mean[{len(lr_mean.flatten())}] = {{")
for x in lr_mean:
    f.write(f"{x}, ")
f.write("\n};")

f.write(f"static const float Wl[{len(Wl)}][{len(Wl[0])}] = {{")
for x in Wl:
    f.write(" {")
    f.write(",".join(f"{val:.8f}f" for val in x))
    f.write("},\n")
f.write("\n};")

f.write(f"static const float features[{len(features)}][{len(features[0])}] = {{")
for x in features:
    f.write(" {")
    f.write(",".join(f"{val:.8f}f" for val in x))
    f.write("},\n")
f.write("\n};")

f.write("\n #endif")
f.close()

# Completion Message
print(f"Completed CCA Training on {name}'s face. Parameters and important stats below:")
print(f"\tName: {name}")
print(f"\tNumber of Eigenvalues Taken: {k}")
print(f"\tThreshold: {threshold}")
print(f"\tLR_Mean Dimensions: {lr_mean.shape}")
print(f"\tWl Dimensions: {Wl.shape}")
print(f"\tFeatures shape: {features.shape}")
