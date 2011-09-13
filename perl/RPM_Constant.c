/*
 * This file was generated automatically by ExtUtils::ParseXS version 2.2203 from the
 * contents of RPM_Constant.xs. Do not edit this file, edit RPM_Constant.xs instead.
 *
 *	ANY CHANGES MADE HERE WILL BE LOST! 
 *
 */

#line 1 "RPM_Constant.xs"
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#undef Fflush
#undef Mkdir
#undef Stat
#undef Fstat

#include "rpmconstant.h"

#line 22 "RPM_Constant.c"
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

#line 74 "RPM_Constant.c"

XS(XS_RPM__Constant_listallcontext); /* prototype to pass -Wmissing-prototypes */
XS(XS_RPM__Constant_listallcontext)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items != 0)
       croak_xs_usage(cv,  "");
    PERL_UNUSED_VAR(ax); /* -Wall */
    SP -= items;
    {
#line 19 "RPM_Constant.xs"
    rpmconst consti = rpmconstNew();
#line 91 "RPM_Constant.c"
#line 21 "RPM_Constant.xs"
    while (rpmconstNextL(consti)) {
        XPUSHs(sv_2mortal(newSVpv(rpmconstContext(consti), 0)));
    }
    consti = rpmconstFree(consti);
#line 97 "RPM_Constant.c"
	PUTBACK;
	return;
    }
}


XS(XS_RPM__Constant_listcontext); /* prototype to pass -Wmissing-prototypes */
XS(XS_RPM__Constant_listcontext)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items < 1 || items > 2)
       croak_xs_usage(cv,  "context, hideprefix = 1");
    PERL_UNUSED_VAR(ax); /* -Wall */
    SP -= items;
    {
	char *	context = (char *)SvPV_nolen(ST(0));
	int	hideprefix;
#line 31 "RPM_Constant.xs"
    rpmconst consti = rpmconstNew();
#line 121 "RPM_Constant.c"

	if (items < 2)
	    hideprefix = 1;
	else {
	    hideprefix = (int)SvIV(ST(1));
	}
#line 33 "RPM_Constant.xs"
    if (rpmconstInitToContext(consti, context)) {
        while (rpmconstNextC(consti)) {
            XPUSHs(sv_2mortal(newSVpv(rpmconstName(consti, hideprefix), 0)));
        }
    }
    consti = rpmconstFree(consti);
#line 135 "RPM_Constant.c"
	PUTBACK;
	return;
    }
}


XS(XS_RPM__Constant_getvalue); /* prototype to pass -Wmissing-prototypes */
XS(XS_RPM__Constant_getvalue)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items < 2 || items > 3)
       croak_xs_usage(cv,  "context, name, hideprefix = 1");
    PERL_UNUSED_VAR(ax); /* -Wall */
    SP -= items;
    {
	char *	context = (char *)SvPV_nolen(ST(0));
	char *	name = (char *)SvPV_nolen(ST(1));
	int	hideprefix;
#line 46 "RPM_Constant.xs"
    rpmconst consti = rpmconstNew();
    int val = 0;
#line 161 "RPM_Constant.c"

	if (items < 3)
	    hideprefix = 1;
	else {
	    hideprefix = (int)SvIV(ST(2));
	}
#line 49 "RPM_Constant.xs"
    if (rpmconstantFindName(context, name, &val, !hideprefix))
        XPUSHs(sv_2mortal(newSViv(val)));
    consti = rpmconstFree(consti);
#line 172 "RPM_Constant.c"
	PUTBACK;
	return;
    }
}

#ifdef __cplusplus
extern "C"
#endif
XS(boot_RPM__Constant); /* prototype to pass -Wmissing-prototypes */
XS(boot_RPM__Constant)
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

        (void)newXSproto_portable("RPM::Constant::listallcontext", XS_RPM__Constant_listallcontext, file, "");
        (void)newXSproto_portable("RPM::Constant::listcontext", XS_RPM__Constant_listcontext, file, "$;$");
        (void)newXSproto_portable("RPM::Constant::getvalue", XS_RPM__Constant_getvalue, file, "$$;$");
#if (PERL_REVISION == 5 && PERL_VERSION >= 9)
  if (PL_unitcheckav)
       call_list(PL_scopestack_ix, PL_unitcheckav);
#endif
    XSRETURN_YES;
}
