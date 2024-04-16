_BUILD_CONTENT_PROLOGUE = """
load("@@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])"""

_CC_LIBRARY_PROLOGUE = """
cc_library(
    name = "{name}",
    srcs = ["{lib}"],
    hdrs = glob({hdrs}),
    includes = {includes},"""

_CC_LIBRARY_LINKOPTS = """    linkopts = {linkopts},"""

_CC_LIBRARY_EPILOGUE = """)"""

def _resolve_lib_name(lib, os, arch):
    if os == "linux":
        if arch == "aarch64":
            return "lib/aarch64-linux-gnu/lib{lib}.so".format(lib = lib)
        elif arch == "amd64":
            return "lib/x86_64-linux-gnu/lib{lib}.so".format(lib = lib)
        else:
            fail("Unsupported arch: {arch}".format(arch = arch))
    elif os.startswith("mac"):
        return "lib/lib{lib}.dylib".format(lib = lib)
    else:
        fail("Unsupported os: {os}".format(os = os))

def _sysroot_repo_impl(repository_ctx):
    source_folder = repository_ctx.path(repository_ctx.attr.path)

    if not source_folder.exists:
        fail("Local repo path does not exist: {path}".format(path = source_folder.path))

    if not source_folder.is_dir:
        fail("Local repo path is not a directory: {path}".format(path = source_folder.path))

    for child in source_folder.readdir():
        repository_ctx.symlink(child, child.basename)

    repository_ctx.file("BUILD", repository_ctx.attr.build_file_content)

_sysroot_repo = repository_rule(
    implementation = _sysroot_repo_impl,
    local = True,
    attrs = {
        "build_file_content": attr.string(mandatory = True),
        "path": attr.string(mandatory = True),
    },
)

def _get_local_repo(os, module):
    # Would be great to have a shortcut for unique tags similar to attrs module
    local_path = module.tags.local_paths[0]
    if os == "linux":
        return local_path.path_linux
    elif os.startswith("mac"):
        return local_path.path_macos
    return None

def _sysroot_impl(module_ctx):
    build_file_content = []
    build_file_content.append(_BUILD_CONTENT_PROLOGUE)

    # Would be great to have a shortcut for unique tags similar to attrs module
    local_repo = _get_local_repo(module_ctx.os.name, module_ctx.modules[0])

    # Validate the local path
    if local_repo == None:
        fail("No local repo specified")

    for module in module_ctx.modules:
        for lib in module.tags.declare_library:
            # Generate the includes globs
            hdrs = ["{include}/*".format(include = include) for include in lib.includes]

            # Begin the cc_library rule
            build_file_content.append(_CC_LIBRARY_PROLOGUE.format(
                name = lib.name,
                lib = _resolve_lib_name(lib.name, module_ctx.os.name, module_ctx.os.arch),
                hdrs = hdrs,
                includes = lib.includes,
            ))

            # Add the linkopts if they are specified
            if module_ctx.os.name == "linux" and lib.linkopts_linux:
                build_file_content.append(_CC_LIBRARY_LINKOPTS.format(linkopts = lib.linkopts_linux))
            elif module_ctx.os.name.startswith("mac") and lib.linkopts_macos:
                build_file_content.append(_CC_LIBRARY_LINKOPTS.format(linkopts = lib.linkopts_macos))

            # Close the cc_library rule
            build_file_content.append(_CC_LIBRARY_EPILOGUE)

    # Generate the sysroot repository
    _sysroot_repo(
        name = "sysroot",
        build_file_content = "\n".join(build_file_content),
        path = local_repo,
    )

_local_paths = tag_class(attrs = {
    "path_linux": attr.string(),
    "path_macos": attr.string(),
})

_library = tag_class(attrs = {
    "includes": attr.string_list(),
    "linkopts_linux": attr.string_list(),
    "linkopts_macos": attr.string_list(),
    "name": attr.string(mandatory = True),
})

sysroot = module_extension(
    implementation = _sysroot_impl,
    os_dependent = True,
    arch_dependent = True,
    tag_classes = {"declare_library": _library, "local_paths": _local_paths},
)
