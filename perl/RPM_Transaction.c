/*
 * This file was generated automatically by ExtUtils::ParseXS version 2.2203 from the
 * contents of RPM_Transaction.xs. Do not edit this file, edit RPM_Transaction.xs instead.
 *
 *	ANY CHANGES MADE HERE WILL BE LOST! 
 *
 */

#line 1 "RPM_Transaction.xs"
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#undef Fflush
#undef Mkdir
#undef Stat
#undef Fstat

#include <stdio.h>
#include <string.h>
#include <utime.h>

#include "rpmio.h"
#include "rpmiotypes.h"

#include "rpmtypes.h"
#include "rpmtag.h"
#include "rpmdb.h"

#include "rpmts.h"
#include "rpmte.h"
#include "misc.h"

#include "rpmcli.h"


/* Chip, this is somewhat stripped down from the default callback used by
   the rpmcli.  It has to be here to insure that we open the pkg again. 
   If we don't do this we get segfaults.  I also, kept the updating of some
   of the rpmcli static vars, but I may not have needed to do this.

   Also, we probably want to give a nice interface such that we could allow
   users of RPM to do their own callback, but that will have to come later.
*/
#ifdef NOTYET
static void * _null_callback(
	const void * arg, 
	const rpmCallbackType what, 
	const uint64_t amount, 
	const uint64_t total, 
	fnpyKey key, 
	rpmCallbackData data)
{
	Header h = (Header) arg;
	int flags = (int) ((long)data);
	void * rc = NULL;
	const char * filename = (const char *)key;
	static FD_t fd = NULL;
	int xx;

	/* Code stolen from rpminstall.c and modified */
	switch(what) {
		case RPMCALLBACK_INST_OPEN_FILE:
	 		if (filename == NULL || filename[0] == '\0')
			     return NULL;
			fd = Fopen(filename, "r.ufdio");
			/* FIX: still necessary? */
			if (fd == NULL || Ferror(fd)) {
				fprintf(stderr, "open of %s failed!\n", filename);
				if (fd != NULL) {
					xx = Fclose(fd);
					fd = NULL;
				}
			} else
				fd = fdLink(fd, "persist (showProgress)");
			return (void *)fd;
	 		break;

	case RPMCALLBACK_INST_CLOSE_FILE:
		/* FIX: still necessary? */
		fd = fdFree(fd, "persist (showProgress)");
		if (fd != NULL) {
			xx = Fclose(fd);
			fd = NULL;
		}
		break;

	case RPMCALLBACK_INST_START:
		rpmcliHashesCurrent = 0;
		if (h == NULL || !(flags & INSTALL_LABEL))
			break;
		break;

	case RPMCALLBACK_TRANS_PROGRESS:
	case RPMCALLBACK_INST_PROGRESS:
		break;

	case RPMCALLBACK_TRANS_START:
		rpmcliHashesCurrent = 0;
		rpmcliProgressTotal = 1;
		rpmcliProgressCurrent = 0;
		break;

	case RPMCALLBACK_TRANS_STOP:
		rpmcliProgressTotal = rpmcliPackagesTotal;
		rpmcliProgressCurrent = 0;
		break;

	case RPMCALLBACK_REPACKAGE_START:
		rpmcliHashesCurrent = 0;
		rpmcliProgressTotal = total;
		rpmcliProgressCurrent = 0;
		break;

	case RPMCALLBACK_REPACKAGE_PROGRESS:
		break;

	case RPMCALLBACK_REPACKAGE_STOP:
		rpmcliProgressTotal = total;
		rpmcliProgressCurrent = total;
		rpmcliProgressTotal = rpmcliPackagesTotal;
		rpmcliProgressCurrent = 0;
		break;

	case RPMCALLBACK_UNINST_PROGRESS:
		break;
	case RPMCALLBACK_UNINST_START:
		break;
	case RPMCALLBACK_UNINST_STOP:
		break;
	case RPMCALLBACK_UNPACK_ERROR:
		break;
	case RPMCALLBACK_CPIO_ERROR:
		break;
	case RPMCALLBACK_UNKNOWN:
		break;
	default:
		break;
	}
	
	return rc;	
}
#endif

