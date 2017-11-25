project('cglib', 'c',
        default_options : ['c_std=c11', 'cpp_std=c++14'],
        license: ['MIT'])
clib_sp = subproject('clib')
clib_dep = clib_sp.get_variable('clib_dep')

support_libuv = get_option('support_libuv')
support_glib = get_option('support_glib')
support_x11 = get_option('support_x11')

support_sdl = get_option('support_sdl')
support_glx = get_option('support_glx')
support_egl = get_option('support_egl')
support_egl_kms = get_option('support_egl_kms')
support_egl_wayland = get_option('support_egl_wayland')
support_egl_xlib = get_option('support_egl_xlib')

support_gles2 = get_option('support_gles2')

thread_dep = dependency('threads')

compiler = meson.get_compiler('c')
math_dep = compiler.find_library('m', required : false)

deps = [ thread_dep, math_dep, clib_dep ]

public_inc = include_directories('.')
inc = include_directories(
        '.',
        'cglib',
	'cglib/winsys',
	'cglib/driver/nop',
	'cglib/driver/gl',
	'cglib/driver/gl/gl',
	'cglib/driver/gl/gles')

public_defines = []

c_defines = [
        '-DCG_COMPILATION',
#        '-DENABLE_UNIT_TESTS',
]

warn_opts = [
        '-Wall',
        '-Wextra',
        '-Wno-unused-parameter',
]

sys_libs = []

