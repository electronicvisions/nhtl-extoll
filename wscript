#!/usr/bin/env python
import os
from os.path import join
from waflib.extras.test_base import summary
from waflib.extras.symwaf2ic import get_toplevel_path

def depends(dep):
    dep('code-format')
    dep('hate')


def options(opt):
    opt.load('compiler_c')
    opt.load('compiler_cxx')
    opt.load("test_base")
    opt.load("gtest")
    opt.load("doxygen")


def configure(conf):
    conf.load('compiler_c')
    conf.load('compiler_cxx')
    conf.load("test_base")
    conf.load("gtest")
    conf.load("doxygen")
    conf.env.CXXFLAGS_NHTL_EXTOLL = [
        '-fvisibility=hidden',
        '-fvisibility-inlines-hidden',
    ]
    conf.env.LINKFLAGS_NHTL_EXTOLL = [
        '-fvisibility=hidden',
        '-fvisibility-inlines-hidden',
    ]


def build(bld):
    bld(target          = 'nhtl_extoll_inc',
        export_includes = 'include'
    )

    bld(
        target       = 'nhtl_extoll',
        features     = 'cxx use',
        source       = bld.path.ant_glob('src/nhtl-extoll/*.cpp'),
        use          = ['nhtl_extoll_inc', 'hate_inc'],
        install_path = '${PREFIX}/lib',
    )

    bld(
        target       = 'nhtl_extoll_swtest',
        features     = 'gtest cxx cxxprogram',
        source       = bld.path.ant_glob('tests/sw/nhtl-extoll/test-*.cpp'),
        use          = ['nhtl_extoll'],
        uselib       = 'NHTL_EXTOLL',
        test_main    = 'tests/common/src/main.cpp',
    )

    bld(
        features = 'doxygen',
        name = 'nhtl_extoll_documentation',
        doxyfile = bld.root.make_node(join(get_toplevel_path(), "code-format", "doxyfile")),
        install_path = 'doc/nhtl_extoll',
        pars = {
            "PROJECT_NAME": "\"NHTL-EXTOLL\"",
            "INPUT": join(get_toplevel_path(), "nhtl-extoll", "include"),
            "INCLUDE_PATH": join(get_toplevel_path(), "nhtl-extoll", "include"),
            "OUTPUT_DIRECTORY": join(get_toplevel_path(), "build", "nhtl-extoll", "doc")
        },
    )

    # Create test summary (to stdout and XML file)
    bld.add_post_fun(summary)