static void *
    transCallback(const void *h,
       const rpmCallbackType what,
       const unsigned long long amount,
       const unsigned long long total,
       const void * pkgKey,
       void * data) {
    
    /* The call back is used to open/close file, so we fix value, run the perl callback
 *      * and let rpmShowProgress from rpm rpmlib doing its job.
 *           * This unsure we'll not have to follow rpm code at each change. */
    const char * filename = (const char *)pkgKey;
    const char * s_what = NULL;
    dSP;

    PUSHMARK(SP);
    
    switch (what) {
        case RPMCALLBACK_UNKNOWN:
            s_what = "UNKNOWN";
        break;
        case RPMCALLBACK_INST_OPEN_FILE:
            if (filename != NULL && filename[0] != '\0') {
                XPUSHs(sv_2mortal(newSVpv("filename", 0)));
                XPUSHs(sv_2mortal(newSVpv(filename, 0)));
            }
            s_what = "INST_OPEN_FILE";
        break;
        case RPMCALLBACK_INST_CLOSE_FILE:
            s_what = "INST_CLOSE_FILE";
        break;
        case RPMCALLBACK_INST_PROGRESS:
            s_what = "INST_PROGRESS";
        break;
        case RPMCALLBACK_INST_START:
            s_what = "INST_START";
            if (h) {
                XPUSHs(sv_2mortal(newSVpv("header", 0)));
                XPUSHs(sv_2mortal(sv_setref_pv(newSVpv("", 0), "RPM::Header", &h)));
#ifdef HDRPMMEM
                PRINTF_NEW(bless_header, &h, -1);
#endif
            }
        break;
        case RPMCALLBACK_TRANS_PROGRESS:
            s_what = "TRANS_PROGRESS";
        break;
        case RPMCALLBACK_TRANS_START:
            s_what = "TRANS_START";
        break;
        case RPMCALLBACK_TRANS_STOP:
            s_what = "TRANS_STOP";
        break;
        case RPMCALLBACK_UNINST_PROGRESS:
            s_what = "UNINST_PROGRESS";
        break;
        case RPMCALLBACK_UNINST_START:
            s_what = "UNINST_START";
        break;
        case RPMCALLBACK_UNINST_STOP:
            s_what = "UNINST_STOP";
        break;
        case RPMCALLBACK_REPACKAGE_PROGRESS:
            s_what = "REPACKAGE_PROGRESS";
        break;
        case RPMCALLBACK_REPACKAGE_START:
            s_what = "REPACKAGE_START";
        break;
        case RPMCALLBACK_REPACKAGE_STOP:
            s_what = "REPACKAGE_STOP";
        break;
        case RPMCALLBACK_UNPACK_ERROR:
            s_what = "UNPACKAGE_ERROR";
        break;
        case RPMCALLBACK_CPIO_ERROR:
            s_what = "CPIO_ERROR";
        break;
        case RPMCALLBACK_SCRIPT_ERROR:
            s_what = "SCRIPT_ERROR";
        break;
    }
   
    XPUSHs(sv_2mortal(newSVpv("what", 0)));
    XPUSHs(sv_2mortal(newSVpv(s_what, 0)));
    XPUSHs(sv_2mortal(newSVpv("amount", 0)));
    XPUSHs(sv_2mortal(newSViv(amount)));
    XPUSHs(sv_2mortal(newSVpv("total", 0)));
    XPUSHs(sv_2mortal(newSViv(total)));
    PUTBACK;
    call_sv((SV *) data, G_DISCARD | G_SCALAR);
    SPAGAIN;
  
    /* Running rpmlib callback, returning its value */
    return rpmShowProgress(h,
            what, 
            amount, 
            total, 
            pkgKey, 
            (long *) INSTALL_NONE /* shut up */);
}



#line 249 "RPM_Transaction.c"
#ifndef PERL_UNUSED_VAR
#  define PERL_UNUSED_VAR(var) if (0) var = var
#endif

#ifndef PERL_ARGS_ASSERT_CROAK_XS_USAGE
#define PERL_ARGS_ASSERT_CROAK_XS_USAGE assert(cv); assert(params)

/* prototype to pass -Wmissing-prototypes */
STATIC void
S_croak_xs_usage(pTHX_ const CV *const cv, const char *const params);

STATIC void
S_croak_xs_usage(pTHX_ const CV *const cv, const char *const params)
{
    const GV *const gv = CvGV(cv);

    PERL_ARGS_ASSERT_CROAK_XS_USAGE;

    if (gv) {
        const char *const gvname = GvNAME(gv);
        const HV *const stash = GvSTASH(gv);
        const char *const hvname = stash ? HvNAME(stash) : NULL;

        if (hvname)
            Perl_croak(aTHX_ "Usage: %s::%s(%s)", hvname, gvname, params);
        else
            Perl_croak(aTHX_ "Usage: %s(%s)", gvname, params);
    } else {
        /* Pants. I don't think that it should be possible to get here. */
        Perl_croak(aTHX_ "Usage: CODE(0x%"UVxf")(%s)", PTR2UV(cv), params);
    }
}
#undef  PERL_ARGS_ASSERT_CROAK_XS_USAGE

#ifdef PERL_IMPLICIT_CONTEXT
#define croak_xs_usage(a,b)	S_croak_xs_usage(aTHX_ a,b)
#else
#define croak_xs_usage		S_croak_xs_usage
#endif

#endif

/* NOTE: the prototype of newXSproto() is different in versions of perls,
 * so we define a portable version of newXSproto()
 */
#ifdef newXS_flags
#define newXSproto_portable(name, c_impl, file, proto) newXS_flags(name, c_impl, file, proto, 0)
#else
#define newXSproto_portable(name, c_impl, file, proto) (PL_Sv=(SV*)newXS(name, c_impl, file), sv_setpv(PL_Sv, proto), (CV*)PL_Sv)
#endif /* !defined(newXS_flags) */

#line 301 "RPM_Transaction.c"

XS(XS_RPM__Transaction_new); /* prototype to pass -Wmissing-prototypes */
XS(XS_RPM__Transaction_new)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items < 1)
       croak_xs_usage(cv,  "class, ...");
    {
	char *	class = (char *)SvPV_nolen(ST(0));
	rpmts	RETVAL;
#line 247 "RPM_Transaction.xs"
    RETVAL = rpmtsCreate();
#line 318 "RPM_Transaction.c"
	ST(0) = sv_newmortal();
        sv_setref_pv( ST(0), "RPM::Transaction", (void*)RETVAL );
    }
    XSRETURN(1);
}


