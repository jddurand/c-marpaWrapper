#!env perl
use strict;
use diagnostics;
use Config::AutoConf::INI;
use ExtUtils::CBuilder;
use File::chdir;
use File::Basename qw/basename dirname/;
use File::Find qw/find/;
use File::Path qw/make_path/;
use File::Copy::Recursive qw/dircopy/;
use IPC::Run qw/run/;
use POSIX qw/EXIT_SUCCESS/;
use Try::Tiny;

#
# ExtUtils::CBuilder custom configuration from environment:
# Any environment variable PERL_EXTUTILS_CBUILDER_CONFIG_\w+ is
# considered
#
my %EXTUTILS_BUILDER_CONFIG =
    map {
        my $key = $_;
        $key =~ s/^PERL_EXTUTILS_CBUILDER_CONFIG_//;
        $key => $ENV{$_}
    }
    grep {
        $_ =~ /^PERL_EXTUTILS_CBUILDER_CONFIG_\w+/
    } keys %ENV;

# ------------------------------------------------------------
# Generation of objects using perl setup, for use in perl's XS
# ------------------------------------------------------------
my $version = get_version();
print "Generating config for marpaWrapper version $version\n";

# ------------------------
# Write configuration file
# ------------------------
my $config_h_in = File::Spec->catfile('include', 'config.h.in');
my $config_h    = File::Spec->catfile('output', 'include', 'config.h');
make_path(dirname($config_h));

my $ac = Config::AutoConf::INI->new(logfile => 'config.log');
$ac->define_var('MARPA_LIB_MAJOR_VERSION', 'MARPA_MAJOR_VERSION');
$ac->define_var('MARPA_LIB_MICRO_VERSION', 'MARPA_MICRO_VERSION');
$ac->define_var('MARPA_LIB_MINOR_VERSION', 'MARPA_MINOR_VERSION');
$ac->define_var('MARPAWRAPPER_NTRACE', 1);

$ac->msg_checking("for __builtin_expect()");
my $__builtin_expect = <<__BUILTIN_EXPECT;
#include <stdlib.h>

#define C__BUILTIN_EXPECT __builtin_expect
#define C_LIKELY(x)    C__BUILTIN_EXPECT(!!(x), 1)
#define C_UNLIKELY(x)  C__BUILTIN_EXPECT(!!(x), 0)

/* Copied from https://kernelnewbies.org/FAQ/LikelyUnlikely */
int main(char *argv[], int argc)
{
   int a;

   /* Get the value from somewhere GCC can't optimize */
   a = atoi (argv[1]);

   if (C_UNLIKELY(a == 2))
      a++;
   else
      a--;

   return 0;
}
__BUILTIN_EXPECT
    if (try_compile($__builtin_expect)) {
        $ac->msg_result('yes');
        $ac->define_var("C___BUILTIN_EXPECT", "__builtin_expect");
    } else {
        $ac->msg_result('no');
    }

foreach my $inline_keyword (qw/inline __inline__ inline__ __inline/) {
    $ac->msg_checking("for $inline_keyword");
    my $inline_source = <<INLINE_SOURCE;
typedef int foo_t;
static $inline_keyword foo_t static_foo() {
  return 0;
}
foo_t foo() {
  return 0;
}
int main(int argc, char *argv[]){
  return 0;
}
INLINE_SOURCE
    if (try_compile($inline_source)) {
        $ac->msg_result('yes');
        $ac->define_var("C_INLINE", $inline_keyword);
        if ($inline_keyword eq 'inline') {
            $ac->define_var("C_INLINE_IS_INLINE", $inline_keyword);
        }
        last;
    } else {
        $ac->msg_result('no');
    }
}

$ac->check;
write_config($version, $config_h_in, $config_h);

# -------------
# Fake export.h
# -------------
my $export_h = File::Spec->catfile('output', 'include', 'marpaWrapper', 'export.h');
make_path(dirname($export_h));
open(my $fh, '>', $export_h) || die "Cannot open $export_h, $!";
print $fh "#define marpaWrapper_EXPORT\n";
print $fh "#define MARPAWRAPPER_NO_EXPORT\n";
close($fh) || warn "Cannot close $export_h, $!";

my @additional_includes = ();
my @additional_srcdir = ();
# -----------------------------------------------------
# We depend on marpa third party that we manage ourself
# -----------------------------------------------------
print "... Copying Marpa--R2\n";
if (! dircopy(File::Spec->catdir('3rdparty', 'github', 'Marpa--R2'),
              File::Spec->catdir('3rdparty', 'output', 'Marpa--R2'))) {
  die 'Failed to copy Marpa--R2';
}
push(@additional_includes, File::Spec->catdir('3rdparty', 'output', 'Marpa--R2', 'cpan', 'engine', 'read_only'));
push(@additional_srcdir, File::Spec->catdir('3rdparty', 'output', 'Marpa--R2', 'cpan', 'engine', 'read_only'));

