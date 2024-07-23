# `@philipmueller/global-hotkey`

## Installation

```shell
npm install @philipmueller/global-hotkey
```

## Build

```shell
electron-rebuild
```

## Usage

```js
const globalHotkey = require('@philipmueller/global-hotkey');
globalHotkey.addListener({meta: true, keyCode: 'V'});
globalHotkey.startListening();
```