XS(XS_RPM__Transaction_DESTROY); /* prototype to pass -Wmissing-prototypes */
XS(XS_RPM__Transaction_DESTROY)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items != 1)
       croak_xs_usage(cv,  "ts");
    {
	rpmts	ts;

    if (sv_isobject(ST(0)) && (SvTYPE(SvRV(ST(0))) == SVt_PVMG))
        ts = (rpmts)SvIV((SV*)SvRV( ST(0) ));
    else {
        warn( "RPM::Transaction::DESTROY() -- ts is not a blessed SV reference" );
        XSRETURN_UNDEF;
    };
#line 255 "RPM_Transaction.xs"
	(void)rpmtsFree(ts);
	ts = NULL;
#line 348 "RPM_Transaction.c"
    }
    XSRETURN_EMPTY;
}


XS(XS_RPM__Transaction_setrootdir); /* prototype to pass -Wmissing-prototypes */
XS(XS_RPM__Transaction_setrootdir)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items != 2)
       croak_xs_usage(cv,  "ts, root");
    PERL_UNUSED_VAR(ax); /* -Wall */
    SP -= items;
    {
	rpmts	ts;
	char *	root = (char *)SvPV_nolen(ST(1));

    if (sv_isobject(ST(0)) && (SvTYPE(SvRV(ST(0))) == SVt_PVMG))
        ts = (rpmts)SvIV((SV*)SvRV( ST(0) ));
    else {
        warn( "RPM::Transaction::setrootdir() -- ts is not a blessed SV reference" );
        XSRETURN_UNDEF;
    };
#line 263 "RPM_Transaction.xs"
    rpmtsSetRootDir(ts, root);
#line 378 "RPM_Transaction.c"
	PUTBACK;
	return;
    }
}


XS(XS_RPM__Transaction_vsflags); /* prototype to pass -Wmissing-prototypes */
XS(XS_RPM__Transaction_vsflags)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items < 1 || items > 2)
       croak_xs_usage(cv,  "ts, sv_vsflags = NULL");
    {
	rpmts	ts;
	SV *	sv_vsflags;
#line 270 "RPM_Transaction.xs"
    rpmVSFlags vsflags; 
#line 400 "RPM_Transaction.c"
	int	RETVAL;
	dXSTARG;

    if (sv_isobject(ST(0)) && (SvTYPE(SvRV(ST(0))) == SVt_PVMG))
        ts = (rpmts)SvIV((SV*)SvRV( ST(0) ));
    else {
        warn( "RPM::Transaction::vsflags() -- ts is not a blessed SV reference" );
        XSRETURN_UNDEF;
    };

	if (items < 2)
	    sv_vsflags = NULL;
	else {
	    sv_vsflags = ST(1);
	}
#line 272 "RPM_Transaction.xs"
    if (sv_vsflags != NULL) {
        vsflags = sv2constant(sv_vsflags, "rpmvsflags");
        RETVAL = rpmtsSetVSFlags(ts, vsflags);
    } else {
        RETVAL = rpmtsVSFlags(ts);
    }
#line 423 "RPM_Transaction.c"
	XSprePUSH; PUSHi((IV)RETVAL);
    }
    XSRETURN(1);
}


XS(XS_RPM__Transaction_transflags); /* prototype to pass -Wmissing-prototypes */
XS(XS_RPM__Transaction_transflags)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items < 1 || items > 2)
       croak_xs_usage(cv,  "ts, sv_transflag = NULL");
    {
	rpmts	ts;
	SV *	sv_transflag;
#line 286 "RPM_Transaction.xs"
    rpmtransFlags transflags;
#line 445 "RPM_Transaction.c"
	int	RETVAL;
	dXSTARG;

    if (sv_isobject(ST(0)) && (SvTYPE(SvRV(ST(0))) == SVt_PVMG))
        ts = (rpmts)SvIV((SV*)SvRV( ST(0) ));
    else {
        warn( "RPM::Transaction::transflags() -- ts is not a blessed SV reference" );
        XSRETURN_UNDEF;
    };

	if (items < 2)
	    sv_transflag = NULL;
	else {
	    sv_transflag = ST(1);
	}
#line 288 "RPM_Transaction.xs"
    if (sv_transflag != NULL) {
        transflags = sv2constant(sv_transflag, "rpmtransflags");
        /* Take care to rpm config (userland) */
        if (rpmExpandNumeric("%{?_repackage_all_erasures}"))
            transflags |= RPMTRANS_FLAG_REPACKAGE;
        RETVAL = rpmtsSetFlags(ts, transflags);
    } else {
        RETVAL = rpmtsFlags(ts);
    }
#line 471 "RPM_Transaction.c"
	XSprePUSH; PUSHi((IV)RETVAL);
    }
    XSRETURN(1);
}


XS(XS_RPM__Transaction_packageiterator); /* prototype to pass -Wmissing-prototypes */
XS(XS_RPM__Transaction_packageiterator)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items < 1 || items > 4)
       croak_xs_usage(cv,  "ts, sv_tagname = NULL, sv_tagvalue = NULL, keylen = 0");
    PERL_UNUSED_VAR(ax); /* -Wall */
    SP -= items;
    {
	rpmts	ts;
	SV *	sv_tagname;
	SV *	sv_tagvalue;
	int	keylen;

    if (sv_isobject(ST(0)) && (SvTYPE(SvRV(ST(0))) == SVt_PVMG))
        ts = (rpmts)SvIV((SV*)SvRV( ST(0) ));
    else {
        warn( "RPM::Transaction::packageiterator() -- ts is not a blessed SV reference" );
        XSRETURN_UNDEF;
    };

	if (items < 2)
	    sv_tagname = NULL;
	else {
	    sv_tagname = ST(1);
	}

	if (items < 3)
	    sv_tagvalue = NULL;
	else {
	    sv_tagvalue = ST(2);
	}

	if (items < 4)
	    keylen = 0;
	else {
	    keylen = (int)SvIV(ST(3));
	}
#line 307 "RPM_Transaction.xs"
    PUTBACK;
    _newiterator(ts, sv_tagname, sv_tagvalue, keylen);
    SPAGAIN;
#line 524 "RPM_Transaction.c"
	PUTBACK;
	return;
    }
}


