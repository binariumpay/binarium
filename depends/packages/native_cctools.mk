package=native_cctools
$(package)_version=ee31ae567931c426136c94aad457c7b51d844beb
#$(package)_version=f53c6186393915ba9bff287f18769f9e14ec02e6
#$(package)_version=
$(package)_download_path=https://github.com/theuni/cctools-port/archive
#$(package)_download_path=https://github.com/tpoechtrager/cctools-port/archive/
#$(package)_download_path=https://github.com/theuni/cctools-port/archive
$(package)_file_name=$($(package)_version).tar.gz
#$(package)_file_name=cctools-877.8-ld64-253.9-1.tar.gz
#$(package)_file_name=862-ld64-241.9-2.tar.gz
$(package)_sha256_hash=ef107e6ab1b3994cb22e14f4f5c59ea0c0b5a988e6b21d42ed9616b018bbcbf9
#$(package)_sha256_hash=ab513d71c4f8458362ae34d501202c6362dc8316de8e6237865f8999f1def8a7
#$(package)_sha256_hash=c88b0631b1d7bb5186dd6466a62f5220dc6191f2b2d9c7c122b327385e734aaf
$(package)_build_subdir=cctools
#$(package)_clang_version=3.3
#$(package)_clang_version=6.0.0
#$(package)_clang_version=3.9.1
$(package)_clang_version=4.0.0
$(package)_clang_download_path=http://llvm.org/releases/$($(package)_clang_version)
#$(package)_clang_download_file=clang+llvm-$($(package)_clang_version)-amd64-Ubuntu-12.04.2.tar.gz
#$(package)_clang_download_file=clang+llvm-$($(package)_clang_version)-x86_64-linux-gnu-ubuntu-16.04.tar.xz
#$(package)_clang_download_file=clang+llvm-$($(package)_clang_version)-x86_64-linux-gnu-ubuntu-16.04.tar.xz
$(package)_clang_download_file=clang+llvm-$($(package)_clang_version)-x86_64-linux-gnu-ubuntu-16.10.tar.xz
#$(package)_clang_file_name=clang-llvm-$($(package)_clang_version)-amd64-Ubuntu-12.04.2.tar.gz
#$(package)_clang_file_name=clang-llvm-$($(package)_clang_version)-x86_64-linux-gnu-ubuntu-16.04.tar.xz
#$(package)_clang_file_name=clang-llvm-$($(package)_clang_version)-x86_64-linux-gnu-ubuntu-16.04.tar.xz
$(package)_clang_file_name=clang-llvm-$($(package)_clang_version)-x86_64-linux-gnu-ubuntu-16.10.tar.xz
#$(package)_clang_sha256_hash=60d8f69f032d62ef61bf527857ebb933741ec3352d4d328c5516aa520662dab7
#$(package)_clang_sha256_hash=cc99fda45b4c740f35d0a367985a2bf55491065a501e2dd5d1ad3f97dcac89da
#$(package)_clang_sha256_hash=99d1ffd4be8fd3331b4d2478ada7ee6ed352729bfe4a1070450cdb9a3ce8ef9b
$(package)_clang_sha256_hash=3b8761eaaee7d408aa2128a449d5c9b45ba40c9ca2bde308a24dd4ddc7611348
$(package)_extra_sources=$($(package)_clang_file_name)

#$(call fetch_file,$(package),$($(package)_download_path),$($(package)_download_file),$($(package)_file_name),$($(package)_sha256_hash)) &&
define $(package)_fetch_cmds
$(call fetch_file,$(package),$($(package)_download_path),$($(package)_download_file),$($(package)_file_name),$($(package)_sha256_hash)) && \
$(call fetch_file,$(package),$($(package)_clang_download_path),$($(package)_clang_download_file),$($(package)_clang_file_name),$($(package)_clang_sha256_hash))
endef

define $(package)_extract_cmds
  mkdir -p toolchain/bin toolchain/lib/clang/3.5/include && \
  tar --strip-components=1 -C toolchain -xf $($(package)_source_dir)/$($(package)_clang_file_name) && \
  echo "#!/bin/sh" > toolchain/bin/$(host)-dsymutil && \
  echo "exit 0" >> toolchain/bin/$(host)-dsymutil && \
  chmod +x toolchain/bin/$(host)-dsymutil && \
  tar --strip-components=1 -xf $($(package)_source)
endef

define $(package)_set_vars
$(package)_config_opts=--target=$(host) --disable-libuuid
$(package)_ldflags+=-Wl,-rpath=\\$$$$$$$$\$$$$$$$$ORIGIN/../lib
$(package)_cc=$($(package)_extract_dir)/toolchain/bin/clang
$(package)_cxx=$($(package)_extract_dir)/toolchain/bin/clang++
endef

define $(package)_preprocess_cmds
  cd $($(package)_build_subdir); ./autogen.sh
endef

define $(package)_config_cmds
  $($(package)_autoconf)
endef

define $(package)_build_cmds
  $(MAKE)
endef

define $(package)_stage_cmds
  $(MAKE) DESTDIR=$($(package)_staging_dir) install && \
  cd $($(package)_extract_dir)/toolchain && \
  mkdir -p $($(package)_staging_prefix_dir)/lib/clang/$($(package)_clang_version)/include && \
  mkdir -p $($(package)_staging_prefix_dir)/bin $($(package)_staging_prefix_dir)/include && \
  cp -P bin/clang bin/clang++ $($(package)_staging_prefix_dir)/bin/ &&\
  cp lib/libLTO.so $($(package)_staging_prefix_dir)/lib/ && \
  cp -rf lib/clang/$($(package)_clang_version)/include/* $($(package)_staging_prefix_dir)/lib/clang/$($(package)_clang_version)/include/ && \
  cp bin/$(host)-dsymutil $($(package)_staging_prefix_dir)/bin && \
  if `test -d include/c++/`; then cp -rf include/c++/ $($(package)_staging_prefix_dir)/include/; fi && \
  if `test -d lib/c++/`; then cp -rf lib/c++/ $($(package)_staging_prefix_dir)/lib/; fi
endef
