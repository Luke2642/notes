# Fedora 42 Development Environment Setup - Some wheels, some building from source.

I enjoyed Ubuntu 22.04 for a while, it's widely used so good package and wheel availibility. However, I recently found Fedora 42 with KDE is a much nicer out of the box experience. These instructions set up my workstation in about 1 hour and avoids the little papercuts and install all the things essential as of August 2025 for some basic development on ComfyUI.

For a workstation setup it has key quality of life advantages such as fractional scaling on the display, and btrfs by default, making snapshotting and rolling back a doddle.

Instructions for a Fresh install of Fedora 42 Plasma Desktop (https://fedoraproject.org/kde/) with ComfyUI essentials:

- Nvidia Drivers 575.64.05
- Cuda Toolkit 12.8
- pytorch 2.8.0, (torch, torchvision) 
- ComfyUI with xformers, triton, sageattention 2+, flash attention, nunchaku

Fedora 42 comes with Python 3.13 but it seems to work just fine with everything, although it does mean building xformers.

# Resources

https://fedoraproject.org/kde/download

https://rpmfusion.org/Howto/NVIDIA

https://github.com/thu-ml/SageAttention

https://github.com/Dao-AILab/flash-attention

https://github.com/nunchaku-tech/nunchaku

https://github.com/facebookresearch/xformers


After Fedora installation, during the first welcome screen, choose **Enable non-free repositories**, which enables RPM Fusion. The following commands to install Nvidia drivers then gets the latest 575.64.05:

```
sudo dnf update
sudo dnf install akmod-nvidia
sudo dnf install xorg-x11-drv-nvidia-cuda
```

There's no Cuda 12.8 toolkit for Fedora 42, but the Fedora 41 version works fine. First a 6Gb download, then install:

```
wget https://developer.download.nvidia.com/compute/cuda/12.8.1/local_installers/cuda-repo-fedora41-12-8-local-12.8.1_570.124.06-1.x86_64.rpm
sudo rpm -i cuda-repo-fedora41-12-8-local-12.8.1_570.124.06-1.x86_64.rpm
sudo dnf clean all
sudo dnf -y install cuda-toolkit-12-8
```

Then get some essentials. Don't use pip, remove it, and use uv. It's built in Rust, is faster and has better caching and dependency resolution:

```
sudo dnf install git
sudo dnf install uv
sudo dnf install cmake
sudo dnf install python3-devel
sudo dnf install python3-cython
sudo dnf remove python3-pip
sudo dnf install ninja-build
```

These are needed for building some cuda 12.8 dependent repos:

```
sudo dnf install gcc14
sudo dnf install gcc14-c++
```

I put projects in /home/github to keep it tidy:

```
mkdir github
cd github
git clone https://github.com/comfyanonymous/ComfyUI.git
cd ComfyUI/
uv venv
source .venv/bin/activate
uv pip install torch==2.8.0 torchvision torchaudio --index-url https://download.pytorch.org/whl/cu128
uv pip install ninja
uv pip install packaging wheel
```

Then, I modify the requirements.txt to include at the top:

```
--extra-index-url https://download.pytorch.org/whl/cu128
```

Then, to build anything useful like sage attention with cuda 12.8 on fedora 42, we must first modify a bad cuda header file that Nvidia really should fix, but hasn't. Plenty of forum posts about this, but not easy to find. Add:

```
 noexcept (true)
```

to the end of each of the cospi sinpi cospif sinpif function definitions, before the semicolon in:

```
/usr/local/cuda-12.8/targets/x86_64-linux/include/crt/math_functions.h
```

Fedora 42 comes with gcc-15 and g++-15 so we need that gcc 14 we installed earlier to be temporarily enabled:

```
export CC=/usr/bin/gcc-14
export CXX=/usr/bin/g++-14
```

Then we can proceed with building sage attention. Inside the comfyui folder, with the .venv activated:

```
git clone https://github.com/thu-ml/SageAttention.git
cd SageAttention
uv pip install -e . --no-build-isolation
```

Now all the requirements are met, flash attention is now a one line build:

```
uv pip install flash-attn --no-build-isolation
```

Building the latest Nunchaku backend from source "just works" too, and only takes a few minutes as of August 2025:

```
git clone https://github.com/nunchaku-tech/nunchaku.git
cd nunchaku
git submodule init
git submodule update
uv pip install -e . --no-build-isolation
```

This does take a while. Next time I should investigate if there is a faster way to build only for your GPU:

```

CUDA_VISIBLE_DEVICES=0 NUNCHAKU_INSTALL_MODE=FAST uv pip install -v -e . --no-build-isolation

or perhaps

# For a sm_120 (Blackwell: RTX 5090)
TORCH_CUDA_ARCH_LIST="12.0" uv pip install -v -e . --no-build-isolation

# For a sm_89 (Ada: RTX 4090)
TORCH_CUDA_ARCH_LIST="8.9" uv pip install -v -e . --no-build-isolation

# For a sm_86 (Ampere: RTX 3090, A6000) 
TORCH_CUDA_ARCH_LIST="8.6" uv pip install -v -e . --no-build-isolation

# For a sm_80 (Ampere: A100)
TORCH_CUDA_ARCH_LIST="8.0" uv pip install -v -e . --no-build-isolation

# For a sm_75 (Turing: RTX 2080)
TORCH_CUDA_ARCH_LIST="7.5" uv pip install -v -e . --no-build-isolation
```

Similarly xformers builds fine too, they suggest the all in one command instead of cloning it into a subfolder and building:

```
uv pip install -v --no-build-isolation -U git+https://github.com/facebookresearch/xformers.git@main#egg=xformers
```

But you can also do it step by step, it works the same:

```
git clone https://github.com/facebookresearch/xformers.git
cd xformers
git submodule update --init --recursive
uv pip install -e . --no-build-isolation
```

Then finally, check your work:

```
uv pip list
```

And you should see all the latest packages:

```
Using Python 3.13.5 environment at: /home/luke/github/ComfyUI/.venv
Package                    Version                    
-------------------------- -------------------------- 
accelerate                 1.10.0
aiohappyeyeballs           2.6.1
aiohttp                    3.12.15
aiosignal                  1.4.0
alembic                    1.16.4
annotated-types            0.7.0
attrs                      25.3.0
av                         15.0.0
certifi                    2022.12.7
cffi                       1.17.1
charset-normalizer         2.1.1
comfyui-embedded-docs      0.2.4
comfyui-frontend-package   1.24.4
comfyui-workflow-templates 0.1.52
diffusers                  0.34.0
einops                     0.8.1
filelock                   3.18.0
flash-attn                 2.8.2
frozenlist                 1.7.0
fsspec                     2025.7.0
greenlet                   3.2.4
hf-xet                     1.1.7
huggingface-hub            0.34.4
idna                       3.4
importlib-metadata         8.7.0
jinja2                     3.1.6
kornia                     0.8.1
kornia-rs                  0.1.9
lxml                       6.0.0
mako                       1.3.10
markupsafe                 3.0.2
mpmath                     1.3.0
multidict                  6.6.3
networkx                   3.5
ninja                      1.11.1.4
numpy                      2.3.2
nunchaku                   0.3.2.dev20250808+torch2.8 
nvidia-cublas-cu12         12.8.4.1
nvidia-cuda-cupti-cu12     12.8.90
nvidia-cuda-nvrtc-cu12     12.8.93
nvidia-cuda-runtime-cu12   12.8.90
nvidia-cudnn-cu12          9.10.2.21
nvidia-cufft-cu12          11.3.3.83
nvidia-cufile-cu12         1.13.1.3
nvidia-curand-cu12         10.3.9.90
nvidia-cusolver-cu12       11.7.3.90
nvidia-cusparse-cu12       12.5.8.93
nvidia-cusparselt-cu12     0.7.1
nvidia-nccl-cu12           2.27.3
nvidia-nvjitlink-cu12      12.8.93
nvidia-nvtx-cu12           12.8.90
packages                   0.1.0
packaging                  25.0
pillow                     11.0.0
propcache                  0.3.2
protobuf                   6.31.1
psutil                     7.0.0
pycparser                  2.22
pydantic                   2.11.7
pydantic-core              2.33.2
pydantic-settings          2.10.1
python-dotenv              1.1.1
pyyaml                     6.0.2
regex                      2025.7.34
requests                   2.28.1
safetensors                0.6.1
sageattention              2.2.0                      
scipy                      1.16.1
sentencepiece              0.2.0
setuptools                 80.9.0
soundfile                  0.13.1
spandrel                   0.4.1
sqlalchemy                 2.0.42
sympy                      1.14.0
tokenizers                 0.21.4
torch                      2.8.0+cu128
torchaudio                 2.8.0+cu128
torchsde                   0.2.6
torchvision                0.23.0+cu128
tqdm                       4.66.5
trampoline                 0.1.2
transformers               4.55.0
triton                     3.4.0
typing-extensions          4.14.1
typing-inspection          0.4.1
urllib3                    1.26.13
wheel                      0.45.1
xformers                   0.0.32+635a8ab2.d20250808
yarl                       1.20.1
zipp                       3.23.0
```

# Ditch Firefox and install Brave:

```
curl -fsS https://dl.brave.com/install.sh | sh
```

Start the sync chain on Brave on your phone, then on the desktop choose settings > Sync > I have a sync code

Disable that annoying accidental tab mute button: settings > search > mute > disable mute on tab

And set tabs to vertical, because screens are wider than tall, and page titles are long.
