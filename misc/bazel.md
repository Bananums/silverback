# Bazel #

## Autocomplete ##

### VSCode ###

For VSCode, the answer is compile_commands.json — a compilation database that tells the language server (clangd) where every header lives, including Bazel-managed ones like zenoh. 

Remember to install `clangd` extension

The standard tool for generating this from Bazel is hedron_compile_commands. Here's the setup:

1. Add to MODULE.bazel

```text
bazel_dep(name = "hedron_compile_commands", dev_dependency = True)
git_override(
    module_name = "hedron_compile_commands",
    remote = "https://github.com/hedronvision/bazel-compile-commands-extractor.git",
    commit = "<latest commit hash from that repo>",
)
```

