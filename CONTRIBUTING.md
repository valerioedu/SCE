
# Contributing to SCE - Simple Code Editor

Thank you for your interest in contributing to SCE! Your help is appreciated. Please read the following guidelines to make the process smooth for everyone.

## How to Contribute

### 1. Fork the Repository

Click the "Fork" button at the top right of the [repository](https://github.com/valerioedu/SCE2) page to create your own copy.

### 2. Clone Your Fork

```sh
git clone https://github.com/your-username/SCE2.git
cd SCE2
```

### 3. Create a Branch

Create a new branch for your feature or bugfix:

```sh
git checkout -b my-feature
```

### 4. Make Your Changes

- Follow the existing code style.
- Add comments where necessary.
- Update or add documentation if needed.

### 5. Test Your Changes

```sh
  ./build.sh -t # or --test
```

### 6. Commit and Push

```sh
git add .
git commit -m "Describe your changes"
git push origin my-feature
```

### 7. Open a Pull Request

Go to your fork on GitHub and click "New Pull Request". Fill in the template and describe your changes.

## Code Style

- Use C11 standard.
- Indent with 4 spaces.
- Use descriptive variable and function names.
- Document public functions in headers.

## Reporting Issues

If you find a bug or have a feature request, please open an [issue](https://github.com/valerioedu/SCE2/issues) and provide as much detail as possible.

## License

By contributing, you agree that your contributions will be licensed under the [GPL-3.0 License](LICENSE).

Thank you for helping make SCE better!