src = [
    'cglib-config.h',

    'cglib/cglib.h',
    'cglib/cglib.c',

    'cglib/cg-bitmask.c',

    'cglib/cg-display.h',
    'cglib/cg-display-private.h',
    'cglib/cg-display.c',

    'cglib/cg-renderer.h',
    'cglib/cg-renderer-private.h',
    'cglib/cg-renderer.c',

    'cglib/cg-pipeline.h',
    'cglib/cg-pipeline-private.h',
    'cglib/cg-pipeline.c',
    'cglib/cg-pipeline-state.h',
    'cglib/cg-pipeline-state-private.h',
    'cglib/cg-pipeline-state.c',
    'cglib/cg-pipeline-hash-table.h',
    'cglib/cg-pipeline-hash-table.c',
    'cglib/cg-pipeline-cache.h',
    'cglib/cg-pipeline-cache.c',
    'cglib/cg-pipeline-layer-private.h',
    'cglib/cg-pipeline-layer.c',
    'cglib/cg-pipeline-layer-state-private.h',
    'cglib/cg-pipeline-layer-state.h',
    'cglib/cg-pipeline-layer-state.c',
    'cglib/cg-pipeline-snippet-private.h',
    'cglib/cg-pipeline-snippet.c',
    'cglib/cg-pipeline-debug.c',

    'cglib/cg-attribute-buffer.c',
    'cglib/cg-output.h',
    'cglib/cg-pixel-format-private.h',
    'cglib/cg-rectangle-map.c',
    'cglib/cg-util.c',
    'cglib/cg-frame-info-private.h',
    'cglib/cg-point-in-poly-private.h',
    'cglib/cg-object.c',
    'cglib/cg-atlas-texture.h',
    'cglib/cg-matrix-stack.c',
    'cglib/cg-primitive-private.h',

    'cglib/cg-feature-private.c',
    'cglib/cg-gpu-info-private.h',
    'cglib/cg-snippet-private.h',
    'cglib/cg-framebuffer.c',
    'cglib/cg-primitive-texture.h',

    'cglib/cg-object.h',
    'cglib/cg-bitmask.h',
    'cglib/cg-driver.h',
    'cglib/cg-texture-2d.c',
    'cglib/cg-texture-2d.h',
    'cglib/cg-debug.c',
    'cglib/cg-framebuffer-private.h',
    'cglib/cg-blend-string.c',
    'cglib/cg-indices-private.h',
    'cglib/cg-spans.h',
    'cglib/cg-onscreen-template.c',
    'cglib/cg-snippet.h',
    'cglib/cg-fence.c',
    'cglib/cg-memory-stack.c',

    'cglib/cg-gles2-context-private.h',
    'cglib/cg-point-in-poly.c',

    'cglib/cg-blit.h',
    'cglib/cg-blit.c',
    'cglib/cg-onscreen-private.h',
    'cglib/cg-indices.h',
    'cglib/cg-primitive.h',
    'cglib/cg-flags.h',
    'cglib/cg-clip-stack.h',
    'cglib/cg-onscreen.h',
    'cglib/cg-framebuffer.h',
    'cglib/cg-atlas.h',
    'cglib/cg-frame-info.c',
    'cglib/cg-object-private.h',
    'cglib/cg-matrix-stack.h',
    'cglib/cg-atlas-set-private.h',
    'cglib/cg-private.h',
    'cglib/cg-index-buffer.c',
    'cglib/cg-sampler-cache-private.h',
    'cglib/cg-device.h',

    'cglib/cg-glsl-shader.c',

    'cglib/cg-buffer-private.h',
    'cglib/cg-texture-2d-sliced.c',
    'cglib/cg-texture-2d-sliced.h',
    'cglib/cg-depth-state.c',

    'cglib/cg-bitmap-private.h',
    'cglib/cg-bitmap.h',
    'cglib/cg-bitmap.c',
    'cglib/cg-bitmap-pack.h',
    'cglib/cg-bitmap-unpack-unsigned-normalized.h',
    'cglib/cg-bitmap-unpack-fallback.h',
    'cglib/cg-bitmap-conversion.c',
    'cglib/cg-bitmap-pixbuf.c',

    'cglib/cg-error.h',
    'cglib/cg-atlas-set.h',
    'cglib/cg-texture-3d.h',
    'cglib/cg-color.c',
    'cglib/cg-atlas-texture.c',
    'cglib/cg-texture.h',
    'cglib/cg-spans.c',
    'cglib/cg-loop.c',
    'cglib/cg-primitive-texture.c',
    'cglib/cg-pixel-buffer.c',
    'cglib/cg-i18n-private.h',
    'cglib/cg-config-private.h',
    'cglib/cg-buffer.c',
    'cglib/cg-attribute-buffer.h',
    'cglib/cg-types.h',
    'cglib/cg-memory-stack-private.h',
    'cglib/cg-color.h',
    'cglib/cg-glsl-shader-boilerplate.h',
    'cglib/cg-loop-private.h',
    'cglib/cg-texture-3d.c',
    'cglib/cg-debug.h',
    'cglib/cg-version.h',
    'cglib/cg-fence-private.h',
    'cglib/cg-offscreen.h',
    'cglib/cg-closure-list-private.h',
    'cglib/cg-error-private.h',
    'cglib/cg-gpu-info.c',
    'cglib/cg-matrix-stack-private.h',
    'cglib/cg-index-buffer-private.h',
    'cglib/cg-magazine-private.h',
    'cglib/cg-node-private.h',
    'cglib/cg-atlas-set.c',
    'cglib/cg-attribute-private.h',
    'cglib/cg-blend-string.h',
    'cglib/cg-attribute.h',
    'cglib/cg-pixel-format.c',
    'cglib/cg-texture-2d-sliced-private.h',
    'cglib/cg-loop.h',
    'cglib/cg-texture-private.h',
    'cglib/cg-fence.h',
    'cglib/cg-depth-state.h',
    'cglib/cg-attribute.c',
    'cglib/cg-onscreen-template-private.h',
    'cglib/cg-texture.c',
    'cglib/cg-defines.h',
    'cglib/cg-pixel-buffer-private.h',
    'cglib/cg-index-buffer.h',
    'cglib/cg-glsl-shader-private.h',
    'cglib/cg-sub-texture.c',
    'cglib/cg-sampler-cache.c',
    'cglib/cg-texture-2d-gl.h',
    'cglib/cg-gles2-types.h',
    'cglib/cg-magazine.c',
    'cglib/cg-rectangle-map.h',
    'cglib/cg-color-private.h',
    'cglib/cg-device.c',
    'cglib/cg-snippet.c',
    'cglib/cg-profile.c',
    'cglib/cg-texture-3d-private.h',
    'cglib/cg-texture-driver.h',
    'cglib/cg-atlas-texture-private.h',
    'cglib/cg-util.h',
    'cglib/cg-atlas-private.h',
    'cglib/cg-error.c',
    'cglib/cg-onscreen.c',
    'cglib/cg-meta-texture.h',
    'cglib/cg-primitive.c',
    'cglib/cg-meta-texture.c',
    'cglib/cg-debug-options.h',
    'cglib/cg-indices.c',
    'cglib/cg-onscreen-template.h',
    'cglib/cg-clip-stack.c',
    'cglib/cg-depth-state-private.h',
    'cglib/cg-boxed-value.c',
    'cglib/cg-buffer.h',
    'cglib/cg-closure-list.c',
    'cglib/cg-atlas.c',
    'cglib/cg-frame-info.h',
    'cglib/cg-output-private.h',
    'cglib/cg-profile.h',
    'cglib/cg-texture-2d-private.h',
    'cglib/cg-attribute-buffer-private.h',
    'cglib/cg-config.c',
    'cglib/cg-sub-texture-private.h',
    'cglib/cg-boxed-value.h',
    'cglib/cg-output.c',
    'cglib/cg-device-private.h',
    'cglib/cg-node.c',
    'cglib/cg-pixel-buffer.h',
    'cglib/cg-sub-texture.h',
    'cglib/cg-feature-private.h',

    'cglib/driver/nop/cg-attribute-nop.c',
    'cglib/driver/nop/cg-pipeline-vertend-nop-private.h',
    'cglib/driver/nop/cg-framebuffer-nop-private.h',
    'cglib/driver/nop/cg-pipeline-progend-nop-private.h',
    'cglib/driver/nop/cg-attribute-nop-private.h',
    'cglib/driver/nop/cg-pipeline-fragend-nop-private.h',
    'cglib/driver/nop/cg-pipeline-vertend-nop.c',
    'cglib/driver/nop/cg-texture-2d-nop.c',
    'cglib/driver/nop/cg-texture-2d-nop-private.h',
    'cglib/driver/nop/cg-clip-stack-nop.c',
    'cglib/driver/nop/cg-pipeline-fragend-nop.c',
    'cglib/driver/nop/cg-pipeline-progend-nop.c',
    'cglib/driver/nop/cg-driver-nop.c',
    'cglib/driver/nop/cg-clip-stack-nop-private.h',
    'cglib/driver/nop/cg-framebuffer-nop.c',

    'cglib/cg-gles2.h',
    'cglib/cg-gl-header.h',

    'cglib/cg-gles2-context.c',

    'cglib/gl-prototypes/cg-all-functions.h',
    'cglib/gl-prototypes/cg-glsl-functions.h',
    'cglib/gl-prototypes/cg-fixed-functions.h',
    'cglib/gl-prototypes/cg-in-gles2-core-functions.h',
    'cglib/gl-prototypes/cg-gles2-functions.h',
    'cglib/gl-prototypes/cg-core-functions.h',

    'cglib/driver/gl/cg-attribute-gl-private.h',
    'cglib/driver/gl/cg-clip-stack-gl.c',
    'cglib/driver/gl/cg-pipeline-opengl.c',
    'cglib/driver/gl/cg-pipeline-vertend-glsl.c',
    'cglib/driver/gl/cg-texture-2d-gl.c',
    'cglib/driver/gl/cg-pipeline-fragend-glsl.c',
    'cglib/driver/gl/cg-texture-2d-gl-private.h',
    'cglib/driver/gl/cg-pipeline-progend-glsl-private.h',
    'cglib/driver/gl/cg-texture-gl-private.h',
    'cglib/driver/gl/cg-pipeline-vertend-glsl-private.h',
    'cglib/driver/gl/cg-framebuffer-gl.c',
    'cglib/driver/gl/cg-pipeline-progend-glsl.c',
    'cglib/driver/gl/cg-buffer-gl.c',
    'cglib/driver/gl/cg-clip-stack-gl-private.h',
    'cglib/driver/gl/cg-attribute-gl.c',
    'cglib/driver/gl/cg-util-gl.c',
    'cglib/driver/gl/cg-util-gl-private.h',
    'cglib/driver/gl/cg-pipeline-fragend-glsl-private.h',
    'cglib/driver/gl/cg-buffer-gl-private.h',
    'cglib/driver/gl/cg-pipeline-opengl-private.h',
    'cglib/driver/gl/cg-framebuffer-gl-private.h',
    'cglib/driver/gl/cg-texture-gl.c',

    'cglib/driver/gl/gl/cg-texture-driver-gl.c',
    'cglib/driver/gl/gl/cg-driver-gl.c',

    'cglib/driver/gl/gles/cg-driver-gles.c',
    'cglib/driver/gl/gles/cg-texture-driver-gles.c',

    'cglib/winsys/cg-winsys-private.h',
    'cglib/winsys/cg-winsys-stub-private.h',
    'cglib/winsys/cg-winsys-stub.c',
    'cglib/winsys/cg-winsys.c',
]

