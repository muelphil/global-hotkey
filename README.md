# `@philipmueller/global-hotkey`

## Installation
```
npm install @philipmueller/global-hotkey
```
## Build
```
electron-rebuild
```
## Usage
```js
const globalHotkey = require('@philipmueller/global-hotkey');
globalHotkey.addListener({meta: true, keyCode: 'V'});
globalHotkey.startListening();
```
