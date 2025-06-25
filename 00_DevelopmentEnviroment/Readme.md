## Setting Up the Development Environment

To follow along with this project, you’ll need to set up a development environment. Since I’m targeting the ARM64 architecture of the M1 Mac, the setup will be tailored for macOS. Below are the steps to get started:

### 1. Install Xcode and Command Line Tools

Xcode provides essential development tools for macOS. To install Xcode’s command line tools, open a terminal and run:

```
xcode-select --install
```

This will install the necessary compilers, assemblers, and linkers for macOS development.

---



### 2. Install Homebrew

[Homebrew](https://brew.sh/) is a popular package manager for macOS that makes it easy to install additional tools. To install Homebrew, run the following command in your terminal:

```
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

Once Homebrew is installed, you can use it to install other tools required for this project.

---



### 3. Install GCC and Make

To compile and build the project, you’ll need gcc and make. Install them using Homebrew:

```
brew install gcc make
```

---

### 4. Install LLVM (Optional)

If you want to use LLVM for generating and testing ARM64 assembly code, you can install it with Homebrew:

```
brew install llvm
```

LLVM provides tools like clang (a C compiler) and llc (a low-level code generator) that can be useful for testing and debugging.

---



### 5. Verify Your Setup

To ensure everything is installed correctly, you can verify the versions of the tools:

```
gcc --version
make --version
clang --version
```

If these commands return version information, your setup is ready to go.