libuv_dep = dependency('libuv', required: get_option('support_libuv'))
if libuv_dep.found()
    deps += libuv_dep
    public_defines += [ '-DCG_HAS_UV_SUPPORT' ]
    c_defines += '-DUSE_UV=1'
    src += [
        'cglib/cg-uv-private.h',
        'cglib/cg-uv.h',
        'cglib/cg-uv.c',
    ]
endif

glib_dep = dependency('glib-2.0', required: get_option('support_glib'))
if glib_dep.found()
    deps += glib_dep
    public_defines += [ '-DCG_HAS_GLIB_SUPPORT' ]
    src += [
        'cglib/cg-glib-source.h',
        'cglib/cg-glib-source.c',
        'cglib/cg-gtype-private.h',
    ]
endif

x11_dep = dependency('x11', required: get_option('support_x11'))
if x11_dep.found()
    deps += x11_dep
    deps += dependency('xext', required: true)
    deps += dependency('xfixes', required: true)
    deps += dependency('xcomposite', required: true)
    deps += dependency('xrandr', required: true)
    deps += dependency('xdamage', required: true)
    public_defines += [ '-DCG_HAS_XLIB_SUPPORT', '-DCG_HAS_X11_SUPPORT' ]
    src += [
        'cglib/cg-xlib.h',
        'cglib/cg-xlib-renderer.h',
        'cglib/cg-xlib-renderer.c',
        'cglib/cg-x11-renderer-private.h',
        'cglib/cg-xlib-renderer-private.h',
        'cglib/winsys/cg-texture-pixmap-x11-private.h',
        'cglib/winsys/cg-texture-pixmap-x11.h',
        'cglib/winsys/cg-texture-pixmap-x11.c',
    ]
