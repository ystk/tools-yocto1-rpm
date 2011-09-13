/*
 * This file was generated automatically by ExtUtils::ParseXS version 2.2203 from the
 * contents of RPM_Problems.xs. Do not edit this file, edit RPM_Problems.xs instead.
 *
 *	ANY CHANGES MADE HERE WILL BE LOST! 
 *
 */

#line 1 "RPM_Problems.xs"
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#undef Fflush
#undef Mkdir
#undef Stat
#undef Fstat

#include "rpmconstant.h"

#line 22 "RPM_Problems.c"
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

#line 74 "RPM_Problems.c"

XS(XS_RPM__Problems_new); /* prototype to pass -Wmissing-prototypes */
XS(XS_RPM__Problems_new)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items != 2)
       croak_xs_usage(cv,  "perlclass, ts");
    PERL_UNUSED_VAR(ax); /* -Wall */
    SP -= items;
    {
	char *	perlclass = (char *)SvPV_nolen(ST(0));
	rpmts	ts;
#line 21 "RPM_Problems.xs"
    rpmps ps;
#line 93 "RPM_Problems.c"

    if (sv_isobject(ST(1)) && (SvTYPE(SvRV(ST(1))) == SVt_PVMG))
        ts = (rpmts)SvIV((SV*)SvRV( ST(1) ));
    else {
        warn( "RPM::Problems::new() -- ts is not a blessed SV reference" );
        XSRETURN_UNDEF;
    };
#line 23 "RPM_Problems.xs"
    ps = rpmtsProblems(ts);
    if (ps &&  rpmpsNumProblems(ps)) /* if no problem, return undef */
        XPUSHs(sv_2mortal(sv_setref_pv(newSVpv("", 0), "RPM::Problems", ps)));
#line 105 "RPM_Problems.c"
	PUTBACK;
	return;
    }
}


XS(XS_RPM__Problems_DESTROY); /* prototype to pass -Wmissing-prototypes */
XS(XS_RPM__Problems_DESTROY)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items != 1)
       croak_xs_usage(cv,  "ps");
    PERL_UNUSED_VAR(ax); /* -Wall */
    SP -= items;
    {
	rpmps	ps;

    if (sv_isobject(ST(0)) && (SvTYPE(SvRV(ST(0))) == SVt_PVMG))
        ps = (rpmps)SvIV((SV*)SvRV( ST(0) ));
    else {
        warn( "RPM::Problems::DESTROY() -- ps is not a blessed SV reference" );
        XSRETURN_UNDEF;
    };
#line 31 "RPM_Problems.xs"
    ps = rpmpsFree(ps);
#line 135 "RPM_Problems.c"
	PUTBACK;
	return;
    }
}


XS(XS_RPM__Problems_count); /* prototype to pass -Wmissing-prototypes */
XS(XS_RPM__Problems_count)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items != 1)
       croak_xs_usage(cv,  "ps");
    {
	rpmps	ps;
	int	RETVAL;
	dXSTARG;

    if (sv_isobject(ST(0)) && (SvTYPE(SvRV(ST(0))) == SVt_PVMG))
        ps = (rpmps)SvIV((SV*)SvRV( ST(0) ));
    else {
        warn( "RPM::Problems::count() -- ps is not a blessed SV reference" );
        XSRETURN_UNDEF;
    };
#line 37 "RPM_Problems.xs"
    RETVAL = rpmpsNumProblems(ps);
#line 165 "RPM_Problems.c"
	XSprePUSH; PUSHi((IV)RETVAL);
    }
    XSRETURN(1);
}


XS(XS_RPM__Problems_print); /* prototype to pass -Wmissing-prototypes */
XS(XS_RPM__Problems_print)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items != 2)
       croak_xs_usage(cv,  "ps, fp");
    PERL_UNUSED_VAR(ax); /* -Wall */
    SP -= items;
    {
	rpmps	ps;
	FILE *	fp = PerlIO_findFILE(IoIFP(sv_2io(ST(1))));

    if (sv_isobject(ST(0)) && (SvTYPE(SvRV(ST(0))) == SVt_PVMG))
        ps = (rpmps)SvIV((SV*)SvRV( ST(0) ));
    else {
        warn( "RPM::Problems::print() -- ps is not a blessed SV reference" );
        XSRETURN_UNDEF;
    };
#line 46 "RPM_Problems.xs"
    rpmpsPrint(fp, ps);
#line 196 "RPM_Problems.c"
	PUTBACK;
	return;
    }
}


XS(XS_RPM__Problems_pb_info); /* prototype to pass -Wmissing-prototypes */
XS(XS_RPM__Problems_pb_info)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items != 2)
       croak_xs_usage(cv,  "ps, numpb");
    PERL_UNUSED_VAR(ax); /* -Wall */
    SP -= items;
    {
	rpmps	ps;
	int	numpb = (int)SvIV(ST(1));
#line 53 "RPM_Problems.xs"
    rpmProblem p;
#line 220 "RPM_Problems.c"

    if (sv_isobject(ST(0)) && (SvTYPE(SvRV(ST(0))) == SVt_PVMG))
        ps = (rpmps)SvIV((SV*)SvRV( ST(0) ));
    else {
        warn( "RPM::Problems::pb_info() -- ps is not a blessed SV reference" );
        XSRETURN_UNDEF;
    };
#line 55 "RPM_Problems.xs"
    if (rpmpsNumProblems(ps) < numpb)
        XSRETURN_UNDEF;
    else {
        p = rpmpsGetProblem(ps, numpb);
        XPUSHs(sv_2mortal(newSVpv("string", 0)));
        XPUSHs(sv_2mortal(newSVpv(rpmProblemString(p), 0)));
        XPUSHs(sv_2mortal(newSVpv("pkg_nevr", 0)));
        XPUSHs(sv_2mortal(newSVpv(rpmProblemGetPkgNEVR(p), 0)));
        XPUSHs(sv_2mortal(newSVpv("alt_pkg_nevr", 0)));
        XPUSHs(sv_2mortal(newSVpv(rpmProblemGetAltNEVR(p), 0)));
        XPUSHs(sv_2mortal(newSVpv("type", 0)));
        XPUSHs(sv_2mortal(newSViv(rpmProblemGetType(p))));
        XPUSHs(sv_2mortal(newSVpv("key", 0)));
        XPUSHs(sv_2mortal(newSVpv(rpmProblemKey(p), 0)));
    }
#line 244 "RPM_Problems.c"
	PUTBACK;
	return;
    }
}

#ifdef __cplusplus
extern "C"
#endif
XS(boot_RPM__Problems); /* prototype to pass -Wmissing-prototypes */
XS(boot_RPM__Problems)
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

        (void)newXSproto_portable("RPM::Problems::new", XS_RPM__Problems_new, file, "$$");
        (void)newXSproto_portable("RPM::Problems::DESTROY", XS_RPM__Problems_DESTROY, file, "$");
        (void)newXSproto_portable("RPM::Problems::count", XS_RPM__Problems_count, file, "$");
        (void)newXSproto_portable("RPM::Problems::print", XS_RPM__Problems_print, file, "$$");
        (void)newXSproto_portable("RPM::Problems::pb_info", XS_RPM__Problems_pb_info, file, "$$");
#if (PERL_REVISION == 5 && PERL_VERSION >= 9)
  if (PL_unitcheckav)
       call_list(PL_scopestack_ix, PL_unitcheckav);
#endif
    XSRETURN_YES;
}
