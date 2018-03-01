# install-cli

Bash library for guided project bootstrapping &amp; installation

## Why install-cli?

* Bash is widely-available across execution environments &ndash; chances are, anyone attempting to work with your project has it &ndash; no confusion, no fuss
* &hellip;And, as a shell-scripting language, Bash puts immediately in-hand the straight-forward lower-level system introspection and execution tools you need to check/install your project's most basic dependencies
* Unlike higher-level (scripting) languages such as Python and Ruby, there need be little-to-no concern over the *version* installed to the environment

**install-cli** removes the boilerplate of providing a friendly, guided installation &amp; bootstrapping script, such that with a minimum of shell scripting, you can provide your project with its optimal environment, and get straight away to working with the language and tools you *really* want.

## What isn't install-cli?

**install-cli** is *not* intended to handle *all* project building/installation/maintenance tasks. Depending on your project, it might not be appropriate to use this library to *build* your project at all.

Rather, this library is intended to provision the environment your project requires, either for building or execution.

If your project requires compilation via `make`, you might like to use **install-cli** to ensure `make` is installed.

Or, (this library's original impetus), your project might require a command-definition tool such as [argcmdr](https://github.com/dssg/argcmdr/), for building, maintenance, execution, *etc.* Use **install-cli** to ensure the appropriate version of Python is installed, as well as the `argcmdr` library.

## Use

Simply copy [install.example](install.example) into your project, *e.g.* as an executable file `install`, and extend it for your purposes.

The main utility of **install-cli** is the function `require`:

    require REQUIREMENT-NAME CHECK-FN INSTALL-FN [INSTALL-TEXT]

For example, the below will check that `pyenv` is installed, and install it if not:

    exists_pyenv() {
      test $(which pyenv)
    }

    install_pyenv() {
      curl -#L https://github.com/pyenv/pyenv-installer/raw/master/bin/pyenv-installer | bash
    }

    require pyenv \
      exists_pyenv \
      install_pyenv \
      "not found –"

An `install` file, extended by the above, will prompt the user with output like the following:

    (install) begin

    (pyenv) not found – install?
    1) yes, install {curl -#L https://github.com/pyenv/pyenv-installer/raw/master/bin/pyenv-installer | bash}
    2) no, ignore
    #?

See [install.example](install.example) for more information.
