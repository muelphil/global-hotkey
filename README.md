# `global-hotkey`

`global-hotkey` is a Node.js module that allows you to register and handle global hotkeys on Windows. This module provides functionality to listen for key combinations and execute specified callbacks when the keys are pressed.

## Installation

To install `global-hotkey`, use npm:

```shell
npm install global-hotkey
```

## Build

If you are using Electron or need to rebuild native bindings, run:

```shell
electron-rebuild
```

## Usage

Here’s a basic example of how to use `global-hotkey` in your Node.js application:

```js
const globalHotkey = require('global-hotkey');

// Add a listener for META + V
globalHotkey.addListener({meta: true, keyCode: globalHotkey.V_KEY_CODES.V}, () => {
    console.log('Hello from META + V');
});

// Start listening for hotkeys
globalHotkey.startListening();
```

## Functions

### `globalHotkey.addListener(keyEvent, callback)`

- **keyEvent**: An object specifying the key combination to listen for. Needs to include `keyCode` and atleast one of `ctrl`, `meta` and `alt`.
    - `ctrl` (boolean): `true` if Control key should be pressed.
    - `meta` (boolean): `true` if Meta (Windows) key should be pressed.
    - `alt` (boolean): `true` if Alt key should be pressed.
    - `shift` (boolean): `true` if Shift key should be pressed.
    - `keyCode` (number): The virtual key code of the key to listen for.
- **callback**: A function that will be called when the key combination is pressed.

Returns a unique listener ID which can be used to remove the listener.

### `globalHotkey.removeListener(id)`

- **id**: The listener ID returned from `addListener`.

Removes the specified listener.

### `globalHotkey.startListening()`

Starts the internal thread to listen for key events. Must be called to begin processing hotkey events.

### `globalHotkey.stopListening()`

Stops the internal thread from listening for key events and cleans up resources.

## Test

Here’s an example test file to demonstrate how to use the `global-hotkey` module:

```js
const globalHotkey = require('global-hotkey');

// Add a listener for META + V
const listenerId = globalHotkey.addListener({
    meta: true,
    keyCode: globalHotkey.V_KEY_CODES.V
}, () => console.log('Hello from META + V'));

// Add a listener for ALT + SPACE
globalHotkey.addListener({
    alt: true,
    keyCode: globalHotkey.V_KEY_CODES.SPACE
}, () => console.log('Hello from ALT + SPACE'));

// Add a listener for CTRL + SHIFT + C
globalHotkey.addListener({
    ctrl: true,
    shift: true,
    keyCode: globalHotkey.V_KEY_CODES.C
}, () => console.log('Hello from CTRL + C'));

// Remove the META + V listener
globalHotkey.removeListener(listenerId);

console.log('Starting thread...');
globalHotkey.startListening();

// Stop listening after 20 seconds
setTimeout(() => {
    console.log('Stopping thread...');
    globalHotkey.stopListening();
}, 20000);
```

## Issues

If you encounter any issues or have suggestions for improvements, please open an issue on our [GitHub repository](https://github.com/muelphil/global-hotkey/issues).

## License

This module is licensed under the MIT License. See the [LICENSE](LICENSE) file for more details.

## Support the Project

If you enjoy using this project or find it helpful, you can support its continued development by buying me a coffee!

[Buy Me a Coffee](https://www.paypal.com/donate/?hosted_button_id=GH3HD9DSQ3US8)

Thank you for your support and for helping keep this project going!

