package=freetype
$(package)_version=2.10.4
$(package)_download_path=https://de.freedif.org/savannah/freetype/
$(package)_file_name=$(package)-$($(package)_version).tar.gz
$(package)_sha256_hash=5eab795ebb23ac77001cfb68b7d4d50b5d6c7469247b0b01b2c953269f658dac

define $(package)_set_vars
  $(package)_config_opts=--without-zlib --without-png --disable-static --enable-freetype-config
  $(package)_config_opts_linux=--with-pic
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