XS(XS_RPM__Transaction_dbadd); /* prototype to pass -Wmissing-prototypes */
XS(XS_RPM__Transaction_dbadd)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items != 2)
       croak_xs_usage(cv,  "ts, header");
    {
	rpmts	ts;
	Header	header;
#line 316 "RPM_Transaction.xs"
    rpmdb db;
#line 546 "RPM_Transaction.c"
	int	RETVAL;
	dXSTARG;

    if (sv_isobject(ST(0)) && (SvTYPE(SvRV(ST(0))) == SVt_PVMG))
        ts = (rpmts)SvIV((SV*)SvRV( ST(0) ));
    else {
        warn( "RPM::Transaction::dbadd() -- ts is not a blessed SV reference" );
        XSRETURN_UNDEF;
    };

    if (sv_isobject(ST(1)) && (SvTYPE(SvRV(ST(1))) == SVt_PVMG))
        header = (Header)SvIV((SV*)SvRV( ST(1) ));
    else {
        warn( "RPM::Transaction::dbadd() -- header is not a blessed SV reference" );
        XSRETURN_UNDEF;
    };
#line 318 "RPM_Transaction.xs"
    db = rpmtsGetRdb(ts);
    RETVAL = !rpmdbAdd(db, 0, header, ts);
#line 566 "RPM_Transaction.c"
	XSprePUSH; PUSHi((IV)RETVAL);
    }
    XSRETURN(1);
}


XS(XS_RPM__Transaction_dbremove); /* prototype to pass -Wmissing-prototypes */
XS(XS_RPM__Transaction_dbremove)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items != 2)
       croak_xs_usage(cv,  "ts, sv_offset");
    {
	rpmts	ts;
	SV *	sv_offset = ST(1);
#line 328 "RPM_Transaction.xs"
    rpmdb db;
    unsigned int offset = 0;
#line 589 "RPM_Transaction.c"
	int	RETVAL;
	dXSTARG;

    if (sv_isobject(ST(0)) && (SvTYPE(SvRV(ST(0))) == SVt_PVMG))
        ts = (rpmts)SvIV((SV*)SvRV( ST(0) ));
    else {
        warn( "RPM::Transaction::dbremove() -- ts is not a blessed SV reference" );
        XSRETURN_UNDEF;
    };
#line 331 "RPM_Transaction.xs"
    offset = SvUV(sv_offset);
    db = rpmtsGetRdb(ts);
    if (offset)
        RETVAL = !rpmdbRemove(db, 0, offset, ts);
    else
        RETVAL = 0;
#line 606 "RPM_Transaction.c"
	XSprePUSH; PUSHi((IV)RETVAL);
    }
    XSRETURN(1);
}


XS(XS_RPM__Transaction_add_install); /* prototype to pass -Wmissing-prototypes */
XS(XS_RPM__Transaction_add_install)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items < 3 || items > 4)
       croak_xs_usage(cv,  "ts, h, fn, upgrade = 1");
    {
	rpmts	ts;
	Header	h;
	char *	fn = (char *)SvPV_nolen(ST(2));
	int	upgrade;
#line 348 "RPM_Transaction.xs"
	rpmRC rc = 0;
#line 630 "RPM_Transaction.c"
	int	RETVAL;
	dXSTARG;

    if (sv_isobject(ST(0)) && (SvTYPE(SvRV(ST(0))) == SVt_PVMG))
        ts = (rpmts)SvIV((SV*)SvRV( ST(0) ));
    else {
        warn( "RPM::Transaction::add_install() -- ts is not a blessed SV reference" );
        XSRETURN_UNDEF;
    };

    if (sv_isobject(ST(1)) && (SvTYPE(SvRV(ST(1))) == SVt_PVMG))
        h = (Header)SvIV((SV*)SvRV( ST(1) ));
    else {
        warn( "RPM::Transaction::add_install() -- h is not a blessed SV reference" );
        XSRETURN_UNDEF;
    };

	if (items < 4)
	    upgrade = 1;
	else {
	    upgrade = (int)SvIV(ST(3));
	}
#line 350 "RPM_Transaction.xs"
	rc = rpmtsAddInstallElement(ts, h, (fnpyKey) fn, upgrade, NULL);
	RETVAL = (rc == RPMRC_OK) ? 1 : 0;
#line 656 "RPM_Transaction.c"
	XSprePUSH; PUSHi((IV)RETVAL);
    }
    XSRETURN(1);
}


