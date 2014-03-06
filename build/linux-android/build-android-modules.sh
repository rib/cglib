#!/bin/bash

set -e
#set -x

UNSET_VARS=(
    LD_LIBRARY_PATH
    PKG_CONFIG_PATH
    PKG_CONFIG
    GI_TYPELIB_PATH
    XDG_DATA_DIRS
    CMAKE_PREFIX_PATH
    GIO_EXTRA_MODULES
    GST_PLUGIN_PATH
)

EXPORT_VARS=(
    LD_LIBRARY_PATH
    PATH
    PKG_CONFIG_PATH
    PKG_CONFIG
    GI_TYPELIB_PATH
    XDG_DATA_DIRS
    CMAKE_PREFIX_PATH
    GIO_EXTRA_MODULES
    GST_PLUGIN_PATH
    PYTHONPATH
    CFLAGS
    LDFLAGS
    ACLOCAL_FLAGS
)

GL_HEADER_URLS=( \
    http://www.opengl.org/registry/api/glext.h );

GL_HEADERS=( glext.h );

function add_to_path()
{
    local var="$1"; shift;
    local val="$1"; shift;
    local sep;

    if ! echo "${!var}" | grep -q -E "(^|:)$val(:|$)"; then
        if test -z "${!var}"; then
            sep="";
        else
            sep=":";
        fi;
        if test -z "$1"; then
            eval "$var"="${!var}$sep$val";
        else
            eval "$var"="\"$val$sep${!var}\"";
        fi;
    fi;
}

function add_flag()
{
    local var="$1"; shift;
    local letter="$1"; shift;
    local dir="$1"; shift;

    eval "$var"="\"${!var} -$letter$dir\"";
}

function add_prefix()
{
    local prefix;
    local aclocal_dir;
    local x;

    prefix="$1"; shift;

    add_to_path LD_LIBRARY_PATH "$prefix/lib"
    add_to_path PATH "$prefix/bin" backwards
    add_to_path PKG_CONFIG_PATH "$prefix/lib/pkgconfig"
    add_to_path PKG_CONFIG_PATH "$prefix/share/pkgconfig"
    add_to_path GI_TYPELIB_PATH "$prefix/lib/girepository-1.0"
    if test -z "$XDG_DATA_DIRS"; then
        # glib uses these by default if the environment variable is
        # not set and we don't really want to override the default -
        # just add to it so we explicitly add it back in here
        XDG_DATA_DIRS="/usr/local/share/:/usr/share/";
    fi;
    add_to_path XDG_DATA_DIRS "$prefix/share" backwards
    add_to_path CMAKE_PREFIX_PATH "$prefix"
    add_to_path GIO_EXTRA_MODULES "$prefix/lib/gio/modules"
    add_to_path GST_PLUGIN_PATH "$prefix/lib/gstreamer-0.10"
    add_to_path PYTHONPATH "$prefix/lib64/python2.7/site-packages"
    add_flag CFLAGS I "$prefix/include"
    add_flag CXXFLAGS I "$prefix/include"
    add_flag LDFLAGS L "$prefix/lib"
    #add_flag ACLOCAL_FLAGS I "$prefix/share/aclocal"
}

function download_file ()
{
    local url="$1"; shift;
    local filename="$1"; shift;

    if test -f "$DOWNLOAD_DIR/$filename"; then
        echo "Skipping download of $filename because the file already exists";
        return 0;
    fi;

    case "$DOWNLOAD_PROG" in
	curl)
	    curl -L -o "$DOWNLOAD_DIR/$filename" "$url";
	    ;;
	*)
	    $DOWNLOAD_PROG -O "$DOWNLOAD_DIR/$filename" "$url";
	    ;;
    esac;

    if [ $? -ne 0 ]; then
	echo "Downloading ${url} failed.";
	exit 1;
    fi;
}

function guess_dir ()
{
    local var="$1"; shift;
    local suffix="$1"; shift;
    local msg="$1"; shift;
    local prompt="$1"; shift;
    local dir="${!var}";

    if [ -z "$dir" ]; then
	echo "Please enter ${msg}.";
	dir="$PWD/$suffix";
	read -r -p "$prompt [$dir] ";
	if [ -n "$REPLY" ]; then
	    dir="$REPLY";
	fi;
    else
	echo "$prompt = [$dir]"
    fi;

    eval $var="\"$dir\"";
    guess_saves="$guess_saves\n$var=\"$dir\""

    if [ ! -d "$dir" ]; then
	if ! mkdir -p "$dir"; then
	    echo "Error making directory $dir";
	    exit 1;
	fi;
    fi;
}

