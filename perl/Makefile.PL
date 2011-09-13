package MY; # so that "SUPER" works right

use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.

my @libdir = qw(. build lib popt rpmdb rpmio misc rpmconstant);

my @ldaddp = map { '-L../' . $_ . '/.libs' } @libdir;
my @ldadd = map { '-l' . $_ } qw(rpmmisc rpmio rpmbuild rpm popt rpmdb rpmconstant);

my @cppflags = qw(
    -I.
    -I..
    -I../lib
    -I../lib
);

my @objects = qw(
    rpmxs.o 
    RPM.o 
    RPM_Constant.o
    RPM_Header.o
    RPM_Transaction.o
    RPM_PackageIterator.o
    RPM_Problems.o
    RPM_Spec.o
    RPM_Files.o
    RPM_Dependencies.o
);

# teach MakeMaker some Mac OS X LDFLAGS:
my @LDFLAGS = qw(         -L/usr/lib6464         -lpthread -lrt  -lelf $(CCCDLFLAGS) $(CCDLFLAGS) -L$(PERL_ARCHLIB)/CORE -lperl );
my $uname = `uname`; chomp($uname);
foreach (@LDFLAGS) {
    if ("$uname" eq "Darwin" && m/^(-arch|ppc|i386|ppc64|x86_64)$/) {
         push (@ldflags, $&);
    } elsif ("$uname" eq "Darwin" && m/-Wl,-search_paths_first/) {
         push (@ldflags, $&);
    } else {
         push (@ldlibs, $_);    
    }
}

# teach MakeMaker some VPATH $< manners:
sub c_o {
    my $inherited = shift->SUPER::c_o(@_);
    $inherited =~ s/(\.\w+\$\(OBJ_EXT\)\:\n\t.*?)\$\*\.\w+/$1\$\</g;
    $inherited;
}
sub xs_c {
    my $inherited = shift->SUPER::xs_c(@_);
    $inherited =~ s/(\.xs\.c\:\n\t.*?)\$\*\.\w+/$1\$\</g;
    $inherited;
}
sub xs_o {
    my $inherited = shift->SUPER::xs_o(@_);
    $inherited =~ s/(\.\w+\$\(OBJ_EXT\)\:\n\t.*?)\$\*\.\w+/$1\$\</g;
    $inherited;
}

WriteMakefile(
	      'NAME'	=> 'RPM',
	      'OBJECT'	=> join(' ', @objects),
	      'VERSION' => '5.4.2',
	      'MAKEFILE'=> 'Makefile.perl',
	      'CCFLAGS' => join(' ', '-g -O2 -fPIC -DPIC -Wall -W -Wpointer-arith -Wstrict-prototypes -Wmissing-prototypes -Wno-char-subscripts -Wno-unused-parameter -Wformat-security -g3 -fno-delete-null-pointer-checks -D_FORTIFY_SOURCE=2 -fstack-protector -fexceptions -fexceptions -Wno-missing-field-initializers -D_GNU_SOURCE -D_REENTRANT        ', '-Wno-unused-variable', '-Wno-missing-prototypes', '-Wno-implicit', '-Wno-strict-prototypes'),
	      'dynamic_lib'	=> { OTHERLDFLAGS => join(' ', @ldaddp, @ldadd, @ldlibs) },
	      'OPTIMIZE'=> '-g',
	      'INC'	=> join(' ', @cppflags, map { '-I../'. $_ } @libdir) . '  -DRPM_OS_LINUX=020632   -I/usr/include/nss3 -I/usr/include/nspr4     -I/usr/include/python2.6 -I/usr/include/python2.6  ',
	      'INSTALLMAN1DIR'	=> '${prefix}/share/man/man1',
	      'INSTALLMAN3DIR'	=> '${prefix}/share/man/man3',
	      
);