XS(XS_RPM__Transaction_add_delete); /* prototype to pass -Wmissing-prototypes */
XS(XS_RPM__Transaction_add_delete)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items != 3)
       croak_xs_usage(cv,  "ts, h, offset");
    {
	rpmts	ts;
	Header	h;
	unsigned int	offset = (unsigned int)SvUV(ST(2));
#line 361 "RPM_Transaction.xs"
	rpmRC rc = 0;
#line 679 "RPM_Transaction.c"
	int	RETVAL;
	dXSTARG;

    if (sv_isobject(ST(0)) && (SvTYPE(SvRV(ST(0))) == SVt_PVMG))
        ts = (rpmts)SvIV((SV*)SvRV( ST(0) ));
    else {
        warn( "RPM::Transaction::add_delete() -- ts is not a blessed SV reference" );
        XSRETURN_UNDEF;
    };

    if (sv_isobject(ST(1)) && (SvTYPE(SvRV(ST(1))) == SVt_PVMG))
        h = (Header)SvIV((SV*)SvRV( ST(1) ));
    else {
        warn( "RPM::Transaction::add_delete() -- h is not a blessed SV reference" );
        XSRETURN_UNDEF;
    };
#line 363 "RPM_Transaction.xs"
	rc = rpmtsAddEraseElement(ts, h, offset);
	RETVAL = (rc == RPMRC_OK) ? 1 : 0;
#line 699 "RPM_Transaction.c"
	XSprePUSH; PUSHi((IV)RETVAL);
    }
    XSRETURN(1);
}


XS(XS_RPM__Transaction_element_count); /* prototype to pass -Wmissing-prototypes */
XS(XS_RPM__Transaction_element_count)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items != 1)
       croak_xs_usage(cv,  "ts");
    {
	rpmts	ts;
	int	RETVAL;
	dXSTARG;

    if (sv_isobject(ST(0)) && (SvTYPE(SvRV(ST(0))) == SVt_PVMG))
        ts = (rpmts)SvIV((SV*)SvRV( ST(0) ));
    else {
        warn( "RPM::Transaction::element_count() -- ts is not a blessed SV reference" );
        XSRETURN_UNDEF;
    };
#line 372 "RPM_Transaction.xs"
	RETVAL    = rpmtsNElements(ts);
#line 729 "RPM_Transaction.c"
	XSprePUSH; PUSHi((IV)RETVAL);
    }
    XSRETURN(1);
}


XS(XS_RPM__Transaction_check); /* prototype to pass -Wmissing-prototypes */
XS(XS_RPM__Transaction_check)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items != 1)
       croak_xs_usage(cv,  "ts");
    {
	rpmts	ts;
#line 380 "RPM_Transaction.xs"
	int ret;
#line 750 "RPM_Transaction.c"
	int	RETVAL;
	dXSTARG;

    if (sv_isobject(ST(0)) && (SvTYPE(SvRV(ST(0))) == SVt_PVMG))
        ts = (rpmts)SvIV((SV*)SvRV( ST(0) ));
    else {
        warn( "RPM::Transaction::check() -- ts is not a blessed SV reference" );
        XSRETURN_UNDEF;
    };
#line 382 "RPM_Transaction.xs"
	ret    = rpmtsCheck(ts);
	RETVAL = (ret == 0) ? 1 : 0;
#line 763 "RPM_Transaction.c"
	XSprePUSH; PUSHi((IV)RETVAL);
    }
    XSRETURN(1);
}


XS(XS_RPM__Transaction_order); /* prototype to pass -Wmissing-prototypes */
XS(XS_RPM__Transaction_order)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items != 1)
       croak_xs_usage(cv,  "ts");
    PERL_UNUSED_VAR(ax); /* -Wall */
    SP -= items;
    {
	rpmts	ts;
#line 391 "RPM_Transaction.xs"
	int ret;
    I32 gimme = GIMME_V;
#line 787 "RPM_Transaction.c"

    if (sv_isobject(ST(0)) && (SvTYPE(SvRV(ST(0))) == SVt_PVMG))
        ts = (rpmts)SvIV((SV*)SvRV( ST(0) ));
    else {
        warn( "RPM::Transaction::order() -- ts is not a blessed SV reference" );
        XSRETURN_UNDEF;
    };
#line 394 "RPM_Transaction.xs"
	ret    = rpmtsOrder(ts);
	/* XXX:  May want to do something different here.  It actually
	         returns the number of non-ordered elements...maybe we
	         want this?
	*/
    XPUSHs(sv_2mortal(newSViv(ret == 0 ? 1 : 0)));
    if (gimme != G_SCALAR) {
        XPUSHs(sv_2mortal(newSViv(ret)));
    }
#line 805 "RPM_Transaction.c"
	PUTBACK;
	return;
    }
}


XS(XS_RPM__Transaction__elements); /* prototype to pass -Wmissing-prototypes */
XS(XS_RPM__Transaction__elements)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items != 2)
       croak_xs_usage(cv,  "t, type");
    PERL_UNUSED_VAR(ax); /* -Wall */
    SP -= items;
    {
	rpmts	t;
	rpmElementType	type = (rpmElementType)SvIV(ST(1));
#line 409 "RPM_Transaction.xs"
	rpmtsi       i;
	rpmte        te;
	const char * NEVR;
#line 831 "RPM_Transaction.c"

    if (sv_isobject(ST(0)) && (SvTYPE(SvRV(ST(0))) == SVt_PVMG))
        t = (rpmts)SvIV((SV*)SvRV( ST(0) ));
    else {
        warn( "RPM::Transaction::_elements() -- t is not a blessed SV reference" );
        XSRETURN_UNDEF;
    };
#line 413 "RPM_Transaction.xs"
	i = rpmtsiInit(t);
	if(i == NULL) {
		printf("Did not get a thing!\n");
		return;	
	} else {
		while((te = rpmtsiNext(i, type)) != NULL) {
			NEVR = rpmteNEVR(te);
			XPUSHs(sv_2mortal(newSVpv(NEVR,	0)));
		}
		i = rpmtsiFree(i);
	}
#line 851 "RPM_Transaction.c"
	PUTBACK;
	return;
    }
}