function y_or_n ()
{
    local prompt="$1"; shift;

    while true; do
	read -p "${prompt} [y/n] " -n 1;
	echo;
	case "$REPLY" in
	    y) return 0 ;;
	    n) return 1 ;;
	    *) echo "Please press y or n" ;;
	esac;
    done;
}

function do_untar_source ()
{
    local tarfile="$1"; shift;

    if echo "$tarfile" | grep -q '\.xz$'; then
        unxz -d -c "$tarfile" | tar -C "$BUILD_DIR" -xvf - "$@"
    elif echo "$tarfile" | grep -q '\.bz2$'; then
        bzip2 -d -c "$tarfile" | tar -C "$BUILD_DIR" -xvf - "$@"
    else
        tar -C "$BUILD_DIR" -zxvf "$tarfile" "$@"
    fi

    if [ "$?" -ne 0 ]; then
	echo "Failed to extract $tarfile"
	exit 1
    fi;
}

function apply_patches ()
{
    local patches_dir="$1"; shift
    local project_dir="$1"; shift

    cd "$project_dir"

    if test -d "$patches_dir"; then
        for patch in "$patches_dir/"*.patch; do
            patch -p1 < "$patch"
            if grep -q '^+++ .*/\(Makefile\.am\|configure\.ac\)\b' "$patch" ; then
                retool=yes;
            fi
        done
    fi
}

function git_clone ()
{
    local url
    local branch
    local dir

    while true; do
        case "$1" in
	    -url) shift; url="$1"; shift ;;
	    -branch) shift; branch="$1"; shift ;;
            -dir) shift; dir="$1"; shift ;;
            -*) echo "Unknown option $1"; exit 1 ;;
            *) break ;;
        esac
    done

    cd $BUILD_DIR

    git clone $url $dir
    if [ $? -ne 0 ]; then
        echo "Cloning ${url} failed."
        exit 1
    fi

    cd $dir
    git checkout -b rig-build $branch
    if [ $? -ne 0 ]; then
        echo "Checking out $branch failed"
        exit 1
    fi
}

