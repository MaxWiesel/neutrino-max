BUILT_SOURCES = rcsim.h
# ugly: this is in the source tree, but belongs into the build dir :-(
rcsim.h: $(srcdir)/driver/rcinput.h $(srcdir)/driver/rcinput_fake.h $(srcdir)/create_rcsim_h.sh
	pushd $(srcdir) && sh ./create_rcsim_h.sh > $@

AM_CXXFLAGS = -fno-rtti -fno-exceptions -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS

SUBDIRS = zapit gui daemonc driver system eitd timerd nhttpd

AM_CPPFLAGS = \
	-I$(srcdir) \
	-I$(top_builddir) \
	-I$(top_srcdir) \
	-I$(top_srcdir)/lib \
	-I$(top_srcdir)/src/zapit/include \
	-I$(top_srcdir)/lib/connection \
	-I$(top_srcdir)/lib/libeventserver \
	-I$(top_srcdir)/lib/libconfigfile \
	-I$(top_srcdir)/lib/libnet \
	-I$(top_srcdir)/lib/xmltree \
	-I$(top_srcdir)/lib/libmd5sum \
	-I$(top_srcdir)/lib/libupnpclient \
	-I$(top_srcdir)/lib/jsoncpp \
	@SIGC_CFLAGS@ \
	@FREETYPE_CFLAGS@ \
	@AVFORMAT_CFLAGS@ \
	@LUA_CFLAGS@ \
	@HWLIB_CFLAGS@

if USE_PUGIXML
PUGIXML_LIBS = -lpugixml
endif

if USE_TREMOR
VORBISLIBS = @VORBISIDEC_LIBS@
else
VORBISLIBS = -lvorbisfile -lvorbis -logg
endif

bin_PROGRAMS = neutrino

neutrino_SOURCES = neutrino_menue.cpp neutrino.cpp

if ENABLE_FLAC
FLACLIBS = -lFLAC
else
FLACLIBS =
endif

if ENABLE_FFMPEGDEC
ADEC_LIBS = @SWRESAMPLE_LIBS@
else
ADEC_LIBS = @MAD_LIBS@ \
	@ID3TAG_LIBS@ \
	@SWRESAMPLE_LIBS@ \
	$(FLACLIBS) \
	$(VORBISLIBS)
endif

AM_CPPFLAGS += -D_FILE_OFFSET_BITS=64

if BOXMODEL_CST_HD2
MTDUTILSLIBS = \
	system/mtdutils/libneutrino_system_mtdutils.a \
	system/mtdutils/lib/libneutrino_system_mtdutils_lib.a
endif

if ENABLE_LUA
LUALIBS = \
	gui/lua/libneutrino_gui_lua.a
endif

neutrino_LDADD = \
	daemonc/libneutrino_daemonc.a \
	gui/bedit/libneutrino_gui_bedit.a \
	gui/libtimerlist.a \
	gui/libneutrino_gui.a \
	gui/widget/libneutrino_gui_widget.a \
	gui/widget/libneutrino_gui_widget2.a \
	driver/pictureviewer/libneutrino_pictureviewer.a \
	$(MTDUTILSLIBS) \
	driver/movieinfo.o \
	gui/moviebrowser/mb.o \
	gui/libneutrino_gui2.a \
	gui/moviedb/libneutrino_gui_moviedb.a \
	gui/components/libneutrino_gui_components.a \
	$(LUALIBS) \
	eitd/libsectionsd.a \
	gui/volumebar.o \
	driver/libneutrino_driver.a \
	driver/audiodec/libneutrino_driver_audiodec.a \
	driver/libneutrino_driver_netfile.a \
	timerd/libtimerd.a \
	zapit/src/libzapit.a \
	nhttpd/libnhttpd.a \
	nhttpd/tuxboxapi/libnhttpd_tuxboxapi.a \
	nhttpd/yhttpd_mods/libyhttpdmods.a \
	nhttpd/yhttpd_core/libyhttpd.a \
	system/libneutrino_system.a \
	$(top_builddir)/lib/sectionsdclient/libsectionsdclient.a \
	$(top_builddir)/lib/timerdclient/libtimerdclient.a \
	$(top_builddir)/src/zapit/lib/libzapitclient.a \
	$(top_builddir)/lib/libconfigfile/libtuxbox-configfile.a \
	$(top_builddir)/lib/connection/libtuxbox-connection.a \
	$(top_builddir)/lib/libeventserver/libtuxbox-eventserver.a \
	$(top_builddir)/lib/xmltree/libtuxbox-xmltree.a \
	$(top_builddir)/lib/jsoncpp/libjsoncpp.a \
	$(top_builddir)/lib/libnet/libtuxbox-net.a \
	$(top_builddir)/lib/libmd5sum/libtuxbox-md5sum.a \
	$(top_builddir)/lib/libtuxtxt/libtuxtxt.a \
	$(top_builddir)/lib/libdvbsub/libdvbsub.a \
	$(top_builddir)/lib/libiw/libiw.a \
	$(top_builddir)/src/gui/widget/yaft/libneutrino_gui_terminal.a \
	@CURL_LIBS@ \
	@FREETYPE_LIBS@ \
	@PNG_LIBS@ \
	@BLURAY_LIBS@ \
	@AVFORMAT_LIBS@ \
	@AVUTIL_LIBS@ \
	@AVCODEC_LIBS@ \
	$(ADEC_LIBS) \
	@SIGC_LIBS@ \
	$(PUGIXML_LIBS) \
	-ldvbsi++ \
	-ljpeg \
	-lutil \
	-lOpenThreads \
	-lrt -lpthread \
	-ldl \
	-lblkid \
	-lz

