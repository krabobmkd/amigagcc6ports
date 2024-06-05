# amigagcc6ports

## What is that ?

This is some ports of GPL / LGPL libs for the Amiga Gcc6.5 cross compiler from bebbo, using cmake and last version of libs as of January 2024.

 Ths was made primarily to port "heif-convert" tool from libheif, to transform .heic images from Smartphones, into jpg or png on Amiga classic / 68k .. yet a 68040/68060 CPU with FPU is needed... Pistorm, Vampire and UAE in mind. 
 
 All CMakes are tweaked to generate only static libs... because GCC6.5/bebbo does not manage Amiga Shared libs at the moment. (Yet only libs with C Apis could be ported to Amiga shared libraries, for the moment.)

CMakelists are all tuned so it is compiled with "68060 CPU and --fast-math", and modern "C11" needed by libheif and libde265. Also libheif is strongly based on rtti and dynamic_cast. C11 is way more modern than usual Amiga C code. Most sources compile easily, yet tons of modifications where done everywhere, for various reasons and sometimes obvious bug correction. search comment with tag "Vf-Amiga" to trace most modifications from original source.
 
 zlib pnglib and libjpeg are only used by heif-convert to export PNG. 
  
## How to build ?

 On a machine  where gcc6.5 bebbo, and cmake, are installed (linux, mac, Windows),
 building would look like:

``` 
git clone https://github.com/krabobmkd/amigagcc6ports
cd amigagcc6ports
mkdir amiga-build
cd amiga-build
cmake ../libheif/ -DCMAKE_TOOLCHAIN_FILE=../cmake/Modules/Platform/m68k-amigaos.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build .
```
 
 ... this would compile zlib,pnglib,libde265,libheif and Amiga executables examples/heif-convert and heif-info.

## Current state 

 2024/01/18:
 heif-convert now makes all images fine, previous glitch bugs were due to compiler error with -O2 in libde265. Works fine and not slower with -O1. Now each libs finetunes their own optimisation option.
 Does work fine with a few Android and iOS photos tested: a 4624x2604 .heic photo would takes 1m05 to convert to jpeg on piStorm32-lite with Raspberry Pi 3. (and 4 seconds on a modern PC with SSE and multithreading). On a 68060 50Mhz, it would takes 6 minutes.
  Note: the speed critical code in libde265 is already filled with options for intel SSE . A further way to let some Amiga hardware parallelize decoding (rpi VC4? zz9k arm ? icedrake 68080 special ops?) may be implemented around the same parts.
 
## What's left to do ? 


 This version correspond to the aminet.net heif-convert.lha release.

 Yet to do next:
 - have less intrusive modifications according to original sources.
 - cmake/Modules scripts are a scam: next checkpoint would be making each lib's install commands do their job, per m68k family, to some include and lib place, THEN make find_package() scripts worthy. 
 - Amiga MMU tests are also to be done (muGuardian angel and sorts...). Yet if C++ runtime is correct, there should be no leak.
 
 krb.
 
