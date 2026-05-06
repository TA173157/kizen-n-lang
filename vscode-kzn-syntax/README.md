# Kizen-N `.kzn` Syntax Highlighting

This is a minimal VS Code language extension for `.kzn` files.

## Use

1. Open the `vscode-kzn-syntax` folder in VS Code.
2. Press `F5` to launch an Extension Development Host.
3. In the new host window, open a `.kzn` file from your project.

If you want to install it permanently:

1. Install `vsce` if you have it: `npm install -g vsce`
2. From the `vscode-kzn-syntax` folder run: `vsce package`
3. Install the generated `.vsix` in VS Code using `Extensions: Install from VSIX...`.

## Notes

- This extension only provides syntax highlighting.
- No TypeScript or other language validation is enabled for `.kzn` now.