function build_source ()
{
    local unpack_dir
    local module
    local project_name
    local build_dir
    local prefix
    local branch
    local config_name="configure"
    local jobs=4
    local config_guess_dirs="."
    local deps
    local make_args=""
    local retool=no
    local retool_cmd

    while true; do
        case "$1" in
	    -module) shift; prefix="$MODULES_DIR/$1"; module="$1"; shift ;;
	    -dep) shift; deps="$deps $1"; shift ;;
            -prefix) shift; prefix="$1"; shift ;;
            -unpackdir) shift; unpack_dir="$1"; shift ;;
            -branch) shift; branch="$1"; shift ;;
            -configure) shift; config_name="$1"; shift ;;
	    -make_arg) shift; make_args="$make_args $1"; shift ;;
	    -retool) shift; retool=yes ;;
	    -retool_cmd) shift; retool_cmd=$1; shift ;;
	    -j) shift; jobs="$1"; shift ;;
	    -autotools_dir) shift; config_guess_dirs="$config_guess_dirs $1"; shift ;;
            -*) echo "Unknown option $1"; exit 1 ;;
            *) break ;;
        esac
    done

    local source="$1"; shift;
    local tarfile=`basename "$source"`

    if test -z "$unpack_dir"; then
        unpack_dir=`echo "$tarfile" | sed -e 's/\.tar\.[0-9a-z]*$//' -e 's/\.tgz$//' -e 's/\.git$//'`
    fi;

    if test -n "$module"; then
      project_name="$module"
      build_dir="$unpack_dir"
    else
      project_name=`echo "$unpack_dir" | sed 's/-[0-9\.]*$//'`
      build_dir="tool-$unpack_dir"
    fi

    #add_to_path LD_LIBRARY_PATH "$prefix/lib"
    add_to_path PKG_CONFIG_PATH "$prefix/lib/pkgconfig"
    add_to_path PKG_CONFIG_PATH "$prefix/share/pkgconfig"

    if test -n "$module"; then
        mkdir -p $MODULES_DIR/$module
        cp $RIG_BUILD_META_DIR/makefiles/$module-Android.mk $MODULES_DIR/$module/Android.mk
    fi

    if test -e "$BUILD_DIR/installed-projects.txt" && \
        grep -q "^$build_dir$" "$BUILD_DIR/installed-projects.txt"; then
        echo "Skipping $project_name ($build_dir) as it is already installed"
        return
    fi

    if ! test -d $BUILD_DIR/$build_dir; then
        if echo "$source" | grep -q "git$"; then
            git_clone -url "$source" -branch "$branch" -dir "$build_dir"
            retool=yes
        else
            download_file "$source" "$tarfile"

            do_untar_source "$DOWNLOAD_DIR/$tarfile"
	    if test "$BUILD_DIR/$unpack_dir" != "$BUILD_DIR/$build_dir"; then
		mv "$BUILD_DIR/$unpack_dir" "$BUILD_DIR/$build_dir"
	    fi
        fi
        apply_patches "$PATCHES_DIR/$project_name" "$BUILD_DIR/$build_dir"
    fi

    cd "$BUILD_DIR/$build_dir"

    for i in $config_guess_dirs
    do
	cp $DOWNLOAD_DIR/config.{sub,guess} $i/
    done

    save_CPPFLAGS=$CPPFLAGS
    save_CFLAGS=$CFLAGS
    save_LDFLAGS=$LDFLAGS

    for dep in $deps
    do
	export CPPFLAGS="-I$MODULES_DIR/$dep/include $CPPFLAGS"
	export CFLAGS="-I$MODULES_DIR/$dep/include $CFLAGS"
	export LDFLAGS="-L$MODULES_DIR/$dep/lib $LDFLAGS"
	if test -d $MODULES_DIR/$dep/share/aclocal; then
	    export ACLOCAL_FLAGS="-I $MODULES_DIR/$dep/share/aclocal $ACLOCAL_FLAGS"
	fi
    done
    export CFLAGS="$CFLAGS $ANDROID_CFLAGS"
    export LDFLAGS="$LDFLAGS $ANDROID_LDFLAGS"

    if ! test -e ./"$config_name"; then
        retool=yes
    fi

    if test "x$retool" = "xyes"; then
	if test -z "$retool_cmd"; then
	    if test -f ./autogen.sh; then
		retool_cmd="./autogen.sh"
	    else
		retool_cmd="autoreconf -fvi"
	    fi
	fi
	NOCONFIGURE=1 eval "$retool_cmd"
    fi

    #Note we have to pass $@ first since we need to pass a special
    #Linux option as the first argument to the icu configure script
    ./"$config_name" "$@" \
        --prefix="$prefix" \
        CFLAGS="$CFLAGS" \
        CXXFLAGS="$CXXFLAGS" \
        LDFLAGS="$LDFLAGS"
    make $make_args -j"$jobs"
    make $make_args install

    CPPFLAGS=$save_CPPFLAGS
    CFLAGS=$save_CFLAGS
    LDFLAGS=$save_LDFLAGS

    find $prefix -iname '*.la' -exec rm {} \;

    echo "$build_dir" >> "$BUILD_DIR/installed-projects.txt"
}

function build_bzip2 ()
{
    local prefix="$MODULES_DIR/bzip2"
    local source="$1"; shift;
    local tarfile=`basename "$source"`
    local unpack_dir=`echo "$tarfile" | sed -e 's/\.tar\.[0-9a-z]*$//' -e 's/\.tgz$//' -e 's/\.git$//'`

    if test -e "$BUILD_DIR/installed-projects.txt" && \
               grep -q "^$unpack_dir$" "$BUILD_DIR/installed-projects.txt"; then
        echo "Skipping bzip2 as it is already installed"
        return
    fi

    download_file "$source" "$tarfile"

    do_untar_source "$DOWNLOAD_DIR/$tarfile"

    apply_patches "$PATCHES_DIR/bzip2" "$BUILD_DIR/$unpack_dir"

    cd "$BUILD_DIR/$unpack_dir"

    make -f Makefile-libbz2_so
    ln -sf libbz2.so.1.0.6 libbz2.so
    cp -av libbz2.so* "$prefix/lib/"

    echo "$unpack_dir" >> "$BUILD_DIR/installed-projects.txt"
}

