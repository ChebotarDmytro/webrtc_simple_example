# WebRTC Hello World - C++ Example with Conan

A simple WebRTC application demonstrating peer-to-peer connection and data channel communication using Google's native WebRTC library on Linux, packaged with Conan 2.x.

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Prerequisites](#prerequisites)
- [Installation](#installation)
- [Building WebRTC with Conan](#building-webrtc-with-conan)
- [Building the Example Project](#building-the-example-project)
- [Running the Application](#running-the-application)
- [Project Structure](#project-structure)
- [Build Configuration Details](#build-configuration-details)
- [Troubleshooting](#troubleshooting)
- [Advanced Configuration](#advanced-configuration)
- [License](#license)

## Overview

This project demonstrates the basic concepts of WebRTC by creating two peer connections that communicate through a data channel. It uses **Conan 2.x** to manage the WebRTC dependency, making builds reproducible and easier to manage.

**Key Features**:
- Uses Conan 2.x for dependency management
- Supports Debug and RelWithDebInfo build types
- Uses system Clang-21 compiler with lld linker
- Monolithic WebRTC library (single `libwebrtc.a`)
- Simple three-step build process

## Features

- ✅ Creates two WebRTC peer connections locally
- ✅ Establishes peer-to-peer data channel
- ✅ Exchanges SDP offers and answers
- ✅ Handles ICE candidate gathering and exchange
- ✅ Sends and receives text messages via data channel
- ✅ Conan 2.x package management
- ✅ Uses Clang-21 + lld linker
- ✅ Proper memory management with smart pointers
- ✅ Modern C++23

## Prerequisites

### System Requirements

- **OS**: Linux (Ubuntu 20.04+ recommended)
- **RAM**: 16GB minimum (for building WebRTC)
- **Disk Space**: 100GB+ free space
- **CPU**: Multi-core processor (building takes time)

### Software Requirements

```bash
sudo apt update
sudo apt install -y \
  git python3 python3-pip curl \
  libgtk-3-dev libx11-dev libxss1 libnss3-dev libasound2-dev \
  libglu1-mesa-dev libxrandr-dev libxcomposite-dev libxdamage-dev \
  libxfixes-dev libxext-dev clang-21 lld-21
```

### Install Conan 2.x

```bash
pip install conan
conan --version  # Should show 2.x.x

# Initialize Conan profile
conan profile detect --force
```

## Installation

### Step 1: Create Workspace Structure

```bash
mkdir -p ~/workspace
cd ~/workspace
```

Your final structure will be:
```
~/workspace/
├── webrtc/                    # WebRTC Conan package recipe
│   ├── conanfile.py
│   ├── depot_tools/           # Created during conan source
│   └── webrtc_checkout/       # Created during conan source
│       └── src/
└── webrtc_simple_example/     # This example project
    ├── conanfile.py
    ├── CMakeLists.txt
    ├── main.cpp
    └── ...
```

### Step 2: Clone Repositories

```bash
cd ~/workspace

# Clone WebRTC Conan recipe
mkdir webrtc
cd webrtc
# Copy your WebRTC conanfile.py here

# Clone example project
cd ~/workspace
git clone <your-repository-url> webrtc_simple_example
```

## Building WebRTC with Conan

### Build Types Supported

- **Debug**: Full debug symbols, no optimization (`-O0`), asserts enabled
- **RelWithDebInfo**: Optimized with `-O2`, minimal debug symbols (recommended for development)

### Step 1: Fetch WebRTC Source

This downloads depot_tools and WebRTC source (~10GB):

```bash
cd ~/workspace/webrtc
conan source .
```

**What this does**:
- Clones depot_tools into `depot_tools/`
- Creates `webrtc_checkout/.gclient` config
- Runs `gclient sync` to download WebRTC source
- Checks out branch `branch-heads/7151` (M137)

**Time**: 30-60 minutes depending on network speed

### Step 2: Build WebRTC Package

**For Debug build**:
```bash
cd ~/workspace/webrtc
conan build . -s build_type=Debug
```

**For RelWithDebInfo build**:
```bash
cd ~/workspace/webrtc
conan build . -s build_type=RelWithDebInfo
```

**What this does**:
- Configures GN with appropriate args for build type
- Builds monolithic `libwebrtc.a` using depot_tools Ninja
- Uses Clang-21 from `/usr/bin/clang++-21`
- Runs `ranlib` on all static libraries

**Time**: 2-4 hours on first build

### Step 3: Package WebRTC

Export the built package to Conan cache:

**For Debug**:
```bash
cd ~/workspace/webrtc
conan export-pkg . -s build_type=Debug
```

**For RelWithDebInfo**:
```bash
cd ~/workspace/webrtc
conan export-pkg . -s build_type=RelWithDebInfo
```

**What this does**:
- Packages headers from `api/`, `rtc_base/`, `p2p/`, etc.
- Packages `libwebrtc.a` library
- Copies to Conan cache: `~/.conan2/p/webrtc.../p/`
- Makes package available for other projects

### Verify Package

```bash
conan list webrtc/7151
```

You should see:
```
webrtc/7151
  Debug
  RelWithDebInfo
```

## Building the Example Project

### Step 1: Install Dependencies

This fetches the WebRTC package from Conan cache (or builds it if missing):

**For Debug**:
```bash
cd ~/workspace/webrtc_simple_example
conan install . --output-folder=build --build=missing -s build_type=Debug
```

**For RelWithDebInfo**:
```bash
cd ~/workspace/webrtc_simple_example
conan install . --output-folder=build --build=missing -s build_type=RelWithDebInfo
```

**What this does**:
- Resolves `webrtc/7151` dependency
- Generates CMake toolchain files in `build/`
- Creates `conan_toolchain.cmake` with correct compiler flags

### Step 2: Build Example

**For Debug**:
```bash
cd ~/workspace/webrtc_simple_example
conan build . -s build_type=Debug
```

**For RelWithDebInfo**:
```bash
cd ~/workspace/webrtc_simple_example
conan build . -s build_type=RelWithDebInfo
```

**What this does**:
- Runs CMake configure with Conan toolchain
- Compiles your application with Clang-21
- Links against `libwebrtc.a` and system libraries
- Creates executable in `build/Debug/` or `build/RelWithDebInfo/`

## Running the Application

```bash
cd ~/workspace/webrtc_simple_example

# Debug build
./build/Debug/webrtcexample

# RelWithDebInfo build
./build/RelWithDebInfo/webrtcexample
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
├── webrtc/                              # WebRTC Conan package
│   ├── conanfile.py                     # Package recipe
│   ├── depot_tools/                     # Google build tools
│   │   ├── gclient
│   │   ├── gn
│   │   └── ninja
│   └── webrtc_checkout/
│       ├── .gclient
│       └── src/                         # WebRTC source
│           ├── api/
│           ├── modules/
│           ├── out/
│           │   ├── Debug/
│           │   │   └── obj/
│           │   │       └── libwebrtc.a
│           │   └── RelWithDebInfo/
│           │       └── obj/
│           │           └── libwebrtc.a
│           └── third_party/
│
└── webrtc_simple_example/               # Example project
    ├── conanfile.py                     # Dependencies
    ├── CMakeLists.txt                   # Build config
    ├── main.cpp
    ├── data_channel_observer.cpp
    ├── data_channel_observer.h
    ├── simple_peer_connection_observer.cpp
    ├── simple_peer_connection_observer.h
    ├── sdp_observer.cpp
    ├── sdp_observer.h
    ├── local_signaling.cpp
    ├── local_signaling.h
    └── build/                           # Build output
        ├── Debug/
        │   └── webrtcexample
        └── RelWithDebInfo/
            └── webrtcexample
```

## Build Configuration Details

### GN Args Used

#### Debug Build
```python
target_os = "linux"
target_cpu = "x64"
is_debug = true                # No optimization (-O0)
is_official_build = false
symbol_level = 2               # Full debug symbols
is_component_build = false     # Monolithic library
rtc_include_tests = false
rtc_build_examples = false
rtc_use_h264 = true
use_rtti = false
use_custom_libcxx = false
```

**Characteristics**:
- No optimization (`-O0`)
- Full debug symbols
- Assertions enabled
- Best for debugging

#### RelWithDebInfo Build
```python
target_os = "linux"
target_cpu = "x64"
is_debug = false               # Enables optimization
is_official_build = false      # Uses -O2 (not -O3 + PGO)
symbol_level = 1               # Minimal debug symbols
chrome_pgo_phase = 0           # Disable PGO explicitly
is_component_build = false     # Monolithic library
rtc_include_tests = false
rtc_build_examples = false
rtc_use_h264 = true
use_rtti = false
use_custom_libcxx = false
```

**Characteristics**:
- Optimized with `-O2` (automatically applied by WebRTC)
- Minimal debug symbols (can still debug crashes)
- Assertions disabled
- **4-5x faster than Debug**
- Best for development & profiling

### Why Not is_official_build=true?

Setting `is_official_build=true` enables:
- Profile-Guided Optimization (PGO)
- Requires Chrome-specific `.profdata` files
- Needs multiple build passes
- Can cause `SIGILL` (Illegal Instruction) errors

**For standalone WebRTC builds, always use `is_official_build=false`.**

### Optimization Levels

WebRTC automatically selects optimization based on GN args:

| is_debug | is_official_build | Optimization | Use Case |
|----------|-------------------|--------------|----------|
| true     | false             | `-O0`        | Debug |
| false    | false             | `-O2`        | **RelWithDebInfo** ✅ |
| false    | true              | `-O3` + PGO  | Chrome only ❌ |

**You cannot override optimization levels in GN args** - they're controlled by WebRTC's `//build/config/compiler/BUILD.gn`.

## Troubleshooting

### Conan Issues

#### ❌ Error: `conan: command not found`
**Solution**:
```bash
pip install conan
# Add to PATH if needed
export PATH="$HOME/.local/bin:$PATH"
```

#### ❌ Error: `webrtc/7151: Not found in local cache`
**Solution**: Build and export the WebRTC package:
```bash
cd ~/workspace/webrtc
conan source .
conan build . -s build_type=Debug
conan export-pkg . -s build_type=Debug
```

#### ❌ Error: `Incompatible package`
**Solution**: Make sure build types match:
```bash
# Check what's available
conan list webrtc/7151

# Remove and rebuild if needed
conan remove webrtc/7151 -c
```

### Build Issues

#### ❌ Error: `FileNotFoundError: chrome/build/linux.pgo.txt`
**Cause**: PGO (Profile-Guided Optimization) is enabled but Chrome profile files are missing  
**Solution**: This is fixed in the conanfile.py by setting:
```python
is_official_build = false
chrome_pgo_phase = 0
```
If you still see this, make sure you're using the updated conanfile.py.

#### ❌ Error: `Unexpected token '-'` in GN args
**Cause**: Invalid GN argument like `optimize_max=-O2`  
**Solution**: Remove invalid args. Optimization is automatic:
```python
# ❌ WRONG
args.append('optimize_max=-O2')

# ✅ CORRECT
args.append('is_debug=false')  # This automatically uses -O2
```

#### ❌ Error: `SIGILL (Illegal Instruction)` during runtime
**Cause**: PGO or aggressive optimizations generated CPU-specific instructions  
**Solution**: The updated conanfile.py fixes this by:
- Setting `chrome_pgo_phase=0` to disable PGO
- Using `is_official_build=false` to use `-O2` (not `-O3`)

If you built with old settings, rebuild:
```bash
cd ~/workspace/webrtc
rm -rf webrtc_checkout/src/out/RelWithDebInfo
conan build . -s build_type=RelWithDebInfo
conan export-pkg . -s build_type=RelWithDebInfo --force
```

#### ❌ Error: `gclient: command not found`
**Solution**: Run `conan source .` first to clone depot_tools:
```bash
cd ~/workspace/webrtc
conan source .
```

#### ❌ Error: `clang++-21: not found`
**Solution**: Install LLVM 21:
```bash
sudo apt install clang-21 lld-21
which clang++-21  # Should show /usr/bin/clang++-21
```

#### ❌ Error: `undefined reference to CreateBuiltinVideoEncoderFactory()`
**Cause**: Video codec factory targets not built  
**Solution**: This shouldn't happen with monolithic build. If it does:
```bash
cd ~/workspace/webrtc/webrtc_checkout/src
ninja -C out/Debug api/video_codecs:builtin_video_encoder_factory
ninja -C out/Debug api/video_codecs:builtin_video_decoder_factory
```

#### ❌ Error: `archive has no index`
**Solution**: Already handled in conanfile.py. If still occurs:
```bash
cd ~/workspace/webrtc/webrtc_checkout/src/out/Debug/obj
find . -name "*.a" -exec ranlib {} \;
```

### Runtime Issues

#### ❌ No output or program hangs
**Solution**: Enable verbose logging in `main.cpp`:
```cpp
#include "rtc_base/logging.h"

int main() {
    rtc::LogMessage::LogToDebug(rtc::LS_VERBOSE);
    rtc::LogMessage::LogTimestamps();
    // ...
}
```

### Verification Commands

#### Check GN Configuration
```bash
cd ~/workspace/webrtc/webrtc_checkout/src

# View all GN args used
gn args out/RelWithDebInfo --short

# Should show:
# is_debug = false
# is_official_build = false
# chrome_pgo_phase = 0
```

#### Check Optimization Level
```bash
# Check actual compiler flags
gn desc out/RelWithDebInfo //api:libjingle_peerconnection_api cflags | grep "\-O"

# Should see: -O2
# Should NOT see: -O3
```

#### Verify Library
```bash
# Check library exists
ls -lh ~/workspace/webrtc/webrtc_checkout/src/out/RelWithDebInfo/obj/libwebrtc.a

# Check symbols
nm -C out/RelWithDebInfo/obj/libwebrtc.a | grep CreateBuiltinVideo
```

## Advanced Configuration

### Changing WebRTC Branch

Edit `webrtc/conanfile.py`:
```python
class WebRTCConan(ConanFile):
    name = "webrtc"
    version = "7151"  # Change to desired branch number
```

Available branches:
- `7151` - M137 (2024, current)
- `6099` - M120 (2023)
- `5790` - M108 (2022)

Then rebuild:
```bash
cd ~/workspace/webrtc
rm -rf webrtc_checkout/
conan source .
conan build . -s build_type=Debug
conan export-pkg . -s build_type=Debug
```

### Custom GN Args

Edit the `_get_gn_args()` method in `webrtc/conanfile.py`:

```python
def _get_gn_args(self):
    args = []
    # ... existing args ...
    
    # Add custom args
    args.append('rtc_use_h265=true')  # Enable H.265 (if available)
    
    return args
```

### Build Only Specific Configurations

```bash
# Build only Debug
cd ~/workspace/webrtc
conan source .
conan build . -s build_type=Debug
conan export-pkg . -s build_type=Debug

# Or only RelWithDebInfo
conan build . -s build_type=RelWithDebInfo
conan export-pkg . -s build_type=RelWithDebInfo
```

### Clean Rebuild

```bash
# Remove from Conan cache
conan remove webrtc/7151 -c

# Clean workspace
cd ~/workspace/webrtc
rm -rf depot_tools/ webrtc_checkout/

# Rebuild from scratch
conan source .
conan build . -s build_type=Debug
conan export-pkg . -s build_type=Debug
```

## Conan Commands Reference

### WebRTC Package Commands

```bash
# Fetch source
conan source .

# Build Debug
conan build . -s build_type=Debug

# Build RelWithDebInfo
conan build . -s build_type=RelWithDebInfo

# Export to cache
conan export-pkg . -s build_type=Debug
conan export-pkg . -s build_type=RelWithDebInfo

# List packages
conan list webrtc/7151

# Remove package
conan remove webrtc/7151 -c
```

### Example Project Commands

```bash
# Install dependencies + generate build files
conan install . --output-folder=build --build=missing -s build_type=Debug

# Build project
conan build . -s build_type=Debug

# Clean build
rm -rf build/
```

## Build Type Comparison

| Build Type      | Optimization | Debug Symbols | Asserts | Speed vs Debug | Use Case                |
|-----------------|--------------|---------------|---------|----------------|-------------------------|
| Debug           | None (-O0)   | Full          | Enabled | 1x (baseline)  | Development, debugging  |
| RelWithDebInfo  | High (-O2)   | Minimal       | Disabled| 4-5x faster    | Profiling, performance  |

**Recommendation**: Use `RelWithDebInfo` for development to get good performance while still being able to debug crashes.

## Performance Notes

### Expected Build Times

| Operation | Time (First Build) | Time (Incremental) |
|-----------|-------------------|-------------------|
| `conan source` | 30-60 min | ~1 min (if already done) |
| `conan build` (Debug) | 2-4 hours | 5-15 min |
| `conan build` (RelWithDebInfo) | 2-4 hours | 5-15 min |
| Example project | 1-2 min | 10-30 sec |

### Disk Space Usage

- WebRTC source: ~10 GB
- Debug build: ~15 GB
- RelWithDebInfo build: ~10 GB
- **Total**: ~35-40 GB per workspace

**Tip**: Use `export-pkg` workflow instead of `conan create` to avoid copying the entire source tree.

## Next Steps

1. **Add Audio/Video Streams**
   - Use `AudioDeviceModule` for audio capture
   - Add video tracks with `VideoTrackSource`

2. **Network Signaling**
   - Implement WebSocket server for SDP/ICE exchange
   - Enable connections between different machines

3. **TURN Server Integration**
   - Add TURN servers for NAT traversal
   - Configure in `RTCConfiguration`

4. **Create More Conan Packages**
   - Package your application
   - Share with team via Conan remotes

## Resources

- [Conan 2.x Documentation](https://docs.conan.io/2/)
- [WebRTC Native Code](https://webrtc.googlesource.com/src/)
- [WebRTC Native API](https://webrtc.github.io/webrtc-org/native-code/native-apis/)
- [depot_tools](https://commondatastorage.googleapis.com/chrome-infra-docs/flat/depot_tools/docs/html/depot_tools.html)
- [GN Build Configuration](https://gn.googlesource.com/gn/+/main/docs/reference.md)

## License

This project is provided as-is for educational purposes. WebRTC is licensed under BSD-style license by Google.

---

## Quick Start Summary

**1. Build WebRTC package once**:
```bash
cd ~/workspace/webrtc
conan source .
conan build . -s build_type=Debug
conan export-pkg . -s build_type=Debug

# Optional: Also build RelWithDebInfo
conan build . -s build_type=RelWithDebInfo
conan export-pkg . -s build_type=RelWithDebInfo
```

**2. Build your project**:
```bash
cd ~/workspace/webrtc_simple_example
conan install . --output-folder=build --build=missing -s build_type=Debug
conan build . -s build_type=Debug
./build/Debug/webrtcexample
```

**3. For RelWithDebInfo**, replace `Debug` with `RelWithDebInfo` in all commands.

---

## Key Takeaways

1. ✅ Always use `is_official_build=false` for standalone WebRTC builds
2. ✅ Set `chrome_pgo_phase=0` to disable PGO
3. ✅ Don't try to override optimization levels - WebRTC controls them automatically
4. ✅ RelWithDebInfo gives 4-5x better performance than Debug
5. ✅ Use `conan export-pkg` workflow to save disk space
6. ✅ Only Debug and RelWithDebInfo build types are supported

**Happy Coding! 🚀**
