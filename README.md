# WebRTC Hello World - C++ Example

A simple WebRTC application demonstrating peer-to-peer connection and data channel communication using Google's native WebRTC library on Linux with depot_tools bundled Clang.

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Prerequisites](#prerequisites)
- [Installation](#installation)
- [Building WebRTC](#building-webrtc)
- [Building the Project](#building-the-project)
- [Running the Application](#running-the-application)
- [Project Structure](#project-structure)
- [Troubleshooting](#troubleshooting)
- [Common Issues and Solutions](#common-issues-and-solutions)
- [Migration to New API](#migration-to-new-api)
- [License](#license)

## Overview

This project demonstrates the basic concepts of WebRTC by creating two peer connections that communicate through a data channel. It simulates a local peer-to-peer connection where both peers run in the same application, exchanging "Hello World" messages.

**Important**: This example uses the **depot_tools bundled Clang** to avoid architecture compatibility issues between the WebRTC build and your application.

## Features

- ✅ Creates two WebRTC peer connections locally
- ✅ Establishes peer-to-peer data channel
- ✅ Exchanges SDP offers and answers
- ✅ Handles ICE candidate gathering and exchange
- ✅ Sends and receives text messages via data channel
- ✅ Uses depot_tools LLVM toolchain (Clang + lld linker)
- ✅ Proper memory management with smart pointers
- ✅ Modern C++17

## Prerequisites

### System Requirements

- **OS**: Linux (Ubuntu 20.04+ recommended)
- **RAM**: 16GB minimum (for building WebRTC)
- **Disk Space**: 100GB+ free space
- **CPU**: Multi-core processor (building takes time)

### Software Requirements

- **Git**: Latest version
- **Python**: 3.8 or later (required by depot_tools)
- **depot_tools**: Google's build tools (includes Clang, Ninja, GN)

**Note**: You do NOT need to install system Clang or CMake separately - depot_tools provides everything needed.

## Installation

### Step 1: Install Basic System Dependencies

```bash
sudo apt update
sudo apt install -y git python3 python3-pip curl \
  libgtk-3-dev libx11-dev libxss1 libnss3-dev libasound2-dev \
  libglu1-mesa-dev libxrandr-dev libxcomposite-dev libxdamage-dev \
  libxfixes-dev libxext-dev
```

### Step 2: Create Workspace Structure

```bash
# Create main workspace directory
mkdir -p ~/workspace
cd ~/workspace
```

Your final structure will be:
```
~/workspace/
├── depot_tools/              # Google's build tools
├── webrtc_checkout/
│   └── src/                  # WebRTC source code
└── webrtc_simple_example/    # This project
    ├── main.cpp
    ├── CMakeLists.txt
    └── ...
```

### Step 3: Install depot_tools

```bash
cd ~/workspace
git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git

# Add to PATH permanently
echo 'export PATH="$HOME/workspace/depot_tools:$PATH"' >> ~/.bashrc
source ~/.bashrc

# Verify installation
which gclient
which gn
which ninja
```

### Step 4: Create Python Symlink (Required)

depot_tools expects `python` to be available:

```bash
# Check if python3 exists
which python3

# Create symlink
sudo ln -s /usr/bin/python3 /usr/bin/python

# Verify
python --version
```

### Step 5: Create vpython Symlink (Required)

```bash
cd ~/workspace/depot_tools
ln -s vpython3 vpython

# Verify
ls -la | grep vpython
```

## Building WebRTC

This is the most time-consuming step. Plan for 2-4 hours on first build.

### Step 1: Fetch WebRTC Source

```bash
cd ~/workspace
mkdir webrtc_checkout
cd webrtc_checkout

# Fetch WebRTC (downloads ~10GB)
fetch --nohooks webrtc
cd src

# Switch to a stable branch (recommended)
git checkout branch-heads/7151

# Sync dependencies (30-60 minutes)
gclient sync
```

### Step 2: Configure WebRTC Build

**Recommended GN Args**:

```bash
cd ~/workspace/webrtc_checkout/src

# Clean any previous builds
rm -rf out/Default

# Configure build
gn gen out/Default --args='
target_os = "linux"
target_cpu = "x64"
is_debug = true
rtc_include_tests = false
rtc_use_h264 = true
ffmpeg_branding = "Chrome"
is_component_build = false
use_rtti = true
use_custom_libcxx = false
rtc_enable_protobuf = false
'
```

**GN Args Explanation**:
- `target_os = "linux"` - Build for Linux
- `target_cpu = "x64"` - 64-bit architecture
- `is_debug = true` - Debug build with symbols
- `use_rtti = true` - Enable RTTI (required by some applications)
- `use_custom_libcxx = false` - Use system libstdc++ to avoid compatibility issues
- `rtc_enable_protobuf = false` - Disable protobuf if not needed
- `rtc_use_h264 = true` - Enable H.264 codec support
- `ffmpeg_branding = "Chrome"` - Enable additional codecs

### Step 3: Build WebRTC Core

```bash
# Build core WebRTC (2-4 hours on first build)
ninja -C out/Default

# If you have limited RAM, limit parallel jobs:
# ninja -C out/Default -j4
```

### Step 4: Build Video Codec Factories

**Important**: As of recent WebRTC versions, the `rtc_include_builtin_video_codecs` flag has been removed (see [WebRTC Issue #42223784](https://issues.webrtc.org/issues/42223784#comment26)).

You must explicitly build the video encoder/decoder factory targets:

```bash
cd ~/workspace/webrtc_checkout/src

# Build video encoder factory
ninja -C out/Default api/video_codecs:builtin_video_encoder_factory

# Build video decoder factory
ninja -C out/Default api/video_codecs:builtin_video_decoder_factory

# Build audio codec factories (if needed)
ninja -C out/Default api/audio_codecs:builtin_audio_encoder_factory
ninja -C out/Default api/audio_codecs:builtin_audio_decoder_factory
```

### Step 5: Run ranlib on All Archives

This fixes "archive has no index" linker errors:

```bash
cd ~/workspace/webrtc_checkout/src/out/Default/obj
find . -name "*.a" -exec ranlib {} \;
```

### Step 6: Verify Video Codec Factory Symbols

Confirm the video encoder/decoder factories were built:

```bash
cd ~/workspace/webrtc_checkout/src/out/Default
find obj -name "*.a" -exec nm {} \; 2>/dev/null | grep "CreateBuiltinVideoEncoder"
```

You should see output like:
```
0000000000000000 T _ZN6webrtc32CreateBuiltinVideoEncoderFactoryEv
0000000000000000 T _ZN6webrtc32CreateBuiltinVideoDecoderFactoryEv
```

If you don't see these symbols, go back to Step 4 and run the explicit ninja commands.

## Building the Project

### Step 1: Clone This Repository

```bash
cd ~/workspace
git clone <your-repository-url> webrtc_simple_example
cd webrtc_simple_example
```

### Step 2: Configure with CMake

The project automatically uses depot_tools' LLVM toolchain. You can override paths if needed:

**Default build** (uses paths relative to project location):
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

**Custom paths**:
```bash
mkdir build && cd build
cmake .. \
  -DLLVM_PATH=/path/to/custom/llvm \
  -DWEBRTC_ROOT=/path/to/webrtc/src \
  -DWEBRTC_BUILD_DIR=/path/to/webrtc/out/Default
make -j$(nproc)
```

### Step 3: Build

```bash
make -j$(nproc)
```

## Running the Application

```bash
# From build directory
./webrtcexample

# Or from project root
./build/webrtcexample
```

### Expected Output

```
WebRTC Hello World - Linux Example
===================================
PeerConnectionFactory created successfully
PeerConnections created successfully
Data channel created: hello_channel
Creating offer...
SDP creation successful: offer
Exchanging offer and creating answer...
SDP creation successful: answer
✅ WebRTC connection established successfully!
[Peer1] Data channel state: Open
[Peer2] Data channel state: Open
[Peer1] Sent: Hello from Peer1!
[Peer2] Sent: Hello from Peer2!
[Peer2] Received: Hello from Peer1!
[Peer1] Received: Hello from Peer2!
✅ Data channel communication successful!
```

## Project Structure

```
~/workspace/
├── depot_tools/                      # Google's build tools
│   ├── gclient
│   ├── gn
│   ├── ninja
│   └── ...
│
├── webrtc_checkout/
│   └── src/                          # WebRTC source
│       ├── api/
│       │   ├── audio_codecs/
│       │   │   ├── builtin_audio_encoder_factory.*
│       │   │   └── builtin_audio_decoder_factory.*
│       │   └── video_codecs/
│       │       ├── builtin_video_encoder_factory.*
│       │       └── builtin_video_decoder_factory.*
│       ├── modules/
│       ├── out/Default/              # Build output
│       │   └── obj/                  # Static libraries
│       └── third_party/
│           └── llvm-build/           # Bundled Clang toolchain
│               └── Release+Asserts/
│                   └── bin/
│                       ├── clang++
│                       ├── clang
│                       └── ld.lld    # LLVM linker
│
└── webrtc_simple_example/            # This project
    ├── main.cpp
    ├── data_channel_observer.cpp
    ├── data_channel_observer.h
    ├── simple_peer_connection_observer.cpp
    ├── simple_peer_connection_observer.h
    ├── sdp_observer.cpp
    ├── sdp_observer.h
    ├── local_signaling.cpp
    ├── local_signaling.h
    ├── CMakeLists.txt
    ├── README.md
    └── build/                        # Build output
        └── webrtcexample             # Executable
```

## Troubleshooting

### Build Issues

#### ❌ Error: `gclient: command not found`
**Cause**: depot_tools not in PATH  
**Solution**:
```bash
export PATH="$HOME/workspace/depot_tools:$PATH"
echo 'export PATH="$HOME/workspace/depot_tools:$PATH"' >> ~/.bashrc
source ~/.bashrc
```

#### ❌ Error: `[Errno 2] No such file or directory: 'python'`
**Cause**: Python symlink missing  
**Solution**:
```bash
sudo ln -s /usr/bin/python3 /usr/bin/python
python --version
```

#### ❌ Error: `[Errno 2] No such file or directory: 'vpython'`
**Cause**: vpython symlink missing in depot_tools  
**Solution**:
```bash
cd ~/workspace/depot_tools
ln -s vpython3 vpython
ls -la | grep vpython
```

#### ❌ Error: `undefined reference to CreateBuiltinVideoEncoderFactory()`
**Cause**: Video codec factory targets not built  
**Solution**: Build the specific targets explicitly:
```bash
cd ~/workspace/webrtc_checkout/src
ninja -C out/Default api/video_codecs:builtin_video_encoder_factory
ninja -C out/Default api/video_codecs:builtin_video_decoder_factory
```

Then verify the symbols exist:
```bash
find out/Default/obj -name "*.a" -exec nm {} \; 2>/dev/null | grep CreateBuiltinVideo
```

Run ranlib and rebuild your project:
```bash
cd out/Default/obj
find . -name "*.a" -exec ranlib {} \;
cd ~/workspace/webrtc_simple_example/build
make clean && make
```

#### ❌ Error: `unknown architecture of input file ... is incompatible with i386:x86-64`
**Cause**: System linker (`/usr/bin/ld`) can't understand depot_tools Clang output  
**Solution**: Ensure CMake uses depot_tools linker (automatically configured in CMakeLists.txt):
```cmake
set(CMAKE_EXE_LINKER_FLAGS "-fuse-ld=lld")
```

Check your build output shows:
```
-- Linker: lld (/.../llvm-build/Release+Asserts/bin/ld.lld)
```

If you see `/usr/bin/ld` being used, your CMakeLists.txt isn't configured correctly.

#### ❌ Error: `archive has no index; run ranlib to add one`
**Cause**: Static libraries missing symbol index  
**Solution**:
```bash
cd ~/workspace/webrtc_checkout/src/out/Default/obj
find . -name "*.a" -exec ranlib {} \;
```

Then rebuild your project.

#### ❌ Error: `GLIBCXX_3.4.30 not found`
**Cause**: Version mismatch between WebRTC's libstdc++ and system libraries  
**Solution**: Already handled in GN args with `use_custom_libcxx = false`. If still occurs:
```bash
# Check your system version
strings /usr/lib/x86_64-linux-gnu/libstdc++.so.6 | grep GLIBCXX | tail -5

# Update system
sudo apt update && sudo apt upgrade
```

### Runtime Issues

#### ❌ No output or program hangs
**Solution**: Check thread initialization
- Verify network, worker, and signaling threads start
- Enable verbose logging (see Debug section)

#### ❌ ICE connection fails
**Solution**:
- Check firewall settings
- Verify STUN server accessibility
- Check logs for ICE gathering errors

### Debug Mode

Enable verbose WebRTC logging by adding to `main.cpp`:

```cpp
#include "rtc_base/logging.h"

int main() {
    // Enable verbose logging
    rtc::LogMessage::LogToDebug(rtc::LS_VERBOSE);
    rtc::LogMessage::LogTimestamps();
    rtc::LogMessage::LogThreads();
    
    // Your code...
}
```

## Common Issues and Solutions

### Issue: Out of Memory During WebRTC Build

**Symptoms**: Build fails with errors, system becomes unresponsive  
**Solution**: Limit parallel jobs
```bash
# Use only 4 parallel jobs
ninja -C out/Default -j4

# Or based on RAM: 2GB RAM per job
ninja -C out/Default -j$(($(free -g | awk '/^Mem:/{print $2}')/2))
```

### Issue: Different WebRTC Branch

**If you need a different branch**:
```bash
cd ~/workspace/webrtc_checkout/src

# List available branches
git branch -r | grep branch-heads | tail -20

# Checkout desired branch
git checkout branch-heads/XXXX
gclient sync
```

**Recommended branches**:
- `branch-heads/7151` - Recent stable (M137, 2024)
- `branch-heads/5790` - M108 (2022) 
- `branch-heads/4324` - M88 (older, may lack builtin factory support)

### Issue: Need to Clean Everything

**Complete clean rebuild**:
```bash
# Clean WebRTC
cd ~/workspace/webrtc_checkout/src
rm -rf out/

# Clean your project
cd ~/workspace/webrtc_simple_example
rm -rf build/

# Start over from build steps
```

### Issue: Missing Codec Support

**If you need specific codecs**:

For **VP8/VP9**:
```bash
ninja -C out/Default modules/video_coding:webrtc_vp8
ninja -C out/Default modules/video_coding:webrtc_vp9
```

For **H.264** (requires `rtc_use_h264 = true`):
```bash
ninja -C out/Default modules/video_coding:webrtc_h264
```

For **AV1**:
```bash
ninja -C out/Default modules/video_coding/codecs/av1:av1_svc_config
```

## Migration to New API

### Deprecated: `CreateBuiltinVideoEncoderFactory()`

WebRTC developers are phasing out `CreateBuiltinVideoEncoderFactory()` in favor of the more flexible `VideoEncoderFactoryTemplate` API.

**Old API** (still works but deprecated):
```cpp
#include "api/video_codecs/builtin_video_encoder_factory.h"

auto encoder_factory = webrtc::CreateBuiltinVideoEncoderFactory();
```

**New API** (recommended):
```cpp
#include "api/video_codecs/video_encoder_factory_template.h"
#include "api/video_codecs/video_encoder_factory_template_libvpx_vp8_adapter.h"
#include "api/video_codecs/video_encoder_factory_template_libvpx_vp9_adapter.h"
#include "api/video_codecs/video_encoder_factory_template_open_h264_adapter.h"

auto encoder_factory = std::make_unique<webrtc::VideoEncoderFactoryTemplate<
    webrtc::LibvpxVp8EncoderTemplateAdapter,
    webrtc::LibvpxVp9EncoderTemplateAdapter,
    webrtc::OpenH264EncoderTemplateAdapter
>>();
```

**Benefits of new API**:
- More explicit codec selection
- Better control over codec configuration
- Easier to add custom encoders
- No need for separate build flags

**Migration steps**:
1. Replace includes
2. Switch to template-based factory
3. Explicitly list desired codecs
4. Update build targets if needed

For more details, see [WebRTC Issue #42223784](https://issues.webrtc.org/issues/42223784).

## Advanced Configuration

### Custom Compiler Flags

Edit `CMakeLists.txt`:
```cmake
target_compile_options(webrtcexample PRIVATE
    -O2                    # Optimization level
    -g                     # Debug symbols
    -DNDEBUG              # Disable asserts
)
```

### Link Against Specific WebRTC Libraries

Instead of all libraries:
```cmake
# In CMakeLists.txt, replace:
file(GLOB_RECURSE WEBRTC_LIBS ...)

# With specific libraries:
set(WEBRTC_LIBS
    ${WEBRTC_BUILD_DIR}/obj/libwebrtc.a
    ${WEBRTC_BUILD_DIR}/obj/api/video_codecs/libbuiltin_video_encoder_factory.a
    # Add only what you need...
)
```

## Next Steps

To extend this example:

1. **Add Audio/Video Streams**
   - Use `AudioDeviceModule` for audio capture
   - Add video tracks with `VideoTrackSource`

2. **Network Signaling**
   - Implement WebSocket server for SDP/ICE exchange
   - Enable connections between different machines

3. **TURN Server Integration**
   - Add TURN servers for NAT traversal
   - Configure in `RTCConfiguration`

4. **Migrate to New API**
   - Use `VideoEncoderFactoryTemplate` instead of builtin factories
   - Better codec control and configuration

5. **Error Recovery**
   - Implement reconnection logic
   - Handle ICE restart
   - Monitor connection quality

## Resources

- [WebRTC Native Code](https://webrtc.googlesource.com/src/)
- [WebRTC Native API Documentation](https://webrtc.github.io/webrtc-org/native-code/native-apis/)
- [depot_tools Documentation](https://commondatastorage.googleapis.com/chrome-infra-docs/flat/depot_tools/docs/html/depot_tools.html)
- [GN Build Configuration](https://gn.googlesource.com/gn/+/main/docs/reference.md)
- [WebRTC Issue Tracker](https://issues.webrtc.org/)

## License

This project is provided as-is for educational purposes. WebRTC is licensed under BSD-style license by Google.

---

**Key Takeaways**:
1. Always use depot_tools' bundled LLVM toolchain (Clang + lld)
2. Explicitly build video codec factories: `ninja -C out/Default api/video_codecs:builtin_video_encoder_factory`
3. Run `ranlib` on all `.a` files before linking
4. Use `-fuse-ld=lld` to avoid architecture mismatch errors
5. WebRTC build takes time - be patient!
6. The `rtc_include_builtin_video_codecs` flag has been removed - build targets explicitly
7. Consider migrating to `VideoEncoderFactoryTemplate` for future compatibility

**Happy Coding! 🚀**