function build_dep ()
{
    build_source "$@" --host=$ANDROID_HOST_TRIPPLE
}

function build_tool ()
{
    build_source -prefix "$TOOLS_PREFIX" "$@"
}

if test "$1"; then
    RIG_COMMIT=$1; shift
fi

# Please read the README before running this script!

if test -z "$RIG_BUILD_META_DIR"; then
    RIG_BUILD_META_DIR=$PWD
fi
if ! test -f $RIG_BUILD_META_DIR/makefiles/cogl-Android.mk; then
    echo "build-android-modules.sh must be run in Rig's build/ directory"
    exit 1
fi

unset "${UNSET_VARS[@]}"

if test -f .last_directories; then
    echo "Reading previous directory names from .last_directories:"
    . .last_directories
fi

guess_dir PKG_DIR "pkg" \
    "the directory to store the resulting package" "Package dir"
PKG_RELEASEDIR="$PKG_DIR/release"
PKG_LIBDIR="$PKG_RELEASEDIR/lib"
PKG_DATADIR="$PKG_RELEASEDIR/share"

cd `dirname "$0"`
BUILD_DATA_DIR=$PWD

RIG_GITDIR=`cd $BUILD_DATA_DIR/../../.git && pwd`

PATCHES_DIR=$BUILD_DATA_DIR/android-patches
PATCHES_DIR=`cd $PATCHES_DIR && pwd`

# If a download directory hasn't been specified then try to guess one
# but ask for confirmation first
guess_dir DOWNLOAD_DIR "downloads" \
    "the directory to download to" "Download directory";

DOWNLOAD_PROG=curl

# If a download directory hasn't been specified then try to guess one
# but ask for confirmation first
guess_dir MODULES_DIR "modules" \
    "the directory to create android ndk modules in" "Modules directory";

guess_dir TOOLS_PREFIX "android-tools" \
    "the install prefix for internal build dependencies" "Tools dir";

# If a build directory hasn't been specified then try to guess one
# but ask for confirmation first
guess_dir BUILD_DIR "android-build" \
    "the directory to build source dependencies in" "Build directory";

echo -e $guess_saves>.last_directories

for dep in "${GL_HEADER_URLS[@]}"; do
    bn="${dep##*/}";
    download_file "$dep" "$bn";
done;

echo "Copying GL headers...";
if ! test -d "$TOOLS_PREFIX/include/OpenGL"; then
    mkdir -p "$TOOLS_PREFIX/include/OpenGL"
fi;
for header in "${GL_HEADERS[@]}"; do
    cp "$DOWNLOAD_DIR/$header" "$TOOLS_PREFIX/include/OpenGL/"
done;

RUN_PKG_CONFIG="$BUILD_DIR/run-pkg-config.sh";

echo "Generating $BUILD_DIR/run-pkg-config.sh";

cat > "$RUN_PKG_CONFIG" <<EOF
# This is a wrapper script for pkg-config that overrides the
# PKG_CONFIG_LIBDIR variable so that it won't pick up the local system
# .pc files.

# The MinGW compiler on Fedora tries to do a similar thing except that
# it also unsets PKG_CONFIG_PATH. This breaks any attempts to add a
# local search path so we need to avoid using that script.

export PKG_CONFIG_LIBDIR="$TOOLS_PREFIX/lib/pkgconfig"

exec pkg-config "\$@"
EOF

chmod a+x "$RUN_PKG_CONFIG";

add_prefix "$TOOLS_PREFIX"

PKG_CONFIG="$RUN_PKG_CONFIG"

ENV_SCRIPT="$BUILD_DIR/env.sh"
echo > "$ENV_SCRIPT"
for x in "${EXPORT_VARS[@]}"; do
    printf "export %q=%q\n" "$x" "${!x}" >> "$ENV_SCRIPT"
done

export "${EXPORT_VARS[@]}"

