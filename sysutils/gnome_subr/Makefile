# Created by: Jean-Yves Lefort <jylefort@FreeBSD.org>
# $FreeBSD$

PORTNAME=	gnome_subr
PORTVERSION=	1.0
PORTREVISION=	1
CATEGORIES=	sysutils
DISTFILES=

MAINTAINER=	gnome@FreeBSD.org
COMMENT=	Common startup and shutdown subroutines used by GNOME scripts

NO_BUILD=	yes
PLIST_FILES=	etc/gnome.subr

do-install:
	${INSTALL_DATA} ${FILESDIR}/gnome.subr ${STAGEDIR}${PREFIX}/etc

.include <bsd.port.mk>
