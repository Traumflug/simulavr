CONFIG_SUBDIR_NAME=`echo $(ac_aux_dir) | sed -e "s|$(srcdir)/\?|./|"`
#CONFIG_SUBDIR_NAME=${ac_aux_dir#$(srcdir)}

EXTRA_DIST	= $(ac_aux_dir)/config.guess \
		$(ac_aux_dir)/config.sub \
		$(ac_aux_dir)/install-sh \
		$(ac_aux_dir)/ltmain.sh \
		$(ac_aux_dir)/missing \
		$(ac_aux_dir)/mkinstalldirs\
		$(ac_aux_dir)/avr_local.m4 \
		$(am_depcomp)
		bootstrap

# WAR: Should condition examples directories based on availability of TCL/Itcl
SUBDIRS		= src @ac_regression_subdir@ examples
DIST_SUBDIRS    = src regress examples

MAINTAINERCLEANFILES = Makefile.in