download_file "http://git.savannah.gnu.org/gitweb/?p=automake.git;a=blob_plain;f=lib/config.guess" "config.guess";
download_file "http://git.savannah.gnu.org/gitweb/?p=automake.git;a=blob_plain;f=lib/config.sub" "config.sub";

build_tool "http://ftp.gnu.org/gnu/m4/m4-1.4.17.tar.gz"
build_tool "http://ftp.gnu.org/gnu/autoconf/autoconf-2.69.tar.gz"
build_tool "http://ftp.gnu.org/gnu/automake/automake-1.14.1.tar.gz"
build_tool "http://ftp.gnu.org/gnu/help2man/help2man-1.45.1.tar.xz"
#Libtool's Android support is only available from git :-/
build_tool -retool_cmd "./bootstrap" \
    "git://git.savannah.gnu.org/libtool.git"
build_tool "http://pkgconfig.freedesktop.org/releases/pkg-config-0.27.1.tar.gz" \
    --with-internal-glib
build_tool "https://download.gnome.org/sources/gtk-doc/1.20/gtk-doc-1.20.tar.xz"

# The Android NDK comes with a small cpu-features api that is used by pixman
# which, as a special case, needs to be build via ndk-build...
if ! test -d $BUILD_DIR/cpufeatures; then
    mkdir $BUILD_DIR/cpufeatures
    cd $BUILD_DIR/cpufeatures
    ln -s $ANDROID_NDK_DIR/sources/android/cpufeatures jni
    ndk-build APP_MODULES=cpufeatures
    find ./obj -iname libcpufeatures.a -exec mv {} . \;
else
    echo "Skipping cpufeatures as it is already built"
fi


export gl_cv_header_working_stdint_h=yes

#XXX: Note re-autotooling libiconv is awkward so instead of getting
#     it to use a newer version of libtool with Android support, we
#     patch it to use -avoid-version for a pristine soname that
#     Android can cope with.
build_dep -module libiconv -autotools_dir ./build-aux -autotools_dir ./libcharset/build-aux \
    "http://ftp.gnu.org/pub/gnu/libiconv/libiconv-1.14.tar.gz"

build_dep -module libffi -retool_cmd "autoreconf -fvi" -retool \
    "ftp://sourceware.org/pub/libffi/libffi-3.0.13.tar.gz"

#XXX: Note re-autotooling gettext is awkward so instead of getting
#     it to use a newer version of libtool with Android support, we
#     patch it to use -avoid-version for a pristine soname that
#     Android can cope with.
build_dep -module gettext -dep libiconv -autotools_dir ./build-aux \
    -make_arg -C -make_arg gettext-tools/intl \
    "http://ftp.gnu.org/gnu/gettext/gettext-0.18.3.2.tar.gz" \
    --without-included-regex --disable-java --disable-openmp --without-libiconv-prefix --without-libintl-prefix --without-libglib-2.0-prefix --without-libcroco-0.6-prefix --with-included-libxml --without-libncurses-prefix --without-libtermcap-prefix --without-libcurses-prefix --without-libexpat-prefix --without-emacs

#make -C gettext-tools/intl install

export glib_cv_stack_grows=no
export glib_cv_uscore=no
export ac_cv_func_posix_getpwuid_r=no
export ac_cv_func_posix_getgrgid_r=no

#export ACLOCAL_INCLUDES="-I $MODULES_DIR/glib/share/aclocal $ACLOCAL_INCLUDES"
prev_CFLAGS="$CFLAGS"
export CFLAGS="$prev_CFLAGS -g3 -O0"
build_dep -module glib -dep libiconv -dep gettext -branch origin/android -retool \
    "https://github.com/djdeath/glib.git" \
    --with-libiconv=gnu \
    --disable-modular-tests
CFLAGS=$prev_CFLAGS

rm -fr $MODULES_DIR/glib/bin
rm -fr $MODULES_DIR/glib/share/gtk-doc

build_dep -module libpng -retool -retool_cmd "autoreconf -v" \
    "http://downloads.sourceforge.net/project/libpng/libpng16/1.6.9/libpng-1.6.9.tar.gz"
export LIBPNG_CFLAGS="-I$MODULES_DIR/libpng/include"
export LIBPNG_LDFLAGS="-L$MODULES_DIR/libpng/lib -lpng16"

