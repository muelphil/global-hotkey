const globalHotkey = require('.');
const {resetKeyPressed} = require("./index");

const listenerId = globalHotkey.addListener({
    ctrl: false,
    meta: true,
    alt: false,
    shift: false,
    keyCode: globalHotkey.V_KEY_CODES.V
}, () => console.log('Hello from META + V'))
globalHotkey.removeListener(listenerId);

globalHotkey.addListener({
    ctrl: false,
    meta: false,
    alt: true,
    shift: false,
    keyCode: globalHotkey.V_KEY_CODES.SPACE
}, () => console.log('Hello from ALT + SPACE'))

globalHotkey.addListener({
    ctrl: true,
    meta: false,
    alt: false,
    shift: false,
    keyCode: globalHotkey.V_KEY_CODES.C
}, () => console.log('Hello from CTRL + V'))

console.log('Starting thread...');
globalHotkey.startListening();
globalHotkey.resetKeyPressed();

setTimeout(() => {
    console.log('Stopping thread...');
    globalHotkey.stopListening();
}, 5000); // Stop after 20 seconds