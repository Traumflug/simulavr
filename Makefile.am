CONFIG_SUBDIR_NAME=`echo $(ac_aux_dir) | sed -e "s|$(srcdir)/\?|./|"`
#CONFIG_SUBDIR_NAME=${ac_aux_dir#$(srcdir)}

AUX_DIST	= $(ac_aux_dir)/config.guess \
		$(ac_aux_dir)/config.sub \
		$(ac_aux_dir)/install-sh \
		$(ac_aux_dir)/ltmain.sh \
		$(ac_aux_dir)/missing \
		$(ac_aux_dir)/mkinstalldirs\
		$(ac_aux_dir)/avr_local.m4


EXTRA_DIST	= bootstrap

# WAR: Should condition examples directories based on availability of TCL/Itcl
SUBDIRS		= src @ac_regression_subdir@ examples
DIST_SUBDIRS    = src regress examples

MAINTAINERCLEANFILES = Makefile.in

# There has to be a better way....but I don't know it...
# This bit might seem silly, but make distcheck failed when in a separate tree...when I fixed that, it wouldn't make distcheck from the source directory....Hoepfully it is solved now! 
dist-hook:
	Xfred=${ac_aux_dir} ; mkdir -p $(distdir)/$${Xfred#$(srcdir)*}
	Xfred=${ac_aux_dir} ; for file in $(AUX_DIST) $(AUX_DIST_EXTRA); do \
		echo cp $$file $(distdir)/$${Xfred#$(srcdir)*}; \
	  cp $$file $(distdir)/$${Xfred#$(srcdir)*}; \
	done
	echo "@@@@"
	echo source = $(AUX_DIST) $(AUX_DIST_EXTRA)
	echo dest   = ./${ac_aux_dir#*$(srcdir)}