build_dep -module tiff -autotools_dir ./config -retool \
    "http://download.osgeo.org/libtiff/tiff-4.0.3.tar.gz"
build_dep -module libjpeg -retool \
    -unpackdir jpeg-9a "http://www.ijg.org/files/jpegsrc.v9a.tar.gz"

build_dep -module freetype -retool \
    "http://download.savannah.gnu.org/releases/freetype/freetype-2.5.2.tar.bz2"
#    --without-png #XXX: this will disable color emoji support :-(

prev_CFLAGS="$CFLAGS"
export CFLAGS="$prev_CFLAGS -I$BUILD_DIR/cpufeatures/jni"
prev_LDFLAGS="$LDFLAGS"
export LDFLAGS="-L$BUILD_DIR/cpufeatures -lcpufeatures"
build_dep -module pixman -retool \
    "http://www.cairographics.org/releases/pixman-0.32.4.tar.gz" \
    --disable-gtk
CFLAGS=$prev_CFLAGS
LDFLAGS=$prev_LDFLAGS

build_dep -module libxml2 -retool \
    "ftp://xmlsoft.org/libxml2/libxml2-2.9.0.tar.gz" \
    --without-python --without-debug --without-legacy --without-catalog \
    --without-docbook --with-c14n --with-gnu-ld --disable-tests

build_dep -module fontconfig -retool \
    "http://www.freedesktop.org/software/fontconfig/release/fontconfig-2.10.95.tar.bz2" \
    --with-default-fonts=/system/fonts \
    --disable-docs \
    --enable-libxml2

build_dep -module cairo -dep freetype -dep gettext -make_arg V=1 -retool \
    "http://www.cairographics.org/releases/cairo-1.12.16.tar.xz" \
    --disable-xlib \
    --disable-trace

build_dep -module harfbuzz -dep gettext -retool \
    "http://www.freedesktop.org/software/harfbuzz/release/harfbuzz-0.9.26.tar.bz2" \
    --with-icu=no

#XXX: the freetype check is blocked by the fontconfig check, is blocked by the harfbuzz check!
build_dep -module pango -dep glib -dep harfbuzz -dep fontconfig -dep freetype -dep cairo -dep gettext -retool \
    "http://ftp.gnome.org/pub/GNOME/sources/pango/1.36/pango-1.36.2.tar.xz" \
    --disable-introspection \
    --with-included-modules=yes \
    --without-dynamic-modules \
    --without-x

build_dep -module gdk-pixbuf -dep glib -dep libpng -dep tiff -dep libjpeg -dep gettext -retool \
    "http://ftp.gnome.org/pub/GNOME/sources/gdk-pixbuf/2.28/gdk-pixbuf-2.28.2.tar.xz" \
    --disable-introspection \
    --disable-glibtest \
    --disable-modules \
    --without-gdiplus \
    --with-included-loaders=png,jpeg,tiff \
    --disable-gio-sniffing

build_dep -module glib-android -dep glib -dep gettext -retool -retool_cmd "autoreconf -fvi" \
    "https://github.com/dlespiau/glib-android.git"

prev_CFLAGS="$CFLAGS"
export CFLAGS="$prev_CFLAGS -g3 -O0"
build_dep -module sdl2 -branch origin/android -retool -retool_cmd "libtoolize; ./autogen.sh" \
    "https://github.com/rig-project/sdl.git"
CFLAGS=$prev_CFLAGS

prev_CFLAGS="$CFLAGS"
export CFLAGS="$prev_CFLAGS -g3 -O0"
build_dep -module cogl -branch origin/rig -dep glib -dep gdk-pixbuf -dep libiconv -dep gettext -retool \
    "https://github.com/rig-project/cogl.git" \
    --disable-gl \
    --disable-gles1 \
    --enable-gles2 \
    --enable-sdl2 \
    --disable-android-egl-platform \
    --enable-debug
CFLAGS=$prev_CFLAGS

build_tool "http://protobuf.googlecode.com/files/protobuf-2.5.0.tar.gz"

# We have to build protobuf-c twice; once to build the protoc-c tool
# and then again to cross-compile libprotobuf-c.so
build_tool -branch origin/rig \
  "https://github.com/rig-project/protobuf-c.git" \
  --enable-protoc-c