# LCD display
if ENABLE_LCD
neutrino_LDADD += $(top_builddir)/lib/liblcddisplay/liblcddisplay.a
endif

if ENABLE_GRAPHLCD
neutrino_LDADD += \
	driver/glcd/libneutrino_driver_glcd.a \
	-lglcddrivers -lglcdgraphics -lusb-1.0
endif

if ENABLE_GIFLIB
neutrino_LDADD += -lgif
else
neutrino_LDADD += -lungif
endif

if ENABLE_FRIBIDI
neutrino_LDADD += -lfribidi
endif

if ENABLE_LUA
neutrino_LDADD += @LUA_LIBS@
endif

if ENABLE_UPNP
neutrino_LDADD += \
	$(top_builddir)/lib/libupnpclient/libtuxbox-upnpclient.a
endif

if BOXTYPE_CST
neutrino_LDADD += \
	$(top_builddir)/lib/hardware/coolstream/libhwcaps.a
if ENABLE_LIBCOOLSTREAM_STATIC
neutrino_LDADD += -lca-sc
neutrino_LDADD += @LIBCOOLSTREAM_STATIC_DIR@/libcoolstream.a
else
neutrino_LDADD += -lcoolstream -lca-sc
endif

if BOXMODEL_CST_HD2
neutrino_LDADD += -lca-ci -llnxtmvssUsr -llnxUKAL -llnxplatUsr -llnxtmasUsr -llnxdvbciUsr -llnxpvrUsr -llnxcssUsr -llnxnotifyqUsr -ltmpvrDataManager-cst -ltmpvrIndexStorage -llnxscsUsr
else
neutrino_LDADD += -lnxp
endif

if BOXMODEL_CST_HD2
neutrino_LDADD += -liconv
endif

#NI bin_PROGRAMS += drivertool
#NI drivertool_SOURCES = drivertool.c
bin_PROGRAMS += dt
dt_SOURCES = dt.c
endif
bin_PROGRAMS += rcsim
rcsim_SOURCES = rcsim.c rcsim.h
bin_PROGRAMS += mdev_helper
mdev_helper_SOURCES = mdev_helper.cpp

if HAVE_LIBSTB_HAL
neutrino_LDADD += \
	-lstb-hal
neutrino_LDFLAGS = \
	$(STB_HAL_LIB)
endif

if BOXTYPE_MIPSBOX
neutrino_LDADD += \
	@SWSCALE_LIBS@ \
	-lass
endif

if BOXTYPE_ARMBOX
neutrino_LDADD += \
	@SWSCALE_LIBS@ \
	-lass
endif

if BOXMODEL_RASPI
neutrino_LDADD += \
	-L/opt/vc/lib/ -lbcm_host -lvcos -lvchiq_arm -lpthread -lrt
endif

if BOXTYPE_CST
bin_PROGRAMS += shutdown_helper
shutdown_helper_SOURCES = shutdown_helper.c
endif