# -----------------------------------------------------------------------
# We depend on these generic* stuff that can manage themselvs, eventually
# -----------------------------------------------------------------------
foreach (qw/genericLogger genericStack genericHash genericSparseArray/) {
    if (! dircopy(File::Spec->catdir('3rdparty', 'github', $_),
                  File::Spec->catdir('output', '3rdparty', $_))) {
        die "Failed to copy $_";
    }
    print "... Copying $_\n";
    my $CMakeObjects = File::Spec->catfile('output', '3rdparty', $_, 'CMakeObjects.PL');
    if (-e $CMakeObjects) {
        print "... Executing $CMakeObjects\n";
        my $in;
        my $out;
        my $err;
        {
            local $CWD = dirname($CMakeObjects);
            run([$^X, basename($CMakeObjects)], \$in, \$out, \$err) || die "Failed to execute $CMakeObjects" . ($err ? "\n$err" : '');
        }
        if ($out) {
            foreach (grep {defined} split(/\R/, $out)) {
                print "... ... $_\n";
            }
        }
        my $inc = File::Spec->catdir('output', '3rdparty', $_, 'output', 'include');
        push(@additional_includes, $inc) if (-d $inc);
    }
    my $inc = File::Spec->catdir('output', '3rdparty', $_, 'include');
    push(@additional_includes, $inc) if (-d $inc);
}

# ----------------
# Get source files
# ----------------
my @sources;
find(
    {
	no_chdir => 1,
	wanted => sub {
	    my $file = File::Spec->canonpath($_);
	    if (-f $file && ($file =~ /\.c$/ || $file =~ /\.cpp$/)) {
		push(@sources, $file)
	    }
	},
    },
    'src', @additional_srcdir);

# ----------------------------------------------------------------------------------------
# Generate objects
# (individually- not efficient but I do not see how CBuilder mixes C and C++ source files)
# ----------------------------------------------------------------------------------------
my $cbuilder = ExtUtils::CBuilder->new(config => \%EXTUTILS_BUILDER_CONFIG);
my @objects;
my $obj_dir = File::Spec->catfile('output', 'obj4perl');

make_path($obj_dir);
foreach my $source (@sources) {
    my $is_cplusplus = ($source =~ /\.cpp$/i || $source =~ /\.c\+\+$/i);
    my $obj = basename($cbuilder->object_file($source));
    $obj = File::Spec->catfile($obj_dir, basename($cbuilder->object_file($source)));
    push(@objects, $cbuilder->object_file($source));
    $cbuilder->compile(
	source        => $source,
	object_file   => $obj,
	include_dirs  => [ 'include', File::Spec->catdir('output', 'include'), @additional_includes ],
	'C++'         => $is_cplusplus
	);
}

# ----
# Done
# ----
exit(EXIT_SUCCESS);

sub try_compile {
    my ($csource, $extra_compiler_flags) = @_;

    my $fh = File::Temp->new(UNLINK => 0, SUFFIX => '.c');
    print $fh "$csource\n";
    close($fh);
    my $source = $fh->filename;
    my $rc = 0;

    try {
        my $cbuilder = ExtUtils::CBuilder->new(config => \%EXTUTILS_BUILDER_CONFIG, quiet => 1);
        my $obj = basename($cbuilder->object_file($source));
        $cbuilder->compile(
            source               => $source,
            object_file          => $obj,
            extra_compiler_flags => $extra_compiler_flags
            );
        $rc = 1;
    };
    unlink $fh->filename;

    return $rc;
}

sub get_version {
    open(my $fh, '<', 'CMakeLists.txt') || die "Cannot open CMakeLists.txt, $!";
    my $content = do { local $/; <$fh>; };
    close($fh) || warn "Failed to close CMakeLists.txt, $!";

    my @rc;
    if ($content =~ /^MYPACKAGESTART\s*\(\s*marpaWrapper\s+(\d+)\s+(\d+)\s+(\d+)\s*\)/sm) {
        @rc = ($1, $2, $3);
    } else {
        foreach (qw/MARPAWRAPPER_VERSION_MAJOR MARPAWRAPPER_VERSION_MINOR MARPAWRAPPER_VERSION_PATCH/) {
            if ($content =~ /^SET\s*\(\s*$_\s*(\d+)\s*\)/sm) {
                push(@rc, $1);
            } else {
                die "Failed to find $_",
            }
        }
    }

    return join('.', @rc)
}

sub write_config {
    my ($version, $input, $output) = @_;

    make_path(dirname($output));
    open(my $fh, '<', $input) || die "Cannot open $input, $!";
    my $source = do { local $/; <$fh>; };
    close($fh) || warn "Cannot close $input, $!";

    $source =~ s/^[ \t]*#[ \t]*cmakedefine[ \t]+(\w+)+[ \t]+\@([^ \t@]*)\@//smg;

    open($fh, '>', $output) || die "Cannot open $output, $!";
    print $fh <<CONFIG;
#ifndef MARPAWRAPPER_CONFIG_WRAPPER_H
#define MARPAWRAPPER_CONFIG_WRAPPER_H

#include <marpaWrapper/config_autoconf.h>

#define MARPAWRAPPER_VERSION "$version"
   
$source

#endif /* MARPAWRAPPER_CONFIG_WRAPPER_H */
CONFIG
    close($fh) || warn "Cannot close $output, $!";
}