#export CXXFLAGS="-I$STAGING_PREFIX/include"
build_dep -module protobuf-c -branch origin/rig \
  "https://github.com/rig-project/protobuf-c.git" \
  --disable-protoc-c
#unset CXXFLAGS

#prev_CPPFLAGS="$CPPFLAGS"
#export CPPFLAGS="$prev_CPPFLAGS -DANDROID_HARDWARE_generic"
#build_dep -module valgrind -retool_cmd "autoreconf -fvi" \
#    "https://github.com/svn2github/valgrind.git" \
#    AR=$ANDROID_HOST_TRIPPLE-ar \
#    LD=$ANDROID_HOST_TRIPPLE-ld \
#    CC=$ANDROID_HOST_TRIPPLE-gcc \
#    --host=armv7-unknown-linux
#CPPFLAGS="$prev_CPPFLAGS"

versioned_sonames=`$ANDROID_HOST_TRIPPLE-readelf -d $MODULES_DIR/*/lib/*.so|grep 'soname:'|grep -v '.so]$'|awk '{print $5}'`
if test -n "$versioned_sonames"; then
    echo "WARNING: The following versioned module sonames were found, but Android doesn't support library versioning:"
    echo ""
    echo "$versioned_sonames"
    exit 1
fi

exit 0

build_tool "ftp://xmlsoft.org/libxml2/libxml2-2.9.0.tar.gz"
build_dep "http://tukaani.org/xz/xz-5.0.4.tar.gz"

build_dep -C Configure -j 1 \
    "http://mirrors.ibiblio.org/openssl/source/openssl-1.0.1c.tar.gz" \
    linux-elf \
    no-shared

build_bzip2 "http://www.bzip.org/1.0.6/bzip2-1.0.6.tar.gz"

# The makefile for this package seems to choke on paralell builds
build_dep -j 1 "http://freedesktop.org/~hadess/shared-mime-info-1.0.tar.xz"

build_dep \
    "http://downloads.sourceforge.net/project/libpng/libpng16/1.6.3/libpng-1.6.3.tar.xz"
#build_dep \
#    "mirrorservice.org/sites/dl.sourceforge.net/pub/sourceforge/l/li/libjpeg/6b/jpegsr6.tar.gz"


#export CFLAGS="-DUNISTR_FROM_CHAR_EXPLICIT -DUNISTR_FROM_STRING_EXPLICIT=explicit"
#build_dep -unpackdir icu -C source/runConfigureICU "http://download.icu-project.org/files/icu4c/51.1/icu4c-51_1-src.tgz" Linux
#unset CFLAGS

build_dep "http://download.savannah.gnu.org/releases/freetype/freetype-2.4.10.tar.bz2"
build_dep "http://www.freedesktop.org/software/fontconfig/release/fontconfig-2.10.92.tar.bz2" \
    --disable-docs \
    --enable-libxml2
build_dep "http://www.freedesktop.org/software/harfbuzz/release/harfbuzz-0.9.16.tar.bz2"

build_dep "http://www.cairographics.org/releases/pixman-0.28.0.tar.gz" \
    --disable-gtk

#NB: we make sure to build cairo after freetype/fontconfig so we have support for these backends
build_dep "http://www.cairographics.org/releases/cairo-1.12.8.tar.xz" \
    --enable-xlib

build_dep "http://ftp.gnome.org/pub/GNOME/sources/pango/1.34/pango-1.34.1.tar.xz" \
    --disable-introspection \
    --with-included-modules=yes \
    --without-dynamic-modules

build_dep -b rig "https://github.com/rig-project/sdl.git"

build_dep "http://ftp.gnu.org/gnu/gdbm/gdbm-1.10.tar.gz"

build_dep "http://dbus.freedesktop.org/releases/dbus/dbus-1.7.2.tar.gz"

build_dep "http://0pointer.de/lennart/projects/libdaemon/libdaemon-0.14.tar.gz"

export CPPFLAGS="-I$STAGING_PREFIX/include"
build_dep "http://avahi.org/download/avahi-0.6.31.tar.gz" \
  --disable-qt3 \
  --disable-qt4 \
  --disable-gtk \
  --disable-gtk3 \
  --disable-python \
  --disable-mono \
  --disable-introspection
