package=fontconfig
$(package)_version=2.8.0
$(package)_download_path=http://www.freedesktop.org/software/fontconfig/release/
#$(package)_file_name=$(package)-$($(package)_version).tar.bz2
$(package)_file_name=$(package)-$($(package)_version).tar.gz
#$(package)_sha256_hash=dc62447533bca844463a3c3fd4083b57c90f18a70506e7a9f4936b5a1e516a99
#$(package)_sha256_hash=cf0c30807d08f6a28ab46c61b8dbd55c97d2f292cf88f3a07d3384687f31f017
$(package)_sha256_hash=fa2a1c6eea654d9fce7a4b1220f10c99cdec848dccaf1625c01f076b31382335
$(package)_dependencies=freetype expat

define $(package)_set_vars
  $(package)_config_opts=--disable-docs --disable-static
endef

define $(package)_config_cmds
  $($(package)_autoconf)
endef

define $(package)_build_cmds
  $(MAKE)
endef

define $(package)_stage_cmds
  $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef
