# Fedora Development Environment Setup

A collection of my development environment notes and solutions, thrown in one big Readme.md

I enjoyed Ubuntu 22.04 for a while as it is so widely used as a base, so good package and wheel availibility. But Fedora 42 with KDE is a much nicer out of the box experience. These instructions set up my workstation in about 1 hour and avoids the little papercuts and install all the things essential as of August 2025 for some basic development on ComfyUI.

For a workstation setup it has key quality of life advantages such as fractional scaling on the display, and btrfs by default, making snapshotting and rolling back a doddle.

Instructions for a Fresh install of Fedora 42 Plasma Desktop (https://fedoraproject.org/kde/) with ComfyUI essentials:

- Nvidia Drivers 575.64.05
- Cuda Toolkit 12.8
- torch, torchvision, xformers, triton
- ComfyUI with sageattention 2+, flash attention

Fedora 42 comes with Python 3.13 but it seems to work just fine with everything.

# Resources

https://fedoraproject.org/kde/download

https://rpmfusion.org/Howto/NVIDIA

https://github.com/thu-ml/SageAttention

https://github.com/Dao-AILab/flash-attention

During the welcome screen, choose **Enable non-free repositories**, which enables RPM Fusion. The following commands to install Nvidia drivers then gets the latest 575.64.05:

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

Then get some essentials. Don't use pip, switch to uv. It's faster and has better caching and dependency resolution:

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
uv pip install torch torchvision xformers --index-url https://download.pytorch.org/whl/cu128
```

Then, I modify the requirements.txt to include at the top:

```
--extra-index-url --index-url https://download.pytorch.org/whl/cu128
```

I think I confused that one of the requirements for *packaging* with *packages* and that had some extra requirements, which can probably be skipped next time:

```
sudo dnf install libxml2-devel libxslt-devel
uv pip install packages lxml==6.0.0
```

So this is correct:

```
uv pip install packaging wheel
```

Then, to build sage attention with cuda 12.8 on fedora 42, you must modify a bad cuda header file that nvidia really should fix. Add:

```
noexcept (true)
```

to the end of each of the cospi sinpi cospif sinpif definitions in:

```
/usr/local/cuda-12.8/targets/x86_64-linux/include/crt/math_functions.h
```

Fedora 42 comes with gcc-15 and g++-15 so we need that gcc 14 we installed earlier to be temporarily enabled:

```
export CC=/usr/bin/gcc-14
export CXX=/usr/bin/g++-14
```

Then we can proceed with building sage attention. So this will then work:

```
git clone https://github.com/thu-ml/SageAttention.git
cd SageAttention
uv pip install -e . --no-build-isolation
```

Now all the requirements are met, flash attention is now a one line build:

```
uv pip install flash-attn --no-build-isolation
```

And finally obviously ditch firefox and install brave:

```
curl -fsS https://dl.brave.com/install.sh | sh
```

Set up sync with your phone, settings > Sync > I have a sync code

Disable that annoying accidental mute button: settings > search > mute > disable mute on tab

And set tabs to vertical, because screens are wider than tall, and page titles are long.