unset CPPFLAGS

build_dep "http://gstreamer.freedesktop.org/src/gstreamer/gstreamer-1.0.7.tar.xz"
build_dep "http://gstreamer.freedesktop.org/src/gst-plugins-base/gst-plugins-base-1.0.7.tar.xz"

export CFLAGS="-g3 -O0"

build_dep \
    -b rig \
    "https://github.com/rig-project/cogl.git" \
    --enable-cairo \
    --disable-profile \
    --enable-gdk-pixbuf \
    --enable-quartz-image \
    --disable-examples-install \
    --disable-gles1 \
    --disable-gles2 \
    --enable-gl \
    --disable-cogl-gles2 \
    --disable-glx \
    --disable-wgl \
    --enable-sdl2 \
    --disable-gtk-doc \
    --enable-glib \
    --enable-cogl-pango \
    --enable-cogl-gst \
    --disable-introspection \
    --enable-debug

if test -z $RIG_COMMIT; then
    build_dep -d rig "$RIG_GITDIR"
else
    build_dep -d rig -c "$RIG_COMMIT" "$RIG_GITDIR"
fi

mkdir -p "$PKG_RELEASEDIR"

cp -v "$STAGING_PREFIX/bin/rig" "$PKG_RELEASEDIR/rig-bin"

mkdir -p "$PKG_LIBDIR"

for lib in \
    libSDL2 \
    libasprintf \
    libavahi-client \
    libavahi-common \
    libavahi-core \
    libavahi-glib \
    libbz2 \
    libcairo \
    libcogl-gst \
    libcogl-pango2 \
    libcogl-path \
    libcogl2 \
    libdaemon \
    libdbus-1 \
    libffi \
    libfontconfig \
    libfreetype \
    libgdk_pixbuf-2.0 \
    libgettextlib-0.18.2 \
    libgettextlib \
    libgettextpo \
    libgettextsrc-0.18.2 \
    libgettextsrc \
    libgio-2.0 \
    libglib-2.0 \
    libgmodule-2.0 \
    libgobject-2.0 \
    libgthread-2.0 \
    libgstfft-1.0 \
    libgstaudio-1.0 \
    libgstvideo-1.0 \
    libgsttag-1.0 \
    libgstcontroller-1.0 \
    libgstbase-1.0 \
    libgstreamer-1.0 \
    libgstfft-1.0 \
    libgstaudio-1.0 \
    libgstvideo-1.0 \
    libgstbase-1.0 \
    libgsttag-1.0 \
    libgstcontroller-1.0 \
    libgstreamer-1.0 \
    libharfbuzz \
    libjpeg \
    liblzma \
    libpango-1.0 \
    libpangocairo-1.0 \
    libpangoft2-1.0 \
    libpixman-1 \
    libpng \
    libpng16 \
    libprotobuf-c \
    libtiff \
    libtiffxx \
    libxml2 \
    preloadable_libintl
do
    cp -av $STAGING_PREFIX/lib/$lib*.so* "$PKG_LIBDIR"
done

mkdir -p "$PKG_LIBDIR"/gio
cp -av $STAGING_PREFIX/lib/gio/modules "$PKG_LIBDIR"/gio

cp -av "$BUILD_DATA_DIR/scripts/"{auto-update.sh,rig-wrapper.sh} \
    "$BUILD_DIR/rig/tools/rig-check-signature" \
    "$PKG_RELEASEDIR"

mkdir -p "$PKG_DATADIR/pixmaps"
cp -av $STAGING_PREFIX/share/pixmaps/rig* $PKG_DATADIR/pixmaps

mkdir -p "$PKG_DATADIR/applications"
cp -av $STAGING_PREFIX/share/applications/rig* $PKG_DATADIR/applications

mkdir -p "$PKG_DATADIR/mime/application"
cp -av $STAGING_PREFIX/share/mime/application/rig-* $PKG_DATADIR/mime/application

cd $PKG_DIR
ln -sf ./release/rig-wrapper.sh $PKG_DIR/rig

mkdir -p $PKG_DIR/cache

mkdir -p "$PKG_DATADIR/rig"

cp -av "$STAGING_PREFIX/share/rig"/* "$PKG_DATADIR/rig"
