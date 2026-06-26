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

## Install Bazel Buildifier tools

**arch linux**
Simply install latest release with `Go`


```shell
# Install go if needed
sudo pacman -S go
```



Install `buildifier` and `buildozer`:

```shell
go install github.com/bazelbuild/buildtools/buildifier@latest
go install github.com/bazelbuild/buildtools/buildozer@latest
```

Add Go's binary directory to your PATH for interactive Bash shells:

```shell
echo 'export PATH="$HOME/go/bin:$PATH"' >> ~/.bashrc
```

Restart your shell or run:

```shell
source ~/.bashrc
```

### VS Code GUI launch ###

When launching VS Code from a desktop application, it may not inherit the PATH from `.bashrc`. Configure the Buildifier path explicitly in VS Code `settings.json`:

```json
{
    "bazel.buildifier.path": "/home/youruser/go/bin/buildifier"
}
```

Replace `/home/youruser` with your actual home directory.