endif

glx_dep = dependency('gl', required: get_option('support_glx'))
if glx_dep.found()
    deps += glx_dep
    public_defines += [ '-DCG_HAS_GLX_SUPPORT', '-DCG_HAS_GL_SUPPORT', '-DCG_HAS_XLIB_SUPPORT', '-DCG_HAS_X11_SUPPORT'  ]
    src += [
        'cglib/cg-glx-display-private.h',
        'cglib/cg-glx-renderer-private.h',
        'cglib/winsys/cg-winsys-glx-private.h',
        'cglib/winsys/cg-winsys-glx-feature-functions.h',
        'cglib/winsys/cg-winsys-glx.c',
    ]
endif

egl_dep = dependency('egl', required: get_option('support_egl'))
if egl_dep.found()
    deps += egl_dep
    public_defines += [ '-DCG_HAS_EGL_SUPPORT'  ]
    src += [
        'cglib/cg-egl.h',
        'cglib/cg-egl-defines.h',
        'cglib/winsys/cg-winsys-egl.c',
        'cglib/winsys/cg-winsys-egl-private.h',
        'cglib/winsys/cg-winsys-egl-feature-functions.h',
        'cglib/winsys/cg-winsys-egl-null.c',
        'cglib/winsys/cg-winsys-egl-null-private.h',
    ]

    egl_wayland_dep = dependency('wayland-egl', required: get_option('support_egl_wayland'))
    if egl_wayland_dep.found()
        deps += egl_wayland_dep
        public_defines += [ '-DCG_HAS_EGL_PLATFORM_WAYLAND_SUPPORT'  ]
        src += [
            'cglib/cg-wayland-server.h',
            'cglib/cg-wayland-renderer.h',
            'cglib/cg-wayland-client.h',
            'cglib/winsys/cg-winsys-egl-wayland-private.h',
            'cglib/winsys/cg-winsys-egl-wayland.c',
        ]
    endif

    gbm_dep = dependency('gbm', required: get_option('support_egl_kms'))
    drm_dep = dependency('libdrm', required: get_option('support_egl_kms'))
    if gbm_dep.found() and drm_dep.found()
        deps += gbm_dep
        deps += drm_dep
        gbm_ver = gbm_dep.version().split('.')
        c_defines += [ '-DCG_GBM_MAJOR=' + gbm_ver[0] ]
        c_defines += [ '-DCG_GBM_MINOR=' + gbm_ver[1] ]
        c_defines += [ '-DCG_GBM_MICRO=' + gbm_ver[2] ]
        public_defines += [ '-DCG_HAS_EGL_PLATFORM_KMS_SUPPORT'  ]
        src += [
            'cglib/cg-kms-display.h',
            'cglib/cg-kms-renderer.h',
            'cglib/winsys/cg-winsys-egl-kms-private.h',
            'cglib/winsys/cg-winsys-egl-kms.c',
        ]
    endif

    egl_xlib_dep = dependency('x11', required: get_option('support_egl_xlib'))
    if egl_xlib_dep.found()
        deps += egl_xlib_dep
        public_defines += [ '-DCG_HAS_EGL_PLATFORM_XLIB_SUPPORT'  ]
        src += [
            'cglib/winsys/cg-winsys-egl-x11.c'
        ]
    endif