XS(XS_RPM__Transaction_problems); /* prototype to pass -Wmissing-prototypes */
XS(XS_RPM__Transaction_problems)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items != 1)
       croak_xs_usage(cv,  "ts");
    PERL_UNUSED_VAR(ax); /* -Wall */
    SP -= items;
    {
	rpmts	ts;
#line 430 "RPM_Transaction.xs"
    rpmps ps;
#line 874 "RPM_Transaction.c"

    if (sv_isobject(ST(0)) && (SvTYPE(SvRV(ST(0))) == SVt_PVMG))
        ts = (rpmts)SvIV((SV*)SvRV( ST(0) ));
    else {
        warn( "RPM::Transaction::problems() -- ts is not a blessed SV reference" );
        XSRETURN_UNDEF;
    };
#line 432 "RPM_Transaction.xs"
    ps = rpmtsProblems(ts);
    if (ps && rpmpsNumProblems(ps)) /* if no problem, return undef */
        XPUSHs(sv_2mortal(sv_setref_pv(newSVpv("", 0), "RPM::Problems", ps)));
#line 886 "RPM_Transaction.c"
	PUTBACK;
	return;
    }
}


XS(XS_RPM__Transaction_run); /* prototype to pass -Wmissing-prototypes */
XS(XS_RPM__Transaction_run)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items < 2)
       croak_xs_usage(cv,  "ts, callback, ...");
    {
	rpmts	ts;
	SV *	callback = ST(1);
#line 441 "RPM_Transaction.xs"
    int i;
    rpmprobFilterFlags probFilter = RPMPROB_FILTER_NONE;
    rpmInstallInterfaceFlags install_flags = INSTALL_NONE;
    rpmps ps;
#line 911 "RPM_Transaction.c"
	int	RETVAL;
	dXSTARG;

    if (sv_isobject(ST(0)) && (SvTYPE(SvRV(ST(0))) == SVt_PVMG))
        ts = (rpmts)SvIV((SV*)SvRV( ST(0) ));
    else {
        warn( "RPM::Transaction::run() -- ts is not a blessed SV reference" );
        XSRETURN_UNDEF;
    };
#line 446 "RPM_Transaction.xs"
    ts = rpmtsLink(ts, "RPM4 Db::transrun()");
    if (!SvOK(callback)) { /* undef value */
        rpmtsSetNotifyCallback(ts,
                rpmShowProgress,
                (void *) ((long) INSTALL_LABEL | INSTALL_HASH | INSTALL_UPGRADE));
    } else if (SvTYPE(SvRV(callback)) == SVt_PVCV) { /* ref sub */
        rpmtsSetNotifyCallback(ts,
               transCallback,
               (void *) callback);
    } else if (SvTYPE(SvRV(callback)) == SVt_PVAV) { /* array ref */
        install_flags = sv2constant(callback, "rpminstallinterfaceflags");
        rpmtsSetNotifyCallback(ts,
                rpmShowProgress,
                (void *) ((long) install_flags));
    } else {
        croak("Wrong parameter given");
    }

    for (i = 2; i < items; i++)
        probFilter |= sv2constant(ST(i), "rpmprobfilterflags");

    ps = rpmtsProblems(ts);
    RETVAL = rpmtsRun(ts, ps, probFilter);
    ps = rpmpsFree(ps);
    (void)rpmtsFree(ts);
    ts = NULL;
#line 948 "RPM_Transaction.c"
	XSprePUSH; PUSHi((IV)RETVAL);
    }
    XSRETURN(1);
}


XS(XS_RPM__Transaction_closedb); /* prototype to pass -Wmissing-prototypes */
XS(XS_RPM__Transaction_closedb)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items != 1)
       croak_xs_usage(cv,  "ts");
    {
	rpmts	ts;
	int	RETVAL;
	dXSTARG;

    if (sv_isobject(ST(0)) && (SvTYPE(SvRV(ST(0))) == SVt_PVMG))
        ts = (rpmts)SvIV((SV*)SvRV( ST(0) ));
    else {
        warn( "RPM::Transaction::closedb() -- ts is not a blessed SV reference" );
        XSRETURN_UNDEF;
    };
#line 479 "RPM_Transaction.xs"
    RETVAL = !rpmtsCloseDB(ts);
#line 978 "RPM_Transaction.c"
	XSprePUSH; PUSHi((IV)RETVAL);
    }
    XSRETURN(1);
}


XS(XS_RPM__Transaction_opendb); /* prototype to pass -Wmissing-prototypes */
XS(XS_RPM__Transaction_opendb)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items < 1 || items > 2)
       croak_xs_usage(cv,  "ts, write = 0");
    {
	rpmts	ts;
	int	write;
	int	RETVAL;
	dXSTARG;

    if (sv_isobject(ST(0)) && (SvTYPE(SvRV(ST(0))) == SVt_PVMG))
        ts = (rpmts)SvIV((SV*)SvRV( ST(0) ));
    else {
        warn( "RPM::Transaction::opendb() -- ts is not a blessed SV reference" );
        XSRETURN_UNDEF;
    };

	if (items < 2)
	    write = 0;
	else {
	    write = (int)SvIV(ST(1));
	}
#line 488 "RPM_Transaction.xs"
    RETVAL = !rpmtsOpenDB(ts, write ? O_RDWR : O_RDONLY);
#line 1015 "RPM_Transaction.c"
	XSprePUSH; PUSHi((IV)RETVAL);
    }
    XSRETURN(1);
}


