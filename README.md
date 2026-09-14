# Fedora 44 Development Environment Setup - Some wheels, some building from source.

I enjoyed Ubuntu 22.04 for a while, it's widely used so good package and wheel availibility. However, I've since found Fedora 44 with KDE is a much nicer out of the box experience. These instructions set up my workstation in about 1 hour and avoids the little papercuts and installs all the things essential as of September 2026 for some basic development on ComfyUI.

For a workstation setup it has key quality of life advantages such as fractional scaling on the display, and btrfs by default, making snapshotting and rolling back a doddle.

Instructions for a Fresh install of Fedora 44 Plasma Desktop (https://fedoraproject.org/kde/) with ComfyUI essentials:

- Nvidia Drivers 610.57.04 - now installs in one click from "Discover" App.
- Cuda Toolkit 13.4 
- pytorch 2.14.0, (torch, torchvision, torchaudio) 
- ComfyUI with xformers, triton, sageattention 2.2.0, flash attention

Fedora 44 comes with Python 3.14 but it's easy enough to create a venv with 3.12 for maximum compatibility and probably no real performance hit.

# Resources

https://fedoraproject.org/kde/download

https://rpmfusion.org/Howto/NVIDIA

https://github.com/thu-ml/SageAttention

https://github.com/Dao-AILab/flash-attention


After Fedora installation, during the first welcome screen, choose **Enable non-free repositories**, which enables RPM Fusion. 

Then open the discover app and under Hardware Drivers, install the Nvidia drivers which gets the latest 610.57.04 as of time of writing.

Add the repo for the Nvidia toolkit:

```bash
sudo dnf config-manager addrepo \
  --from-repofile=https://developer.download.nvidia.com/compute/cuda/repos/fedora44/x86_64/cuda-fedora44.repo
```

Install the toolkit only. The package `cuda-toolkit-13-4` excludes the NVIDIA driver packages and avoids a conflict with RPM Fusion.

```bash
sudo dnf install cuda-toolkit-13-4
```


Add these 3 lines to `~/.bashrc`, then open a new shell.

```bash
export CUDA_HOME=/usr/local/cuda-13.4
export PATH=$CUDA_HOME/bin:$PATH
export LD_LIBRARY_PATH=$CUDA_HOME/lib64:$LD_LIBRARY_PATH
```

Verify.

```bash
nvcc --version
nvidia-smi
```

CUDA 13.4 accepts GCC 16 as a host compiler. Fedora 44 needs no compatibility compiler.

## 3. Build tools

```bash
sudo dnf install git uv cmake ninja-build python3-devel gcc gcc-c++
```

## 4. Environment

Fedora 44 defaults to Python 3.14. Some CUDA dependencies publish no cp314 wheels, so `uv` must fetch Python 3.12.

I put projects in /home/github to keep it tidy:

```bash
uv python install 3.12
mkdir github
cd github
git clone https://github.com/comfyanonymous/ComfyUI.git
cd ComfyUI/
uv venv --python 3.12
source .venv/bin/activate
uv pip install torch torchvision --index-url https://download.pytorch.org/whl/cu130
uv pip install ninja packaging wheel setuptools
uv pip install -r requirements.txt
```

Then we can proceed with building sage attention. Inside the comfyui folder, with the .venv activated, we modify sage attention to be happy with gcc v20 instead of 17:

```
git clone https://github.com/thu-ml/SageAttention.git
cd SageAttention
sed -i 's/-std=c++17/-std=c++20/g' setup.py
grep -n "std=c++" setup.py
export EXT_PARALLEL=4
export NVCC_APPEND_FLAGS="--threads 8"
export MAX_JOBS=8
export TORCH_CUDA_ARCH_LIST="8.9"
uv pip install -e . --no-build-isolation
```

Flash attention is a similar process, with the .venv activated, which takes ~30 mins or so:

```
git clone https://github.com/Dao-AILab/flash-attention.git
cd flash-attention
sed -i 's/c++17/c++20/g' setup.py
grep -n "c++2" setup.py
export FLASH_ATTN_CUDA_ARCHS="80"
export MAX_JOBS=8
export NVCC_THREADS=4
uv pip install flash-attn --no-build-isolation
```

It's much easier to install xformers now, just uv pip install xformers, if you need it.

Then finally, check your work:

```
uv pip list
```

And you should see all the latest packages:

```
Using Python 3.12.14 environment at: ComfyUI/.venv
Package                Version
---------------------- ------------
cuda-bindings          13.3.1
cuda-pathfinder        1.6.0
cuda-toolkit           13.0.3.0
einops                 0.8.2
filelock               3.32.3
flash-attn             2.8.3.post1
fsspec                 2026.7.0
jinja2                 3.1.6
markupsafe             3.0.3
mpmath                 1.3.0
networkx               3.6.1
ninja                  1.13.2
numpy                  2.5.2
nvidia-cublas          13.1.1.3
nvidia-cuda-cupti      13.0.85
nvidia-cuda-nvrtc      13.0.88
nvidia-cuda-runtime    13.0.96
nvidia-cudnn-cu13      9.24.0.43
nvidia-cufft           12.0.0.61
nvidia-cufile          1.15.1.6
nvidia-curand          10.4.0.35
nvidia-cusolver        12.0.4.66
nvidia-cusparse        12.6.3.3
nvidia-cusparselt-cu13 0.8.1
nvidia-nccl-cu13       2.30.7
nvidia-nvjitlink       13.3.33
nvidia-nvshmem-cu13    3.4.5
nvidia-nvtx            13.0.85
packaging              26.3
pillow                 12.3.0
sageattention          2.2.0
setuptools             78.1.0
sympy                  1.14.0
torch                  2.14.0+cu130
torchvision            0.29.0+cu130
triton                 3.8.0
typing-extensions      4.16.0
wheel                  0.48.0
xformers               0.0.35
```

# Ditch Firefox and install Brave:

```
curl -fsS https://dl.brave.com/install.sh | sh
```

Start the sync chain on Brave on your phone, then on the desktop choose settings > Sync > I have a sync code

- Disable that annoying accidental tab mute button: settings > search > mute > disable mute on tab
- And set tabs to vertical, because screens are wider than tall, and page titles are long.
- use wide address bar
- show full urls