endif # support_egl

sdl_dep = dependency('sdl2', required: get_option('support_sdl'))
if sdl_dep.found()
    deps += sdl_dep
    public_defines += [ '-DCG_HAS_SDL_SUPPORT'  ]
    src += [
        'cglib/cg-sdl.h',
        'cglib/cg-sdl.c',
        'cglib/winsys/cg-winsys-sdl2.c',
        'cglib/winsys/cg-winsys-sdl-private.h',
    ]
endif

gles2_dep = dependency('glesv2', required: get_option('support_gles2'))
if gles2_dep.found()
    deps += gles2_dep
    public_defines += [ '-DCG_HAS_GLES2_SUPPORT'  ]
endif


if compiler.get_define('ANDROID') != ''
src += [
    'cglib/winsys/cg-winsys-egl-android-private.h',
    'cglib/winsys/cg-winsys-egl-android.c',
]
endif

if host_machine.system() == 'windows'
src += [
    'cglib/cg-win32-renderer.h',
    'cglib/cg-win32-renderer.c',
    'cglib/winsys/cg-winsys-wgl-feature-functions.h',
    'cglib/winsys/cg-winsys-wgl-private.h',
    'cglib/winsys/cg-winsys-wgl.c',
]
sys_libs += [
]
elif host_machine.system() == 'darwin'
src += [
    'clib/cmisc-unix.c',
    'clib/ctls.c',
    'clib/fmemopen.c',
]
elif compiler.get_define('EMSCRIPTEN') != ''
src += [
    'cglib/cg-webgl-private.h',
    'cglib/cg-webgl.h',
    'cglib/cg-webgl.c',
    'cglib/cg-webgl-renderer.c',
    'cglib/cg-emscripten-lib.c',
]
endif

cglib_lib = library('cglib', src,
                    version: '1',
                    include_directories: inc,
                    c_args: c_defines + warn_opts + public_defines,
                    link_args: sys_libs,
                    dependencies: deps)
cglib_dep = declare_dependency(link_with: cglib_lib, include_directories: public_inc)