XS(XS_RPM__Transaction_initdb); /* prototype to pass -Wmissing-prototypes */
XS(XS_RPM__Transaction_initdb)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items < 1 || items > 2)
       croak_xs_usage(cv,  "ts, write = 0");
    {
	rpmts	ts;
	int	write;
	int	RETVAL;
	dXSTARG;

    if (sv_isobject(ST(0)) && (SvTYPE(SvRV(ST(0))) == SVt_PVMG))
        ts = (rpmts)SvIV((SV*)SvRV( ST(0) ));
    else {
        warn( "RPM::Transaction::initdb() -- ts is not a blessed SV reference" );
        XSRETURN_UNDEF;
    };

	if (items < 2)
	    write = 0;
	else {
	    write = (int)SvIV(ST(1));
	}
#line 497 "RPM_Transaction.xs"
    RETVAL = 1;
#line 1052 "RPM_Transaction.c"
	XSprePUSH; PUSHi((IV)RETVAL);
    }
    XSRETURN(1);
}


XS(XS_RPM__Transaction_rebuilddb); /* prototype to pass -Wmissing-prototypes */
XS(XS_RPM__Transaction_rebuilddb)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items != 1)
       croak_xs_usage(cv,  "ts");
    {
	rpmts	ts;
	int	RETVAL;
	dXSTARG;

    if (sv_isobject(ST(0)) && (SvTYPE(SvRV(ST(0))) == SVt_PVMG))
        ts = (rpmts)SvIV((SV*)SvRV( ST(0) ));
    else {
        warn( "RPM::Transaction::rebuilddb() -- ts is not a blessed SV reference" );
        XSRETURN_UNDEF;
    };
#line 505 "RPM_Transaction.xs"
    RETVAL = !rpmtsRebuildDB(ts);
#line 1082 "RPM_Transaction.c"
	XSprePUSH; PUSHi((IV)RETVAL);
    }
    XSRETURN(1);
}


XS(XS_RPM__Transaction_verifydb); /* prototype to pass -Wmissing-prototypes */
XS(XS_RPM__Transaction_verifydb)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items != 1)
       croak_xs_usage(cv,  "ts");
    {
	rpmts	ts;
	int	RETVAL;
	dXSTARG;

    if (sv_isobject(ST(0)) && (SvTYPE(SvRV(ST(0))) == SVt_PVMG))
        ts = (rpmts)SvIV((SV*)SvRV( ST(0) ));
    else {
        warn( "RPM::Transaction::verifydb() -- ts is not a blessed SV reference" );
        XSRETURN_UNDEF;
    };
#line 513 "RPM_Transaction.xs"
    RETVAL = 1;
#line 1112 "RPM_Transaction.c"
	XSprePUSH; PUSHi((IV)RETVAL);
    }
    XSRETURN(1);
}


XS(XS_RPM__Transaction_readheader); /* prototype to pass -Wmissing-prototypes */
XS(XS_RPM__Transaction_readheader)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items != 2)
       croak_xs_usage(cv,  "ts, filename");
    PERL_UNUSED_VAR(ax); /* -Wall */
    SP -= items;
    {
	rpmts	ts;
	const char *	filename = (const char *)SvPV_nolen(ST(1));

    if (sv_isobject(ST(0)) && (SvTYPE(SvRV(ST(0))) == SVt_PVMG))
        ts = (rpmts)SvIV((SV*)SvRV( ST(0) ));
    else {
        warn( "RPM::Transaction::readheader() -- ts is not a blessed SV reference" );
        XSRETURN_UNDEF;
    };
#line 522 "RPM_Transaction.xs"
    PUTBACK;
    _rpm2header(ts, filename, 0);
    SPAGAIN;
#line 1145 "RPM_Transaction.c"
	PUTBACK;
	return;
    }
}


XS(XS_RPM__Transaction_importpubkey); /* prototype to pass -Wmissing-prototypes */
XS(XS_RPM__Transaction_importpubkey)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items != 2)
       croak_xs_usage(cv,  "ts, filename");
    {
	rpmts	ts;
	char *	filename = (char *)SvPV_nolen(ST(1));
#line 531 "RPM_Transaction.xs"
    uint8_t *pkt = NULL;
    size_t pktlen = 0;
    int rc;
#line 1169 "RPM_Transaction.c"
	int	RETVAL;
	dXSTARG;

    if (sv_isobject(ST(0)) && (SvTYPE(SvRV(ST(0))) == SVt_PVMG))
        ts = (rpmts)SvIV((SV*)SvRV( ST(0) ));
    else {
        warn( "RPM::Transaction::importpubkey() -- ts is not a blessed SV reference" );
        XSRETURN_UNDEF;
    };
#line 535 "RPM_Transaction.xs"
    rpmtsClean(ts);

    if ((rc = pgpReadPkts(filename, (uint8_t ** ) &pkt, &pktlen)) <= 0) {
        RETVAL = 1;
    } else if (rc != PGPARMOR_PUBKEY) {
        RETVAL = 1;
    } else if (rpmcliImportPubkey(ts, pkt, pktlen) != RPMRC_OK) {
        RETVAL = 1;
    } else {
        RETVAL = 0;
    }
    free(pkt);
#line 1192 "RPM_Transaction.c"
	XSprePUSH; PUSHi((IV)RETVAL);
    }
    XSRETURN(1);
}


XS(XS_RPM__Transaction_checkrpm); /* prototype to pass -Wmissing-prototypes */
XS(XS_RPM__Transaction_checkrpm)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items < 2 || items > 3)
       croak_xs_usage(cv,  "ts, filename, sv_vsflags = NULL");
    PERL_UNUSED_VAR(ax); /* -Wall */
    SP -= items;
    {
	rpmts	ts;
	char *	filename = (char *)SvPV_nolen(ST(1));
	SV *	sv_vsflags;
#line 556 "RPM_Transaction.xs"
    rpmVSFlags vsflags = RPMVSF_DEFAULT;
    rpmVSFlags oldvsflags = RPMVSF_DEFAULT;
#line 1218 "RPM_Transaction.c"

    if (sv_isobject(ST(0)) && (SvTYPE(SvRV(ST(0))) == SVt_PVMG))
        ts = (rpmts)SvIV((SV*)SvRV( ST(0) ));
    else {
        warn( "RPM::Transaction::checkrpm() -- ts is not a blessed SV reference" );
        XSRETURN_UNDEF;
    };

	if (items < 3)
	    sv_vsflags = NULL;
	else {
	    sv_vsflags = ST(2);
	}
#line 559 "RPM_Transaction.xs"
    oldvsflags = rpmtsVSFlags(ts); /* keep track of old settings */
    if (sv_vsflags != NULL) {
	    vsflags = sv2constant((sv_vsflags), "rpmvsflags");
        rpmtsSetVSFlags(ts, vsflags);
    }
    PUTBACK;
    _rpm2header(ts, filename, 1); /* Rpmread header is not the most usefull, 
                                   * but no other function in rpmlib allow this :( */
    SPAGAIN;
    rpmtsSetVSFlags(ts, oldvsflags); /* resetting in case of change */
#line 1243 "RPM_Transaction.c"
	PUTBACK;
	return;
    }
}

#ifdef __cplusplus
extern "C"
#endif
XS(boot_RPM__Transaction); /* prototype to pass -Wmissing-prototypes */
XS(boot_RPM__Transaction)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
#if (PERL_REVISION == 5 && PERL_VERSION < 9)
    char* file = __FILE__;
#else
    const char* file = __FILE__;
#endif

    PERL_UNUSED_VAR(cv); /* -W */
    PERL_UNUSED_VAR(items); /* -W */
    XS_VERSION_BOOTCHECK ;

        (void)newXSproto_portable("RPM::Transaction::new", XS_RPM__Transaction_new, file, "$;@");
        (void)newXSproto_portable("RPM::Transaction::DESTROY", XS_RPM__Transaction_DESTROY, file, "$");
        (void)newXSproto_portable("RPM::Transaction::setrootdir", XS_RPM__Transaction_setrootdir, file, "$$");
        (void)newXSproto_portable("RPM::Transaction::vsflags", XS_RPM__Transaction_vsflags, file, "$;$");
        (void)newXSproto_portable("RPM::Transaction::transflags", XS_RPM__Transaction_transflags, file, "$;$");
        (void)newXSproto_portable("RPM::Transaction::packageiterator", XS_RPM__Transaction_packageiterator, file, "$;$$$");
        (void)newXSproto_portable("RPM::Transaction::dbadd", XS_RPM__Transaction_dbadd, file, "$$");
        (void)newXSproto_portable("RPM::Transaction::dbremove", XS_RPM__Transaction_dbremove, file, "$$");
        (void)newXSproto_portable("RPM::Transaction::add_install", XS_RPM__Transaction_add_install, file, "$$$;$");
        (void)newXSproto_portable("RPM::Transaction::add_delete", XS_RPM__Transaction_add_delete, file, "$$$");
        (void)newXSproto_portable("RPM::Transaction::element_count", XS_RPM__Transaction_element_count, file, "$");
        (void)newXSproto_portable("RPM::Transaction::check", XS_RPM__Transaction_check, file, "$");
        (void)newXSproto_portable("RPM::Transaction::order", XS_RPM__Transaction_order, file, "$");
        (void)newXSproto_portable("RPM::Transaction::_elements", XS_RPM__Transaction__elements, file, "$$");
        (void)newXSproto_portable("RPM::Transaction::problems", XS_RPM__Transaction_problems, file, "$");
        (void)newXSproto_portable("RPM::Transaction::run", XS_RPM__Transaction_run, file, "$$;@");
        (void)newXSproto_portable("RPM::Transaction::closedb", XS_RPM__Transaction_closedb, file, "$");
        (void)newXSproto_portable("RPM::Transaction::opendb", XS_RPM__Transaction_opendb, file, "$;$");
        (void)newXSproto_portable("RPM::Transaction::initdb", XS_RPM__Transaction_initdb, file, "$;$");
        (void)newXSproto_portable("RPM::Transaction::rebuilddb", XS_RPM__Transaction_rebuilddb, file, "$");
        (void)newXSproto_portable("RPM::Transaction::verifydb", XS_RPM__Transaction_verifydb, file, "$");
        (void)newXSproto_portable("RPM::Transaction::readheader", XS_RPM__Transaction_readheader, file, "$$");
        (void)newXSproto_portable("RPM::Transaction::importpubkey", XS_RPM__Transaction_importpubkey, file, "$$");
        (void)newXSproto_portable("RPM::Transaction::checkrpm", XS_RPM__Transaction_checkrpm, file, "$$;$");
#if (PERL_REVISION == 5 && PERL_VERSION >= 9)
  if (PL_unitcheckav)
       call_list(PL_scopestack_ix, PL_unitcheckav);
#endif
    XSRETURN_YES;
}

